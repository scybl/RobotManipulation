/* feel free to change any part of this file, or delete this file. In general,
you can do whatever you want with this template code, including deleting it all
and starting from scratch. The only requirment is to make sure your entire
solution is contained within the cw1_team_<your_team_number> package */

#include <cw1_class.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit/robot_trajectory/robot_trajectory.h>
#include <moveit/trajectory_processing/iterative_time_parameterization.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <pcl/common/common.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/filter.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/search/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl_conversions/pcl_conversions.h>
#include <rmw/qos_profiles.h>
#include <shape_msgs/msg/solid_primitive.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

namespace
{
  constexpr double kCubeSize = 0.04;
  constexpr double kBasketSize = 0.10;
  constexpr double kPreGraspOpenWidth = 0.05;
  constexpr double kContactGripperWidth = 0.045;
  constexpr double kReleaseOpenWidth = 0.06;
  constexpr double kFloorThickness = 0.02;
  constexpr double kTask3PlacedCubeRadius = 0.075;
  constexpr int kTask3MaxCycles = 6;
  constexpr int kTask3MaxConsecutiveFailures = 3;
  constexpr char kWorldFrame[] = "world";

  const std::vector<std::vector<double>> kScanJointTargets = {
      {0.00, -0.35, 0.00, -2.10, 0.00, 1.90, 0.785},  // center
      {0.45, -0.35, 0.00, -2.10, 0.00, 1.90, 0.785},  // left view
      {-0.45, -0.35, 0.00, -2.10, 0.00, 1.90, 0.785}, // right view
      {0.25, -0.10, 0.00, -2.00, 0.00, 1.75, 0.50},   // angled view 1
      {-0.25, -0.10, 0.00, -2.00, 0.00, 1.75, 1.05}   // angled view 2
  };
} // namespace

///////////////////////////////////////////////////////////////////////////////

