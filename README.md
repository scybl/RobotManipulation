# RobotManipulation

RobotManipulation is organised as two independent ROS 2 robotics projects. The original submission archives are preserved under `archive/`, while the showcase entry points use functional project names.

| Project | Focus | Main demo | Environment |
| --- | --- | --- | --- |
| `PickPlace` | Object detection, localisation, grasping, and basket placement | Pick-and-place tasks 1-3 | ROS 2 Humble |
| `ShapeSorting` | Point-cloud shape recognition, scene scanning, and object sorting | Nought/cross manipulation tasks 1-3 | ROS 2 Humble |

## Quick Start Index

| Need | Start here |
| --- | --- |
| Pick-and-place setup | `cd PickPlace && bash scripts/setup_environment.sh` |
| Pick-and-place launch | `cd PickPlace && bash scripts/run_demo.sh launch` |
| Shape-sorting setup | `cd ShapeSorting && bash scripts/setup_environment.sh` |
| Shape-sorting launch | `cd ShapeSorting && bash scripts/run_demo.sh launch` |
| Structural tests without ROS | `conda run -n codex_python pytest tests/ -q` |

## Repository Notes

- Each project has its own README, setup script, run script, and ROS workspace folders.
- Project-owned ROS packages under `src/` use functional package names. External `cw1_world_spawner` and `cw2_world_spawner` dependencies are retained because the supplied simulator services expose those package names.
- Use the functional folder names for presentation and portfolio links.

## Quick Commands

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
