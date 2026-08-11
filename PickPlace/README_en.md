# PickPlace

[中文](README.md)

PickPlace is a ROS 2 Humble pick-and-place demo for object detection, localisation, grasp planning, and basket placement in a simulated workcell. It combines point-cloud filtering, plane segmentation, colour/geometric cues, MoveIt planning, gripper control, and repeated scene rescanning.

The source package is stored under `src/pick_place_solution/`. The functional project name is used at the folder and README level; external simulator spawner package names are retained only where the simulator service interface requires them.

## Features

| Area | Implementation |
| --- | --- |
| Scene perception | Realsense point-cloud processing, filtering, clustering, plane detection, and basket detection |
| Motion planning | MoveIt pose goals, Cartesian approach/retreat paths, and fallback planning options |
| Manipulation | Gripper width control, pick offsets, release offsets, and return-home safety behaviour |
| Robustness | Rescanning, retries, and post-place checks for timing-sensitive simulator scenarios |

## Quick Start

```bash
bash scripts/setup_environment.sh
```

The script sources ROS 2 Humble, sets runtime environment variables, and builds `pick_place_solution` with `colcon`.

## Run

Open one terminal for the simulator and solution node:

```bash
bash scripts/run_demo.sh launch
```

Open another terminal to trigger simulator scenarios:

```bash
bash scripts/run_demo.sh task 1
bash scripts/run_demo.sh task 2
bash scripts/run_demo.sh task 3
```

You can also pass the scenario number directly:

```bash
bash scripts/run_demo.sh 1
```

## Results

| Scenario | Demonstrated behaviour | Runtime record |
| --- | --- | --- |
| Scenario 1 | Grasp and place known objects | Stable completion |
| Scenario 2 | Scan, localise, and manipulate objects | Stable completion |
| Scenario 3 | Repeated pick-and-place with rescanning | Recorded above 90 percent stability; more sensitive to simulator timing and colour observations |

Runtime flow image: `../docs/images/pick-place-run.svg`.

## Project Layout

```text
.
|-- README.md
|-- README_en.md
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
