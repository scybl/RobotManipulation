# ShapeSorting

[中文](README.md)

ShapeSorting is a ROS 2 Humble perception-and-manipulation demo for recognising nought and cross objects from point clouds, estimating their poses, and sorting them with a simulated robot arm. It combines STL/PCD shape assets, point-cloud signatures, obstacle registration, MoveIt planning, and structured pick-and-place routines.

The source package is stored under `src/shape_sorting_solution/`. The functional project name is used at the folder and README level; external simulator spawner package names are retained only where the simulator service interface requires them.

## Features

| Area | Implementation |
| --- | --- |
| Shape assets | STL and PCD references for nought/cross objects |
| Scene understanding | Point-cloud clustering, pairwise shape signatures, colour filtering, and obstacle registration |
| Motion planning | Top-down grasp poses, carry/place paths, obstacle-aware scenes, and MoveIt trajectories |
| Scenario execution | Three service callbacks covering recognition, sorting, and cluttered scenes |

## Quick Start

```bash
bash scripts/setup_environment.sh
```

The script sources ROS 2 Humble, sets runtime environment variables, and builds `shape_sorting_solution` with `colcon`.

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
bash scripts/run_demo.sh 2
```

## Results

| Scenario | Demonstrated behaviour | Runtime record |
| --- | --- | --- |
| Scenario 1 | Single nought/cross recognition and manipulation | Recorded above 90 percent stability |
| Scenario 2 | Reference-shape comparison and target classification | Stable completion in recorded runs |
| Scenario 3 | Clutter-aware pick-and-place with sorted placement | Recorded above 90 percent stability |

Runtime flow image: `../docs/images/shape-sorting-run.svg`.

## Project Layout

```text
.
|-- README.md
|-- README_en.md
|-- scripts/
|   |-- setup_environment.sh
|   `-- run_demo.sh
`-- src/shape_sorting_solution/
    |-- CMakeLists.txt
    |-- package.xml
    |-- data/
    |-- launch/
    |-- include/
    |-- scripts/
    |-- src/
    `-- srv/
```