cw1::cw1(const rclcpp::Node::SharedPtr &node)
{
  node_ = node;

  tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

  service_cb_group_ = node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
  sensor_cb_group_ = node_->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

  t1_service_ = node_->create_service<cw1_world_spawner::srv::Task1Service>(
      "/task1_start",
      std::bind(&cw1::t1_callback, this, std::placeholders::_1, std::placeholders::_2),
      rmw_qos_profile_services_default, service_cb_group_);
  t2_service_ = node_->create_service<cw1_world_spawner::srv::Task2Service>(
      "/task2_start",
      std::bind(&cw1::t2_callback, this, std::placeholders::_1, std::placeholders::_2),
      rmw_qos_profile_services_default, service_cb_group_);
  t3_service_ = node_->create_service<cw1_world_spawner::srv::Task3Service>(
      "/task3_start",
      std::bind(&cw1::t3_callback, this, std::placeholders::_1, std::placeholders::_2),
      rmw_qos_profile_services_default, service_cb_group_);

  rclcpp::SubscriptionOptions joint_state_sub_options;
  joint_state_sub_options.callback_group = sensor_cb_group_;
  auto joint_state_qos = rclcpp::QoS(rclcpp::KeepLast(50));
  joint_state_qos.reliable();
  joint_state_qos.durability_volatile();
  joint_state_sub_ = node_->create_subscription<sensor_msgs::msg::JointState>(
      "/joint_states", joint_state_qos,
      [this](const sensor_msgs::msg::JointState::ConstSharedPtr msg)
      {
        {
          std::lock_guard<std::mutex> lock(joint_state_mutex_);
          latest_joint_state_msg_ = std::make_shared<sensor_msgs::msg::JointState>(*msg);
        }
        const int64_t stamp_ns =
            static_cast<int64_t>(msg->header.stamp.sec) * 1000000000LL +
            static_cast<int64_t>(msg->header.stamp.nanosec);
        latest_joint_state_stamp_ns_.store(stamp_ns, std::memory_order_relaxed);
        joint_state_msg_count_.fetch_add(1, std::memory_order_relaxed);
      },
      joint_state_sub_options);

  rclcpp::SubscriptionOptions cloud_sub_options;
  cloud_sub_options.callback_group = sensor_cb_group_;
  auto cloud_qos = rclcpp::SensorDataQoS();
  cloud_sub_ = node_->create_subscription<sensor_msgs::msg::PointCloud2>(
      "/r200/camera/depth_registered/points",
      cloud_qos,
      [this](const sensor_msgs::msg::PointCloud2::ConstSharedPtr msg)
      {
        {
          std::lock_guard<std::mutex> lock(cloud_mutex_);
          latest_cloud_msg_ = std::make_shared<sensor_msgs::msg::PointCloud2>(*msg);
        }

        const int64_t stamp_ns =
            static_cast<int64_t>(msg->header.stamp.sec) * 1000000000LL +
            static_cast<int64_t>(msg->header.stamp.nanosec);
        latest_cloud_stamp_ns_.store(stamp_ns, std::memory_order_relaxed);
        cloud_msg_count_.fetch_add(1, std::memory_order_relaxed);
      },
      cloud_sub_options);

  const bool use_gazebo_gui = node_->declare_parameter<bool>("use_gazebo_gui", true);
  (void)use_gazebo_gui;
  enable_cloud_viewer_ = node_->declare_parameter<bool>("enable_cloud_viewer", false);
  move_home_on_start_ = node_->declare_parameter<bool>("move_home_on_start", false);
  use_path_constraints_ = node_->declare_parameter<bool>("use_path_constraints", false);
  use_cartesian_reach_ = node_->declare_parameter<bool>("use_cartesian_reach", false);
  allow_position_only_fallback_ = node_->declare_parameter<bool>(
      "allow_position_only_fallback", allow_position_only_fallback_);
  cartesian_eef_step_ = node_->declare_parameter<double>("cartesian_eef_step", cartesian_eef_step_);
  cartesian_jump_threshold_ = node_->declare_parameter<double>(
      "cartesian_jump_threshold", cartesian_jump_threshold_);
  cartesian_min_fraction_ = node_->declare_parameter<double>(
      "cartesian_min_fraction", cartesian_min_fraction_);
  publish_programmatic_debug_ = node_->declare_parameter<bool>(
      "publish_programmatic_debug", publish_programmatic_debug_);
  enable_task1_snap_ = node_->declare_parameter<bool>("enable_task1_snap", false);
  return_home_between_pick_place_ = node_->declare_parameter<bool>(
      "return_home_between_pick_place", return_home_between_pick_place_);
  return_home_after_pick_place_ = node_->declare_parameter<bool>(
      "return_home_after_pick_place", return_home_after_pick_place_);
  pick_offset_z_ = node_->declare_parameter<double>("pick_offset_z", pick_offset_z_);
  task3_pick_offset_z_ = node_->declare_parameter<double>("task3_pick_offset_z", task3_pick_offset_z_);
  task2_capture_enabled_ = node_->declare_parameter<bool>("task2_capture_enabled", task2_capture_enabled_);
  task2_capture_dir_ = node_->declare_parameter<std::string>("task2_capture_dir", task2_capture_dir_);
  place_offset_z_ = node_->declare_parameter<double>("place_offset_z", place_offset_z_);
  grasp_approach_offset_z_ = node_->declare_parameter<double>(
      "grasp_approach_offset_z", grasp_approach_offset_z_);
  post_grasp_lift_z_ = node_->declare_parameter<double>("post_grasp_lift_z", post_grasp_lift_z_);
  gripper_grasp_width_ = node_->declare_parameter<double>("gripper_grasp_width", gripper_grasp_width_);
  joint_state_wait_timeout_sec_ = node_->declare_parameter<double>(
      "joint_state_wait_timeout_sec", joint_state_wait_timeout_sec_);

  pick_hover_offset_z_ = node_->declare_parameter<double>("pick_hover_offset_z", pick_hover_offset_z_);
  basket_release_offset_z_ = node_->declare_parameter<double>(
      "basket_release_offset_z", basket_release_offset_z_);
  scan_settle_time_sec_ = node_->declare_parameter<double>("scan_settle_time_sec", scan_settle_time_sec_);
  voxel_leaf_size_ = node_->declare_parameter<double>("voxel_leaf_size", voxel_leaf_size_);
  cluster_tolerance_ = node_->declare_parameter<double>("cluster_tolerance", cluster_tolerance_);
  candidate_match_distance_ = node_->declare_parameter<double>(
      "candidate_match_distance", candidate_match_distance_);
  workspace_min_x_ = node_->declare_parameter<double>("workspace_min_x", workspace_min_x_);
  workspace_max_x_ = node_->declare_parameter<double>("workspace_max_x", workspace_max_x_);
  workspace_min_y_ = node_->declare_parameter<double>("workspace_min_y", workspace_min_y_);
  workspace_max_y_ = node_->declare_parameter<double>("workspace_max_y", workspace_max_y_);
  workspace_min_z_ = node_->declare_parameter<double>("workspace_min_z", workspace_min_z_);
  workspace_max_z_ = node_->declare_parameter<double>("workspace_max_z", workspace_max_z_);
  plane_distance_threshold_ = node_->declare_parameter<double>(
      "plane_distance_threshold", plane_distance_threshold_);
  colour_min_value_ = node_->declare_parameter<double>("colour_min_value", colour_min_value_);
  colour_min_channel_delta_ = node_->declare_parameter<double>(
      "colour_min_channel_delta", colour_min_channel_delta_);
  cluster_min_points_ = node_->declare_parameter<int>("cluster_min_points", cluster_min_points_);
  cluster_max_points_ = node_->declare_parameter<int>("cluster_max_points", cluster_max_points_);

  RCLCPP_INFO(node_->get_logger(), "cw1 solution class initialised");
}

///////////////////////////////////////////////////////////////////////////////

void cw1::configure_arm_group(moveit::planning_interface::MoveGroupInterface &arm_group) const
{
  arm_group.setPlanningTime(5.0);
  arm_group.setNumPlanningAttempts(10);
  arm_group.allowReplanning(true);
  arm_group.setPoseReferenceFrame(kWorldFrame);
  arm_group.setGoalPositionTolerance(0.01);
  arm_group.setGoalOrientationTolerance(0.05);
  arm_group.setStartStateToCurrentState();
}

geometry_msgs::msg::Pose cw1::make_top_down_pose(double x, double y, double z, double yaw) const
{
  geometry_msgs::msg::Pose pose;
  pose.position.x = x;
  pose.position.y = y;
  pose.position.z = z;

  tf2::Quaternion orientation;
  orientation.setRPY(M_PI, 0.0, yaw);
  orientation.normalize();
  pose.orientation = tf2::toMsg(orientation);
  return pose;
}

