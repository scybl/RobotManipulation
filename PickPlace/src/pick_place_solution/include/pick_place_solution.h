/* Pick-and-place perception pipeline interface. */

#ifndef PICK_PLACE_SOLUTION_H_
#define PICK_PLACE_SOLUTION_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <geometry_msgs/msg/point_stamped.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_sensor_msgs/tf2_sensor_msgs.hpp>

#include "cw1_world_spawner/srv/task1_service.hpp"
#include "cw1_world_spawner/srv/task2_service.hpp"
#include "cw1_world_spawner/srv/task3_service.hpp"

class PickPlaceSolution
{
public:
  explicit PickPlaceSolution(const rclcpp::Node::SharedPtr &node);

  void t1_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task1Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task1Service::Response> response);
  void t2_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task2Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task2Service::Response> response);
  void t3_callback(
    const std::shared_ptr<cw1_world_spawner::srv::Task3Service::Request> request,
    std::shared_ptr<cw1_world_spawner::srv::Task3Service::Response> response);

private:
  struct DetectedObject
  {
    geometry_msgs::msg::Point center;
    std::string color;
    double size_x = 0.0;
    double size_y = 0.0;
    double size_z = 0.0;
    std::size_t point_count = 0;
    bool is_cube = false;
    bool is_basket = false;
  };

  void configure_arm_group(moveit::planning_interface::MoveGroupInterface &arm_group) const;
  geometry_msgs::msg::Pose make_top_down_pose(double x, double y, double z, double yaw) const;
  bool set_gripper_width(
    moveit::planning_interface::MoveGroupInterface &gripper_group,
    double opening_width);
  bool move_arm_to_pose(
    moveit::planning_interface::MoveGroupInterface &arm_group,
    const geometry_msgs::msg::Pose &target_pose);
  bool execute_cartesian_path(
    moveit::planning_interface::MoveGroupInterface &arm_group,
    const std::vector<geometry_msgs::msg::Pose> &waypoints,
    double min_fraction);
  bool transform_pose_to_frame(
    const geometry_msgs::msg::PoseStamped &input_pose,
    const std::string &target_frame,
    geometry_msgs::msg::PoseStamped &output_pose) const;
  bool transform_point_to_frame(
    const geometry_msgs::msg::PointStamped &input_point,
    const std::string &target_frame,
    geometry_msgs::msg::PointStamped &output_point) const;
  void add_floor_collision_object() const;
  void clear_collision_objects() const;
  bool execute_pick_and_place(
    const geometry_msgs::msg::PoseStamped &object_loc,
    const geometry_msgs::msg::PointStamped &goal_loc,
    double grasp_offset_z);
  double get_current_finger_opening_width() const;

  sensor_msgs::msg::PointCloud2::SharedPtr get_latest_cloud_copy() const;
  bool wait_for_cloud_update(
    uint64_t previous_count,
    std::chrono::milliseconds timeout) const;
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr transform_latest_cloud(
    const std::string &target_frame);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr scan_workspace(const std::string &target_frame);
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr preprocess_cloud(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &input_cloud) const;
  std::vector<DetectedObject> detect_objects(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud) const;
  std::string classify_cluster_colour(
    const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cluster_cloud) const;

  rclcpp::Node::SharedPtr node_;
  rclcpp::Service<cw1_world_spawner::srv::Task1Service>::SharedPtr t1_service_;
  rclcpp::Service<cw1_world_spawner::srv::Task2Service>::SharedPtr t2_service_;
  rclcpp::Service<cw1_world_spawner::srv::Task3Service>::SharedPtr t3_service_;
  rclcpp::CallbackGroup::SharedPtr service_cb_group_;
  rclcpp::CallbackGroup::SharedPtr sensor_cb_group_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_;

  mutable std::mutex cloud_mutex_;
  mutable std::mutex joint_state_mutex_;
  sensor_msgs::msg::PointCloud2::SharedPtr latest_cloud_msg_;
  sensor_msgs::msg::JointState::SharedPtr latest_joint_state_msg_;

  std::atomic<int64_t> latest_joint_state_stamp_ns_{0};
  std::atomic<uint64_t> joint_state_msg_count_{0};
  std::atomic<int64_t> latest_cloud_stamp_ns_{0};
  std::atomic<uint64_t> cloud_msg_count_{0};

  bool enable_cloud_viewer_ = false;
  bool move_home_on_start_ = false;
  bool use_path_constraints_ = false;
  bool use_cartesian_reach_ = false;
  bool allow_position_only_fallback_ = false;
  bool publish_programmatic_debug_ = false;
  bool enable_task1_snap_ = false;
  bool return_home_between_pick_place_ = false;
  bool return_home_after_pick_place_ = false;
  bool task2_capture_enabled_ = false;

  double cartesian_eef_step_ = 0.005;
  double cartesian_jump_threshold_ = 0.0;
  double cartesian_min_fraction_ = 0.98;
  double pick_offset_z_ = 0.12;
  double task3_pick_offset_z_ = 0.11;
  double place_offset_z_ = 0.35;
  double grasp_approach_offset_z_ = 0.015;
  double post_grasp_lift_z_ = 0.10;
  double gripper_grasp_width_ = 0.03;
  double joint_state_wait_timeout_sec_ = 2.0;

  double pick_hover_offset_z_ = 0.18;
  double basket_release_offset_z_ = 0.25;
  double scan_settle_time_sec_ = 1.0;
  double voxel_leaf_size_ = 0.005;
  double cluster_tolerance_ = 0.025;
  double candidate_match_distance_ = 0.10;
  double workspace_min_x_ = -0.55;
  double workspace_max_x_ = 0.85;
  double workspace_min_y_ = -0.70;
  double workspace_max_y_ = 0.70;
  double workspace_min_z_ = 0.0;
  double workspace_max_z_ = 0.55;
  double plane_distance_threshold_ = 0.01;
  double colour_min_value_ = 0.18;
  double colour_min_channel_delta_ = 0.15;

  int cluster_min_points_ = 60;
  int cluster_max_points_ = 30000;

  std::string task2_capture_dir_ = "/tmp/pick_place_task2_capture";

  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
};

#endif  // PICK_PLACE_SOLUTION_H_
