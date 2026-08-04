#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PACKAGE_NAME="cw1_team_14"

if [[ ! -f /opt/ros/humble/setup.bash ]]; then
  echo "ROS 2 Humble was not found at /opt/ros/humble/setup.bash."
  echo "Install or source ROS 2 Humble before running this project."
  exit 1
fi

source /opt/ros/humble/setup.bash
export PATH="/usr/bin:$PATH"
export RMW_FASTRTPS_USE_SHM="${RMW_FASTRTPS_USE_SHM:-0}"

if ! command -v colcon >/dev/null 2>&1; then
  echo "Missing command: colcon"
  echo "Install python3-colcon-common-extensions, then rerun this script."
  exit 1
fi

cd "$ROOT"
colcon build --symlink-install --packages-select "$PACKAGE_NAME"

echo "Build complete. Run: bash scripts/run_demo.sh launch"