bool cw1::set_gripper_width(
    moveit::planning_interface::MoveGroupInterface &gripper_group,
    double opening_width)
{
  const double finger_joint_target = std::clamp(opening_width * 0.5, 0.0, 0.04);
  std::vector<double> joint_targets = {finger_joint_target, finger_joint_target};

  gripper_group.setJointValueTarget(joint_targets);
  const bool success = static_cast<bool>(gripper_group.move());

  if (!success)
  {
    RCLCPP_WARN(node_->get_logger(), "Failed to move gripper to width %.3f", opening_width);
  }

  return success;
}

bool cw1::move_arm_to_pose(
    moveit::planning_interface::MoveGroupInterface &arm_group,
    const geometry_msgs::msg::Pose &target_pose)
{
  arm_group.stop();
  arm_group.setStartStateToCurrentState();
  arm_group.setPoseTarget(target_pose);
  const bool success = static_cast<bool>(arm_group.move());
  arm_group.clearPoseTargets();

  if (!success)
  {
    RCLCPP_WARN(node_->get_logger(), "Failed to reach target pose");
  }

  return success;
}

bool cw1::execute_cartesian_path(
    moveit::planning_interface::MoveGroupInterface &arm_group,
    const std::vector<geometry_msgs::msg::Pose> &waypoints,
    double min_fraction)
{
  if (waypoints.empty())
  {
    return true;
  }

  moveit_msgs::msg::RobotTrajectory trajectory;

  arm_group.stop();
  arm_group.setStartStateToCurrentState();

  const double fraction = arm_group.computeCartesianPath(
      waypoints,
      cartesian_eef_step_,
      cartesian_jump_threshold_,
      trajectory,
      true);

  if (fraction < min_fraction)
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Cartesian path fraction %.3f below required minimum %.3f",
        fraction,
        min_fraction);
    return false;
  }

  robot_trajectory::RobotTrajectory timed_trajectory(
      arm_group.getCurrentState()->getRobotModel(),
      "panda_arm");
  timed_trajectory.setRobotTrajectoryMsg(*arm_group.getCurrentState(), trajectory);

  trajectory_processing::IterativeParabolicTimeParameterization time_parameterisation;
  if (!time_parameterisation.computeTimeStamps(timed_trajectory))
  {
    RCLCPP_WARN(node_->get_logger(), "Failed to time-parameterise Cartesian path");
    return false;
  }

  timed_trajectory.getRobotTrajectoryMsg(trajectory);
  const bool success = static_cast<bool>(arm_group.execute(trajectory));

  if (!success)
  {
    RCLCPP_WARN(node_->get_logger(), "Failed to execute Cartesian path");
  }

  return success;
}

double cw1::get_current_finger_opening_width() const
{
  std::lock_guard<std::mutex> lock(joint_state_mutex_);
  if (!latest_joint_state_msg_)
  {
    return -1.0;
  }

  double finger_joint_1 = std::numeric_limits<double>::quiet_NaN();
  double finger_joint_2 = std::numeric_limits<double>::quiet_NaN();

  for (std::size_t i = 0; i < latest_joint_state_msg_->name.size() &&
                          i < latest_joint_state_msg_->position.size();
       ++i)
  {
    const auto &joint_name = latest_joint_state_msg_->name[i];
    if (joint_name == "panda_finger_joint1")
    {
      finger_joint_1 = latest_joint_state_msg_->position[i];
    }
    else if (joint_name == "panda_finger_joint2")
    {
      finger_joint_2 = latest_joint_state_msg_->position[i];
    }
  }

  if (std::isnan(finger_joint_1) || std::isnan(finger_joint_2))
  {
    return -1.0;
  }

  return finger_joint_1 + finger_joint_2;
}

bool cw1::transform_pose_to_frame(
    const geometry_msgs::msg::PoseStamped &input_pose,
    const std::string &target_frame,
    geometry_msgs::msg::PoseStamped &output_pose) const
{
  if (input_pose.header.frame_id.empty() || input_pose.header.frame_id == target_frame)
  {
    output_pose = input_pose;
    output_pose.header.frame_id = target_frame;
    return true;
  }

  try
  {
    const geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(
        target_frame,
        input_pose.header.frame_id,
        tf2::TimePointZero,
        tf2::durationFromSec(1.0));
    tf2::doTransform(input_pose, output_pose, transform);
    return true;
  }
  catch (const tf2::TransformException &ex)
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Failed to transform pose from %s to %s: %s",
        input_pose.header.frame_id.c_str(),
        target_frame.c_str(),
        ex.what());
    return false;
  }
}

bool cw1::transform_point_to_frame(
    const geometry_msgs::msg::PointStamped &input_point,
    const std::string &target_frame,
    geometry_msgs::msg::PointStamped &output_point) const
{
  if (input_point.header.frame_id.empty() || input_point.header.frame_id == target_frame)
  {
    output_point = input_point;
    output_point.header.frame_id = target_frame;
    return true;
  }

  try
  {
    const geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(
        target_frame,
        input_point.header.frame_id,
        tf2::TimePointZero,
        tf2::durationFromSec(1.0));
    tf2::doTransform(input_point, output_point, transform);
    return true;
  }
  catch (const tf2::TransformException &ex)
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Failed to transform point from %s to %s: %s",
        input_point.header.frame_id.c_str(),
        target_frame.c_str(),
        ex.what());
    return false;
  }
}

