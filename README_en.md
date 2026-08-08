# RobotManipulation

[中文](README.md)

RobotManipulation contains two independent ROS 2 Humble manipulation projects: `PickPlace` and `ShapeSorting`.

![RobotManipulation preview](docs/images/manipulation-preview.svg)

## Features

- `PickPlace`: localises a target from point clouds, plans a grasp, and places it into a basket.
- `ShapeSorting`: recognises nought/cross objects, scans the scene, and sorts objects by class.
- Owned ROS packages use functional names: `pick_place_solution` and `shape_sorting_solution`.
- External spawner package names are retained only for simulator compatibility.

## Results

| Subproject | Output |
| --- | --- |
| `PickPlace` | point-cloud target detection, grasping, and placement |
| `ShapeSorting` | shape recognition, scene scanning, and sorted placement |

## Quick Start

```bash
cd PickPlace
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```

```bash
cd ShapeSorting
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```

## Requirements

- ROS 2 Humble
- MoveIt
- Gazebo
- The matching world spawner packages

## Data Notes

The project uses simulated scenes and does not require an external dataset. Full runs require a local ROS 2 and simulator environment.

## Project Layout

```text
PickPlace/              Pick-and-place project
ShapeSorting/           Shape-sorting project
docs/images/            README result image
tests/                  ROS-free structure tests
archive/                Original material archive
```

## Tests

```bash
pytest tests/ -q
```
