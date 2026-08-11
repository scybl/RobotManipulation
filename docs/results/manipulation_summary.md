# RobotManipulation Runtime Summary

## Demo Matrix

| Demo | ROS package | Primary loop | Run entry |
| --- | --- | --- | --- |
| PickPlace | `pick_place_solution` | point-cloud filtering, target localisation, grasp planning, basket placement | `bash scripts/run_project.sh pick-place launch` |
| ShapeSorting | `shape_sorting_solution` | object scan, nought/cross recognition, scene registration, sorted placement | `bash scripts/run_project.sh shape-sorting launch` |

## Recorded Behaviour

| Demo | Scenario 1 | Scenario 2 | Scenario 3 |
| --- | --- | --- | --- |
| PickPlace | known-object grasp and basket placement | scan, localise, and manipulate targets | repeated pick-and-place with rescanning |
| ShapeSorting | recognise and move a single object | compare reference shapes and classify the target | handle clutter, obstacles, and sorted placement |

## Verification

```bash
bash scripts/run_project.sh summary
pytest tests/ -q
```

Full simulator runs require ROS 2 Humble, MoveIt, Gazebo, and the matching simulator spawner packages.