void cw1::add_floor_collision_object() const
{
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface;
  moveit_msgs::msg::CollisionObject floor;
  floor.id = "cw1_floor";
  floor.header.frame_id = kWorldFrame;

  shape_msgs::msg::SolidPrimitive primitive;
  primitive.type = shape_msgs::msg::SolidPrimitive::BOX;
  primitive.dimensions = {2.0, 2.0, kFloorThickness};

  geometry_msgs::msg::Pose floor_pose;
  floor_pose.orientation.w = 1.0;
  floor_pose.position.x = 0.3;
  floor_pose.position.y = 0.0;
  floor_pose.position.z = -0.5 * kFloorThickness;

  floor.primitives.push_back(primitive);
  floor.primitive_poses.push_back(floor_pose);
  floor.operation = moveit_msgs::msg::CollisionObject::ADD;

  planning_scene_interface.applyCollisionObject(floor);
}

void cw1::clear_collision_objects() const
{
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface;
  planning_scene_interface.removeCollisionObjects({"cw1_floor"});
}

bool cw1::execute_pick_and_place(
    const geometry_msgs::msg::PoseStamped &object_loc,
    const geometry_msgs::msg::PointStamped &goal_loc,
    double grasp_offset_z)
{
  geometry_msgs::msg::PoseStamped world_object_loc;
  geometry_msgs::msg::PointStamped world_goal_loc;

  if (!transform_pose_to_frame(object_loc, kWorldFrame, world_object_loc) ||
      !transform_point_to_frame(goal_loc, kWorldFrame, world_goal_loc))
  {
    return false;
  }

  moveit::planning_interface::MoveGroupInterface arm_group(node_, "panda_arm");
  moveit::planning_interface::MoveGroupInterface gripper_group(node_, "hand");
  configure_arm_group(arm_group);

  const double yaw = -M_PI / 4.0;
  const double object_x = world_object_loc.pose.position.x;
  const double object_y = world_object_loc.pose.position.y;
  const double object_z = world_object_loc.pose.position.z;
  const double goal_x = world_goal_loc.point.x;
  const double goal_y = world_goal_loc.point.y;
  const double goal_z = world_goal_loc.point.z;

  const double nominal_grasp_z = object_z + grasp_offset_z;
  const double nominal_hover_z = std::max(
      nominal_grasp_z + grasp_approach_offset_z_, object_z + pick_hover_offset_z_);
  const double lift_z = nominal_hover_z + post_grasp_lift_z_;
  const double place_hover_z = goal_z + place_offset_z_;
  const double place_release_z = goal_z + basket_release_offset_z_;
  const double retreat_z = std::max(place_hover_z, place_release_z + post_grasp_lift_z_);

  add_floor_collision_object();

  if (!set_gripper_width(gripper_group, kPreGraspOpenWidth))
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Continuing pick-and-place after gripper open warning because the hand may already be sufficiently open");
  }

  const geometry_msgs::msg::Pose hover_pose = make_top_down_pose(object_x, object_y, nominal_hover_z, yaw);
  const geometry_msgs::msg::Pose lift_pose = make_top_down_pose(object_x, object_y, lift_z, yaw);

  if (!move_arm_to_pose(arm_group, hover_pose))
  {
    clear_collision_objects();
    return false;
  }

  bool grasp_confirmed = false;
  for (const double grasp_adjustment : {0.0, -0.01, -0.02})
  {
    const double grasp_z = nominal_grasp_z + grasp_adjustment;

    const geometry_msgs::msg::Pose grasp_pose =
        make_top_down_pose(object_x, object_y, grasp_z, yaw);

    const geometry_msgs::msg::Pose pre_grasp_pose =
        make_top_down_pose(object_x, object_y, grasp_z + 0.02, yaw);

    if (!execute_cartesian_path(
            arm_group,
            {pre_grasp_pose, grasp_pose},
            cartesian_min_fraction_))
    {
      clear_collision_objects();
      return false;
    }

    if (!set_gripper_width(gripper_group, gripper_grasp_width_))
    {
      clear_collision_objects();
      return false;
    }

    rclcpp::sleep_for(std::chrono::milliseconds(400));

    const double measured_opening = get_current_finger_opening_width();
    if (measured_opening < 0.0 || measured_opening > (gripper_grasp_width_ + 0.004))
    {
      if (measured_opening >= 0.0)
      {
        RCLCPP_INFO(
            node_->get_logger(),
            "Grasp confirmed with measured finger opening %.3f m",
            measured_opening);
      }

      const double secure_grasp_width = std::max(0.0, gripper_grasp_width_ - 0.01);
      if (!set_gripper_width(gripper_group, secure_grasp_width))
      {
        clear_collision_objects();
        return false;
      }

      rclcpp::sleep_for(std::chrono::milliseconds(300));

      if (!execute_cartesian_path(arm_group, {lift_pose}, cartesian_min_fraction_))
      {
        clear_collision_objects();
        return false;
      }

      const double post_lift_opening = get_current_finger_opening_width();
      if (post_lift_opening >= 0.0 && post_lift_opening <= (secure_grasp_width + 0.004))
      {
        RCLCPP_WARN(
            node_->get_logger(),
            "Object likely slipped during lift (finger opening %.3f m); retrying grasp",
            post_lift_opening);

        if (!set_gripper_width(gripper_group, kPreGraspOpenWidth))
        {
          clear_collision_objects();
          return false;
        }

        if (!execute_cartesian_path(arm_group, {hover_pose}, cartesian_min_fraction_))
        {
          clear_collision_objects();
          return false;
        }

        continue;
      }

      grasp_confirmed = true;
      break;
    }

    RCLCPP_WARN(
        node_->get_logger(),
        "Likely empty grasp at %.3f m opening; retrying from a slightly lower grasp height",
        measured_opening);

    if (!set_gripper_width(gripper_group, kPreGraspOpenWidth))
    {
      clear_collision_objects();
      return false;
    }

    if (!execute_cartesian_path(arm_group, {hover_pose}, cartesian_min_fraction_))
    {
      clear_collision_objects();
      return false;
    }
  }

  if (!grasp_confirmed)
  {
    clear_collision_objects();
    return false;
  }

  const double transit_z = std::max(lift_z, place_hover_z) + 0.05;

  const geometry_msgs::msg::Pose transit_pose =
      make_top_down_pose(object_x, object_y, transit_z, yaw);

  const geometry_msgs::msg::Pose basket_hover_pose =
      make_top_down_pose(goal_x, goal_y, transit_z, yaw);

  if (!execute_cartesian_path(
          arm_group,
          {transit_pose, basket_hover_pose},
          cartesian_min_fraction_))
  {
    clear_collision_objects();
    return false;
  }

  const geometry_msgs::msg::Pose release_pose = make_top_down_pose(goal_x, goal_y, place_release_z, yaw);
  if (!execute_cartesian_path(arm_group, {release_pose}, cartesian_min_fraction_))
  {
    clear_collision_objects();
    return false;
  }

  if (!set_gripper_width(gripper_group, kReleaseOpenWidth))
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Release command reported a gripper warning; continuing with retreat");
  }

  const geometry_msgs::msg::Pose retreat_pose = make_top_down_pose(goal_x, goal_y, retreat_z, yaw);
  const bool retreat_success = execute_cartesian_path(arm_group, {retreat_pose}, cartesian_min_fraction_);

  clear_collision_objects();
  return retreat_success;
}

