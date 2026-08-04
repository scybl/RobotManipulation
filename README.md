# ROS Manipulation Perception Showcase

This folder is organised as two independent ROS 2 robotics projects. The original submission archives are preserved under `archive/`, while the showcase entry points use functional project names.

| Project | Focus | Main demo | Environment |
| --- | --- | --- | --- |
| `robot_pick_place_perception` | Object detection, localisation, grasping, and basket placement | Pick-and-place tasks 1-3 | ROS 2 Humble |
| `robot_shape_sorting_perception` | Point-cloud shape recognition, scene scanning, and object sorting | Nought/cross manipulation tasks 1-3 | ROS 2 Humble |

## Quick Start Index

| Need | Start here |
| --- | --- |
| Pick-and-place setup | `cd robot_pick_place_perception && bash scripts/setup_environment.sh` |
| Pick-and-place launch | `cd robot_pick_place_perception && bash scripts/run_demo.sh launch` |
| Shape-sorting setup | `cd robot_shape_sorting_perception && bash scripts/setup_environment.sh` |
| Shape-sorting launch | `cd robot_shape_sorting_perception && bash scripts/run_demo.sh launch` |
| Structural tests without ROS | `conda run -n codex_python pytest tests/ -q` |

## Repository Notes

- Each project has its own README, setup script, run script, and ROS workspace folders.
- The simulator-compatible ROS package names under `src/` are retained because the supplied task spawner interfaces depend on them.
- Use the functional folder names for presentation and portfolio links.

## Quick Commands

```bash
cd robot_pick_place_perception
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```

```bash
cd robot_shape_sorting_perception
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```
