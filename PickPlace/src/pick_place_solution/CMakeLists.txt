cmake_minimum_required(VERSION 3.8)
project(pick_place_solution)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(moveit_core REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_sensor_msgs REQUIRED)
find_package(pcl_ros REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED)
find_package(shape_msgs REQUIRED)
find_package(cw1_world_spawner REQUIRED)

include_directories(
  include
  ${PCL_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}_pick_place_solution_lib
  src/pick_place_solution.cpp
)

ament_target_dependencies(${PROJECT_NAME}_pick_place_solution_lib
  rclcpp
  std_msgs
  geometry_msgs
  sensor_msgs
  moveit_core
  moveit_msgs
  moveit_ros_planning_interface
  tf2
  tf2_ros
  tf2_geometry_msgs
  tf2_sensor_msgs
  pcl_ros
  pcl_conversions
  shape_msgs
  cw1_world_spawner
)

target_link_libraries(${PROJECT_NAME}_pick_place_solution_lib
  ${PCL_LIBRARIES}
)

add_executable(pick_place_solution_node src/pick_place_node.cpp)

ament_target_dependencies(pick_place_solution_node
  rclcpp
  std_msgs
  geometry_msgs
  sensor_msgs
  moveit_core
  moveit_msgs
  moveit_ros_planning_interface
  tf2
  tf2_ros
  tf2_geometry_msgs
  tf2_sensor_msgs
  pcl_ros
  pcl_conversions
  shape_msgs
  cw1_world_spawner
)

target_link_libraries(pick_place_solution_node
  ${PROJECT_NAME}_pick_place_solution_lib
)

install(TARGETS
  ${PROJECT_NAME}_pick_place_solution_lib
  pick_place_solution_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
  PATTERN "*.launch" EXCLUDE
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