sensor_msgs::msg::PointCloud2::SharedPtr cw1::get_latest_cloud_copy() const
{
  std::lock_guard<std::mutex> lock(cloud_mutex_);
  if (!latest_cloud_msg_)
  {
    return nullptr;
  }

  return std::make_shared<sensor_msgs::msg::PointCloud2>(*latest_cloud_msg_);
}

bool cw1::wait_for_cloud_update(
    uint64_t previous_count,
    std::chrono::milliseconds timeout) const
{
  const auto start_time = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start_time < timeout)
  {
    if (cloud_msg_count_.load(std::memory_order_relaxed) > previous_count)
    {
      return true;
    }

    if (previous_count == 0 && get_latest_cloud_copy())
    {
      return true;
    }

    rclcpp::sleep_for(std::chrono::milliseconds(50));
  }

  return false;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr cw1::transform_latest_cloud(const std::string &target_frame)
{
  const auto cloud_msg = get_latest_cloud_copy();
  if (!cloud_msg)
  {
    RCLCPP_WARN(node_->get_logger(), "No point cloud received yet");
    return std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  }

  sensor_msgs::msg::PointCloud2 transformed_cloud_msg;
  try
  {
    const geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(
        target_frame,
        cloud_msg->header.frame_id,
        tf2::TimePointZero,
        tf2::durationFromSec(1.0));
    tf2::doTransform(*cloud_msg, transformed_cloud_msg, transform);
  }
  catch (const tf2::TransformException &ex)
  {
    RCLCPP_WARN(
        node_->get_logger(),
        "Failed to transform point cloud from %s to %s: %s",
        cloud_msg->header.frame_id.c_str(),
        target_frame.c_str(),
        ex.what());
    return std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  }

  auto pcl_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  pcl::fromROSMsg(transformed_cloud_msg, *pcl_cloud);
  return pcl_cloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr cw1::scan_workspace(const std::string &target_frame)
{
  moveit::planning_interface::MoveGroupInterface arm_group(node_, "panda_arm");
  configure_arm_group(arm_group);

  auto stitched_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  const auto settle_time = std::chrono::milliseconds(
      static_cast<int>(std::max(0.25, scan_settle_time_sec_) * 1000.0));

  if (move_home_on_start_)
  {
    arm_group.setJointValueTarget(kScanJointTargets[1]);
    arm_group.move();
  }

  for (const auto &joint_target : kScanJointTargets)
  {
    const uint64_t cloud_count_before = cloud_msg_count_.load(std::memory_order_relaxed);

    arm_group.setJointValueTarget(joint_target);
    const bool moved = static_cast<bool>(arm_group.move());
    if (!moved)
    {
      RCLCPP_WARN(node_->get_logger(), "Failed to reach one of the scan poses");
      continue;
    }

    if (!wait_for_cloud_update(cloud_count_before, settle_time))
    {
      RCLCPP_WARN(node_->get_logger(), "Timed out waiting for a fresh cloud at scan pose");
    }

    const auto pose_cloud = transform_latest_cloud(target_frame);
    *stitched_cloud += *pose_cloud;
  }

  return stitched_cloud;
}

pcl::PointCloud<pcl::PointXYZRGB>::Ptr cw1::preprocess_cloud(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &input_cloud) const
{
  auto empty_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  if (!input_cloud || input_cloud->empty())
  {
    return empty_cloud;
  }

  auto no_nan_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  std::vector<int> valid_indices;
  pcl::removeNaNFromPointCloud(*input_cloud, *no_nan_cloud, valid_indices);

  auto voxelised_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  pcl::VoxelGrid<pcl::PointXYZRGB> voxel_filter;
  voxel_filter.setInputCloud(no_nan_cloud);
  voxel_filter.setLeafSize(voxel_leaf_size_, voxel_leaf_size_, voxel_leaf_size_);
  voxel_filter.filter(*voxelised_cloud);

  auto filtered_cloud = voxelised_cloud;
  for (const char *axis : {"x", "y", "z"})
  {
    pcl::PassThrough<pcl::PointXYZRGB> pass_filter;
    pass_filter.setInputCloud(filtered_cloud);
    pass_filter.setFilterFieldName(axis);
    if (std::string(axis) == "x")
    {
      pass_filter.setFilterLimits(workspace_min_x_, workspace_max_x_);
    }
    else if (std::string(axis) == "y")
    {
      pass_filter.setFilterLimits(workspace_min_y_, workspace_max_y_);
    }
    else
    {
      pass_filter.setFilterLimits(workspace_min_z_, workspace_max_z_);
    }

    auto axis_filtered_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
    pass_filter.filter(*axis_filtered_cloud);
    filtered_cloud = axis_filtered_cloud;
  }

  if (filtered_cloud->empty())
  {
    return filtered_cloud;
  }

  pcl::SACSegmentation<pcl::PointXYZRGB> segmentation;
  segmentation.setOptimizeCoefficients(true);
  segmentation.setModelType(pcl::SACMODEL_PLANE);
  segmentation.setMethodType(pcl::SAC_RANSAC);
  segmentation.setDistanceThreshold(plane_distance_threshold_);
  segmentation.setInputCloud(filtered_cloud);

  auto plane_inliers = std::make_shared<pcl::PointIndices>();
  auto plane_coefficients = std::make_shared<pcl::ModelCoefficients>();
  segmentation.segment(*plane_inliers, *plane_coefficients);

  if (!plane_inliers->indices.empty())
  {
    pcl::ExtractIndices<pcl::PointXYZRGB> extract_indices;
    extract_indices.setInputCloud(filtered_cloud);
    extract_indices.setIndices(plane_inliers);
    extract_indices.setNegative(true);

    auto objects_only_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
    extract_indices.filter(*objects_only_cloud);
    filtered_cloud = objects_only_cloud;
  }

  pcl::PassThrough<pcl::PointXYZRGB> floor_cleanup;
  floor_cleanup.setInputCloud(filtered_cloud);
  floor_cleanup.setFilterFieldName("z");
  floor_cleanup.setFilterLimits(workspace_min_z_ + 0.01, workspace_max_z_);
  auto cleaned_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
  floor_cleanup.filter(*cleaned_cloud);
  return cleaned_cloud;
}

std::string cw1::classify_cluster_colour(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cluster_cloud) const
{
  if (!cluster_cloud || cluster_cloud->empty())
  {
    return "none";
  }

  auto squared_distance = [](double a, double b, double c, double x, double y, double z)
  {
    return (a - x) * (a - x) + (b - y) * (b - y) + (c - z) * (c - z);
  };

  int coloured_points = 0;
  double red_sum = 0.0;
  double green_sum = 0.0;
  double blue_sum = 0.0;

  for (const auto &point : cluster_cloud->points)
  {
    if (std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z))
    {
      continue;
    }

    const double red = static_cast<double>(point.r) / 255.0;
    const double green = static_cast<double>(point.g) / 255.0;
    const double blue = static_cast<double>(point.b) / 255.0;

    const double max_channel = std::max({red, green, blue});
    const double min_channel = std::min({red, green, blue});

    if (max_channel < colour_min_value_ ||
        (max_channel - min_channel) < colour_min_channel_delta_)
    {
      continue;
    }

    ++coloured_points;
    red_sum += red;
    green_sum += green;
    blue_sum += blue;
  }

  if (coloured_points < 20)
  {
    return "none";
  }

  const double mean_red = red_sum / static_cast<double>(coloured_points);
  const double mean_green = green_sum / static_cast<double>(coloured_points);
  const double mean_blue = blue_sum / static_cast<double>(coloured_points);

  const double red_distance =
      squared_distance(mean_red, mean_green, mean_blue, 0.8, 0.1, 0.1);
  const double blue_distance =
      squared_distance(mean_red, mean_green, mean_blue, 0.1, 0.1, 0.8);
  const double purple_distance =
      squared_distance(mean_red, mean_green, mean_blue, 0.8, 0.1, 0.8);

  // const double min_distance = std::min({red_distance, blue_distance, purple_distance});

  if (blue_distance < red_distance && blue_distance < purple_distance)
  {
    return "blue";
  }

  double rb_diff = mean_red - mean_blue;

  if (std::abs(rb_diff) < 0.25 && mean_blue > 0.2)
  {
    return "purple";
  }

  return "red";
}

