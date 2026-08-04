#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="${1:-launch}"
TASK_INDEX="${2:-1}"

if [[ "$MODE" =~ ^[123]$ ]]; then
  TASK_INDEX="$MODE"
  MODE="task"
fi

if [[ ! -f /opt/ros/humble/setup.bash ]]; then
  echo "ROS 2 Humble was not found at /opt/ros/humble/setup.bash."
  exit 1
fi

source /opt/ros/humble/setup.bash
export PATH="/usr/bin:$PATH"
export RMW_FASTRTPS_USE_SHM="${RMW_FASTRTPS_USE_SHM:-0}"

if [[ -f "$ROOT/install/setup.bash" ]]; then
  source "$ROOT/install/setup.bash"
else
  echo "Local workspace is not built yet. Run: bash scripts/setup_environment.sh"
  exit 1
fi

case "$MODE" in
  launch)
    ros2 launch cw2_team_14 run_solution.launch.py \
      use_gazebo_gui:=true \
      use_rviz:=false
    ;;
  task)
    ros2 service call /task cw2_world_spawner/srv/TaskSetup "{task_index: ${TASK_INDEX}}"
    ;;
  help|-h|--help)
    echo "Usage: bash scripts/run_demo.sh [launch|task TASK_INDEX|1|2|3]"
    ;;
  *)
    echo "Unknown mode: $MODE"
    echo "Usage: bash scripts/run_demo.sh [launch|task TASK_INDEX|1|2|3]"
    exit 1
    ;;
esac
