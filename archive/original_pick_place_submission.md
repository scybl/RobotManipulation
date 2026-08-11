# PickPlace Archived Runtime Note

This note keeps the original runtime observations for the PickPlace demo before the active package was reorganised under `src/pick_place_solution/`.

## Runtime Behaviour

The demo was exercised on both WSL and virtual-machine environments. The first two scenarios were recorded as stable. The repeated pick-and-place scenario was recorded above 90 percent stability, with occasional simulator timing issues around colour observation and placement alignment on slower machines.

## Active Package

The current maintained ROS 2 package is:

```text
pick_place_solution
```

## Build And Run

```bash
source /opt/ros/humble/setup.bash
export PATH=/usr/bin:$PATH
export RMW_FASTRTPS_USE_SHM=0
colcon build --packages-select pick_place_solution
source install/setup.bash
ros2 launch pick_place_solution run_solution.launch.py use_gazebo_gui:=true use_rviz:=true enable_realsense:=true enable_camera_processing:=true control_mode:=effort
```

Trigger the simulator scenarios from another terminal:

```bash
source /opt/ros/humble/setup.bash
source install/setup.bash
export PATH=/usr/bin:$PATH
export RMW_FASTRTPS_USE_SHM=0
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 1}"
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 2}"
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 3}"
```