std::vector<cw1::DetectedObject> cw1::detect_objects(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud) const
{
  std::vector<DetectedObject> detected_objects;
  if (!cloud || cloud->empty())
  {
    return detected_objects;
  }

  auto search_tree = std::make_shared<pcl::search::KdTree<pcl::PointXYZRGB>>();
  search_tree->setInputCloud(cloud);

  std::vector<pcl::PointIndices> cluster_indices;
  pcl::EuclideanClusterExtraction<pcl::PointXYZRGB> cluster_extraction;
  cluster_extraction.setClusterTolerance(cluster_tolerance_);
  cluster_extraction.setMinClusterSize(cluster_min_points_);
  cluster_extraction.setMaxClusterSize(cluster_max_points_);
  cluster_extraction.setSearchMethod(search_tree);
  cluster_extraction.setInputCloud(cloud);
  cluster_extraction.extract(cluster_indices);

  for (const auto &indices : cluster_indices)
  {
    auto cluster_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
    cluster_cloud->reserve(indices.indices.size());
    for (const int index : indices.indices)
    {
      cluster_cloud->push_back(cloud->points[static_cast<std::size_t>(index)]);
    }

    const std::string colour = classify_cluster_colour(cluster_cloud);
    if (colour == "none")
    {
      continue;
    }

    pcl::PointXYZRGB min_point;
    pcl::PointXYZRGB max_point;
    pcl::getMinMax3D(*cluster_cloud, min_point, max_point);

    DetectedObject object;
    object.center.x = 0.5 * (min_point.x + max_point.x);
    object.center.y = 0.5 * (min_point.y + max_point.y);
    object.center.z = 0.5 * (min_point.z + max_point.z);
    object.size_x = max_point.x - min_point.x;
    object.size_y = max_point.y - min_point.y;
    object.size_z = max_point.z - min_point.z;
    object.point_count = cluster_cloud->size();
    object.color = colour;

    double cube_score =
        std::abs(object.size_x - kCubeSize) +
        std::abs(object.size_y - kCubeSize) +
        std::abs(object.size_z - kCubeSize);
    double basket_score =
        std::abs(object.size_x - kBasketSize) +
        std::abs(object.size_y - kBasketSize) +
        std::abs(object.size_z - kBasketSize);
    const double footprint = std::max(object.size_x, object.size_y);

    if (object.point_count > 300)
    {
      basket_score *= 0.85;
    }

    if (footprint > 0.065 || object.size_z > 0.07)
    {
      basket_score *= 0.80;
    }
    else
    {
      cube_score *= 0.85;
    }

    if (object.point_count < 220)
    {
      cube_score *= 0.90;
    }

    object.is_cube = cube_score <= basket_score;
    object.is_basket = !object.is_cube;

    detected_objects.push_back(object);
  }

  return detected_objects;
}

