# COMP0250 Coursework 1  

Github repo: https://github.com/scybl/250Coursework.git

This coursework has been run on both WSL and virtual machines. The success rate of the challenges exceeded 90%, but depending on the performance of different devices, Task 3 had a very small number of bugs such as color mismatch and placement misalignment. It was perfectly successful in environments where one of the three team members was present. Therefore, in such cases, we hope that the instructor and teaching assistants can run the code multiple times to observe the results and structure.

## Pick and Place, Object Detection and Localisation  

---

## 📌 Authors  

- Student A (Zhenggang Chen) Total time consume: 2+5+5+3
  * 2 hours were spent in the lab setting up the environment and running test code, as well as asking questions to the teaching assistants and instructors.
  * 5 hours to complete Task 1 (AI statement: The first one hour was spent having ChatGPT5.3 generate the code framework. Two hours were for manual debugging of code and parameters. The remaining two hours were spent communicating with student A and student B in the lab. The entire Task 1 took five hours to complete.)
  * 5 hours for the basic framework of Task 2 and Task 3
- Student B (Yihan Wang) Total time consume: 5+5+3
  * 5 hours of refactoring and encapsulation code;
  * 5 hours of optimization for Task 2 and Task 3 logic. Task 3 implements a rescan of the plane after each grab to avoid issues caused by old trajectories resulting from collisions with other objects during each grab.
- Student C (Bingze Li) Total time consume: 5+5+3
  * 5 hours of optimization for Task1's garsping operation;
  * 5 hours of optimization for Task2's scanning operation.
- All members
  * 3 hours for code integration, testing, debugging, checking the rules of instruction, and writing this document.

---

## 📦 Package  

This submission contains the ROS2 package:

cw1_team_14

All solution code is implemented inside this package.

---

## ⚙️ build and run the package 

This project requires ROS2 Humble

```bash
# bash 1
source /opt/ros/humble/setup.bash
source install/setup.bash
export PATH=/usr/bin:$PATH 
export RMW_FASTRTPS_USE_SHM=0
colcon build --packages-select cw1_team_14
source /opt/ros/humble/setup.bash
ros2 launch cw1_team_14 run_solution.launch.py  use_gazebo_gui:=true use_rviz:=true  enable_realsense:=true enable_camera_processing:=true  control_mode:=effort
# bash 2
source /opt/ros/humble/setup.bash
source install/setup.bash
export PATH=/usr/bin:$PATH 
export RMW_FASTRTPS_USE_SHM=0
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 1}"
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 2}"
ros2 service call /task cw1_world_spawner/srv/TaskSetup "{task_index: 3}"
