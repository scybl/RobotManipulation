# ShapeSorting Archived Runtime Note

This note keeps the original runtime observations for the ShapeSorting demo before the active package was reorganised under `src/shape_sorting_solution/`.

## Runtime Behaviour

The demo was exercised on both WSL and virtual-machine environments. The single-object recognition and clutter-aware sorting scenarios were recorded above 90 percent stability. The reference-shape matching scenario was recorded as stable in the available runs.

## Active Package

The current maintained ROS 2 package is:

```text
shape_sorting_solution
```

## Build And Run

```bash
source /opt/ros/humble/setup.bash
export PATH=/usr/bin:$PATH
export RMW_FASTRTPS_USE_SHM=0
colcon build --packages-select shape_sorting_solution
source install/setup.bash
ros2 launch shape_sorting_solution run_solution.launch.py use_gazebo_gui:=true use_rviz:=false
```

Trigger the simulator scenarios from another terminal:

```bash
source /opt/ros/humble/setup.bash
source install/setup.bash
export PATH=/usr/bin:$PATH
export RMW_FASTRTPS_USE_SHM=0
ros2 service call /task cw2_world_spawner/srv/TaskSetup "{task_index: 1}"
ros2 service call /task cw2_world_spawner/srv/TaskSetup "{task_index: 2}"
ros2 service call /task cw2_world_spawner/srv/TaskSetup "{task_index: 3}"
```
