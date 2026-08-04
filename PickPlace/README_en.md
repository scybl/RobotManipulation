# PickPlace

[中文](README.md)

PickPlace implements a ROS 2 Humble manipulation pipeline for object detection, localisation, grasp planning, and pick-and-place execution in a simulated robot workcell. It uses point-cloud filtering, MoveIt planning, gripper control, and repeated scene rescanning to solve three object-manipulation tasks.

The source package is stored under `src/pick_place_solution/`. The functional project name is used at the folder and README level; external simulator spawner package names are retained only where the supplied task services require them.

## Highlights

| Area | Implementation |
| --- | --- |
| Scene perception | Realsense point-cloud processing, filtering, clustering, and plane/basket detection |
| Motion planning | MoveIt pose goals, Cartesian approach/retreat paths, and fallback planning options |
| Manipulation | Gripper width control, pick offsets, release offsets, and return-home safety behaviour |
| Robustness | Rescanning and retry controls for task runs that are sensitive to simulator timing |

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
bash scripts/run_demo.sh 1
```

## Result Snapshot

| Task | Demonstrated behaviour | Reported stability |
| --- | --- | --- |
| Task 1 | Grasp and place known objects | High success in WSL/VM runs |
| Task 2 | Scan, localise, and manipulate objects | High success in WSL/VM runs |
| Task 3 | Repeated pick-and-place with rescanning | Above 90 percent, with rare simulator timing/color mismatch issues |

## Layout

```text
.
|-- README.md
|-- scripts/
|   |-- setup_environment.sh
|   `-- run_demo.sh
`-- src/pick_place_solution/
    |-- CMakeLists.txt
    |-- package.xml
    |-- launch/
    |-- include/
    |-- src/
    `-- srv/
```
