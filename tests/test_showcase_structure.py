from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[1]


PROJECTS = [
    "robot_pick_place_perception",
    "robot_shape_sorting_perception",
]


def test_root_readme_has_quick_start_index_and_compatibility_note():
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    assert "## Quick Start Index" in readme
    assert "simulator-compatible ROS package names" in readme
    for project in PROJECTS:
        assert project in readme


def test_project_readmes_present_functional_entrypoints():
    for project in PROJECTS:
        readme = (ROOT / project / "README.md").read_text(encoding="utf-8")
        assert "## One-Command Setup" in readme
        assert "## Run" in readme
        assert "## Result Snapshot" in readme
        assert "src/<simulator-compatible-ros-package>/" in readme


def test_shell_entrypoints_are_syntax_valid_without_ros():
    scripts = sorted(ROOT.glob("*/scripts/*.sh"))
    assert scripts
    for script in scripts:
        subprocess.run(["bash", "-n", str(script)], check=True)
