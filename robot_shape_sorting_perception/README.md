# ROS 2 Shape Sorting and Scene Perception

This project implements a ROS 2 Humble manipulation pipeline for recognising nought and cross objects from point clouds, estimating their pose, and sorting them with a simulated robot arm. It combines model assets, point-cloud signatures, octomap-aware scene handling, MoveIt planning, and task-specific pick-and-place routines.

The source package is stored under `src/`. The functional project name is used at the folder and README level; the internal ROS package identifier is retained for compatibility with the supplied task spawner interfaces.

## Highlights

| Area | Implementation |
| --- | --- |
| Shape assets | STL and PCD references for nought/cross objects |
| Scene understanding | Point-cloud clustering, pairwise shape signatures, and obstacle registration |
| Motion planning | MoveIt trajectories with top-down grasp poses and carry/place waypoints |
| Task execution | Three task service callbacks covering recognition, sorting, and cluttered scenes |

## One-Command Setup

```bash
bash scripts/setup_environment.sh
```

The script sources ROS 2 Humble, sets the runtime environment variables used during testing, and builds the package with `colcon`.

## Run

Open one terminal for the simulator and solution node:

```bash
bash scripts/run_demo.sh launch
```

Open another terminal to trigger a task:

```bash
bash scripts/run_demo.sh task 1
bash scripts/run_demo.sh task 2
bash scripts/run_demo.sh task 3
```

You can also pass the task number directly:

```bash
bash scripts/run_demo.sh 2
```

## Result Snapshot

| Task | Demonstrated behaviour | Reported stability |
| --- | --- | --- |
| Task 1 | Shape recognition and manipulation | Above 90 percent |
| Task 2 | Deterministic object handling | 100 percent in the recorded team runs |
| Task 3 | Clutter-aware pick-and-place | Above 90 percent |

## Layout

```text
.
|-- README.md
|-- scripts/
|   |-- setup_environment.sh
|   `-- run_demo.sh
`-- src/<simulator-compatible-ros-package>/
    |-- CMakeLists.txt
    |-- package.xml
    |-- data/
    |-- launch/
    |-- include/
    |-- scripts/
    |-- src/
    `-- srv/
```