///////////////////////////////////////////////////////////////////////////////

void cw1::t1_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task1Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task1Service::Response> response)
{
  (void)response;

  RCLCPP_INFO(node_->get_logger(), "Task1 started");
  const bool success = execute_pick_and_place(request->object_loc, request->goal_loc, pick_offset_z_);

  if (!success)
  {
    RCLCPP_ERROR(node_->get_logger(), "Task1 failed to complete cleanly");
  }
  else
  {
    RCLCPP_INFO(node_->get_logger(), "Task1 finished");
  }
}

void cw1::t2_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task2Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task2Service::Response> response)
{
  RCLCPP_INFO(node_->get_logger(), "Task2 started");

  response->basket_colours.clear();
  if (request->basket_locs.empty())
  {
    RCLCPP_WARN(node_->get_logger(), "Task2 received no candidate basket locations");
    return;
  }

  const std::string target_frame =
      request->basket_locs.front().header.frame_id.empty() ? std::string(kWorldFrame) : request->basket_locs.front().header.frame_id;

  const auto stitched_cloud = scan_workspace(target_frame);
  const auto filtered_cloud = preprocess_cloud(stitched_cloud);
  const auto detected_objects = detect_objects(filtered_cloud);

  response->basket_colours.reserve(request->basket_locs.size());

  for (const auto &basket_loc : request->basket_locs)
  {
    std::string detected_colour = "none";

    auto local_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZRGB>>();
    if (filtered_cloud)
    {
      local_cloud->reserve(filtered_cloud->size());
      for (const auto &point : filtered_cloud->points)
      {
        if (std::abs(point.x - basket_loc.point.x) > 0.09 ||
            std::abs(point.y - basket_loc.point.y) > 0.09 ||
            point.z < basket_loc.point.z + 0.005 ||
            point.z > basket_loc.point.z + 0.18)
        {
          continue;
        }

        local_cloud->push_back(point);
      }
    }

    detected_colour = classify_cluster_colour(local_cloud);

    double best_distance = candidate_match_distance_;

    for (const auto &object : detected_objects)
    {
      const double dx = object.center.x - basket_loc.point.x;
      const double dy = object.center.y - basket_loc.point.y;
      const double dz = object.center.z - basket_loc.point.z;
      const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
      if (detected_colour == "none" && distance < best_distance)
      {
        best_distance = distance;
        detected_colour = object.color;
      }
    }

    response->basket_colours.push_back(detected_colour);
    RCLCPP_INFO(
        node_->get_logger(),
        "Task2 candidate at (%.3f, %.3f, %.3f) -> %s",
        basket_loc.point.x,
        basket_loc.point.y,
        basket_loc.point.z,
        detected_colour.c_str());
  }

  RCLCPP_INFO(node_->get_logger(), "Task2 finished");
}

