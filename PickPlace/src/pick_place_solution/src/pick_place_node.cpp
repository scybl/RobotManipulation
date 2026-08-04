/* Pick-and-place perception entry point for the simulator solution node. */

#include <pick_place_solution.h>
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<rclcpp::Node>("pick_place_solution_node");

  PickPlaceSolution solution(node);

  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
