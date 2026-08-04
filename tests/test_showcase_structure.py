from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[1]


PROJECTS = [
    "PickPlace",
    "ShapeSorting",
]

ROS_PACKAGES = {
    "PickPlace": "src/pick_place_solution/",
    "ShapeSorting": "src/shape_sorting_solution/",
}


def test_root_readme_has_quick_start_index_and_compatibility_note():
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    assert "## Quick Start Index" in readme
    assert "External `cw1_world_spawner` and `cw2_world_spawner`" in readme
    for project in PROJECTS:
        assert project in readme


def test_project_folder_names_are_pascal_case():
    for project in PROJECTS:
        assert "_" not in project
        assert project[0].isupper()
        assert (ROOT / project).is_dir()


def test_project_readmes_present_functional_entrypoints():
    for project in PROJECTS:
        readme = (ROOT / project / "README.md").read_text(encoding="utf-8")
        assert "## One-Command Setup" in readme
        assert "## Run" in readme
        assert "## Result Snapshot" in readme
        assert ROS_PACKAGES[project] in readme


def test_project_owned_ros_packages_use_functional_names():
    for project, ros_package in ROS_PACKAGES.items():
        assert (ROOT / project / ros_package).is_dir()
        assert "cw" not in ros_package


def test_shell_entrypoints_are_syntax_valid_without_ros():
    scripts = sorted(ROOT.glob("*/scripts/*.sh"))
    assert scripts
    for script in scripts:
        subprocess.run(["bash", "-n", str(script)], check=True)


def test_own_package_names_do_not_use_coursework_prefixes():
    blocked_patterns = ["cw1" + "_team_", "cw2" + "_team_"]
    searchable_files = [
        path
        for path in ROOT.rglob("*")
        if path.is_file()
        and not any(part in {".git", "__pycache__", "archive"} for part in path.parts)
        and path.suffix not in {".STL", ".pcd", ".pyc"}
    ]
    for path in searchable_files:
        text = path.read_text(encoding="utf-8", errors="ignore")
        for pattern in blocked_patterns:
            assert pattern not in text