void cw1::t3_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task3Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task3Service::Response> response)
{
  (void)request;
  (void)response;

  RCLCPP_INFO(node_->get_logger(), "Task3 started");

  auto is_cube_already_in_basket =
      [](const DetectedObject &cube, const std::vector<DetectedObject> &baskets)
  {
    for (const auto &basket : baskets)
    {
      const double dx = basket.center.x - cube.center.x;
      const double dy = basket.center.y - cube.center.y;
      const double planar_distance = std::sqrt(dx * dx + dy * dy);
      if (planar_distance < kTask3PlacedCubeRadius)
      {
        return true;
      }
    }

    return false;
  };

  struct Task3Candidate
  {
    DetectedObject cube;
    DetectedObject basket;
    double basket_distance = 0.0;
  };

  int successful_moves = 0;
  int consecutive_failures = 0;

  for (int cycle = 0; cycle < kTask3MaxCycles &&
                      consecutive_failures < kTask3MaxConsecutiveFailures;
       ++cycle)
  {
    const auto stitched_cloud = scan_workspace(kWorldFrame);
    const auto filtered_cloud = preprocess_cloud(stitched_cloud);
    const auto detected_objects = detect_objects(filtered_cloud);

    std::vector<DetectedObject> cubes;
    std::vector<DetectedObject> baskets;
    for (const auto &object : detected_objects)
    {
      if (object.color == "none")
      {
        continue;
      }

      if (object.is_cube)
      {
        cubes.push_back(object);
      }
      else if (object.is_basket)
      {
        baskets.push_back(object);
      }
    }

    if (cubes.empty())
    {
      if (successful_moves == 0)
      {
        RCLCPP_WARN(node_->get_logger(), "Task3 detected no cubes to move");
      }
      break;
    }

    if (baskets.empty())
    {
      RCLCPP_WARN(node_->get_logger(), "Task3 detected no baskets");
      break;
    }

    std::vector<Task3Candidate> candidates;
    candidates.reserve(cubes.size());

    for (const auto &cube : cubes)
    {
      if (is_cube_already_in_basket(cube, baskets))
      {
        continue;
      }

      double best_distance = std::numeric_limits<double>::max();
      const DetectedObject *matched_basket = nullptr;

      for (const auto &basket : baskets)
      {
        if (basket.color != cube.color)
        {
          continue;
        }

        const double dx = basket.center.x - cube.center.x;
        const double dy = basket.center.y - cube.center.y;
        const double distance = std::sqrt(dx * dx + dy * dy);
        if (distance < best_distance)
        {
          best_distance = distance;
          matched_basket = &basket;
        }
      }

      if (!matched_basket)
      {
        RCLCPP_WARN(
            node_->get_logger(),
            "No matching basket found for %s cube at (%.3f, %.3f)",
            cube.color.c_str(),
            cube.center.x,
            cube.center.y);
        continue;
      }

      candidates.push_back(Task3Candidate{cube, *matched_basket, best_distance});
    }

    if (candidates.empty())
    {
      if (successful_moves == 0)
      {
        RCLCPP_WARN(node_->get_logger(), "Task3 found no cubes that still need placing");
      }
      break;
    }

    std::sort(
        candidates.begin(),
        candidates.end(),
        [](const Task3Candidate &lhs, const Task3Candidate &rhs)
        {
          if (std::abs(lhs.cube.center.x - rhs.cube.center.x) > 0.01)
          {
            return lhs.cube.center.x > rhs.cube.center.x;
          }

          return lhs.basket_distance < rhs.basket_distance;
        });

    const auto &target = candidates.front();

    geometry_msgs::msg::PoseStamped cube_pose;
    cube_pose.header.frame_id = kWorldFrame;
    cube_pose.pose.position = target.cube.center;
    cube_pose.pose.orientation.w = 1.0;

    geometry_msgs::msg::PointStamped basket_point;
    basket_point.header.frame_id = kWorldFrame;
    basket_point.point = target.basket.center;

    const double grasp_offset = std::max(
        0.08, task3_pick_offset_z_ - 0.01 * static_cast<double>(consecutive_failures));

    RCLCPP_INFO(
        node_->get_logger(),
        "Moving %s cube at (%.3f, %.3f, %.3f) to basket at (%.3f, %.3f) with grasp offset %.3f",
        target.cube.color.c_str(),
        target.cube.center.x,
        target.cube.center.y,
        target.cube.center.z,
        target.basket.center.x,
        target.basket.center.y,
        grasp_offset);

    if (execute_pick_and_place(cube_pose, basket_point, grasp_offset))
    {
      ++successful_moves;
      consecutive_failures = 0;
      continue;
    }

    ++consecutive_failures;
    RCLCPP_ERROR(
        node_->get_logger(),
        "Task3 failed while moving %s cube on cycle %d",
        target.cube.color.c_str(),
        cycle + 1);
  }

  RCLCPP_INFO(node_->get_logger(), "Task3 finished");
}
