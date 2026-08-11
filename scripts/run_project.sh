#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="${1:-summary}"

usage() {
  cat <<'EOF'
Usage:
  bash scripts/run_project.sh [mode] [demo-args...]

Modes:
  summary                 Print the ROS-free project summary.
  pick-place [args...]    Forward args to PickPlace/scripts/run_demo.sh.
  shape-sorting [args...] Forward args to ShapeSorting/scripts/run_demo.sh.
  help                    Show this help message.

Examples:
  bash scripts/run_project.sh summary
  bash scripts/run_project.sh pick-place launch
  bash scripts/run_project.sh pick-place task 1
  bash scripts/run_project.sh shape-sorting launch
  bash scripts/run_project.sh shape-sorting task 3
EOF
}

print_summary() {
  cat <<'EOF'
RobotManipulation

PickPlace
  Package: src/pick_place_solution
  Pipeline: point cloud -> target pose -> MoveIt grasp -> basket placement
  Entry: bash scripts/run_project.sh pick-place launch

ShapeSorting
  Package: src/shape_sorting_solution
  Pipeline: scene scan -> nought/cross recognition -> obstacle-aware placement
  Entry: bash scripts/run_project.sh shape-sorting launch

Validation
  ROS-free: pytest tests/ -q
  ROS 2 Humble: run each demo after building its local workspace
EOF
}

run_demo() {
  local project_dir="$1"
  shift
  local args=("$@")
  if [[ "${#args[@]}" -eq 0 ]]; then
    args=("launch")
  fi

  cd "$ROOT_DIR/$project_dir"
  bash scripts/run_demo.sh "${args[@]}"
}

case "$MODE" in
  summary)
    print_summary
    ;;
  pick-place|PickPlace|pickplace)
    shift || true
    run_demo "PickPlace" "$@"
    ;;
  shape-sorting|ShapeSorting|shapesorting)
    shift || true
    run_demo "ShapeSorting" "$@"
    ;;
  help|-h|--help)
    usage
    ;;
  *)
    echo "Unknown mode: $MODE"
    echo
    usage
    exit 1
    ;;
esac
