from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]


PROJECTS = [
    "PickPlace",
    "ShapeSorting",
]

ROS_PACKAGES = {
    "PickPlace": "src/pick_place_solution/",
    "ShapeSorting": "src/shape_sorting_solution/",
}


def test_root_readme_has_standard_project_sections_and_compatibility_note():
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    english = (ROOT / "README_en.md").read_text(encoding="utf-8")
    assert "## 功能说明" in readme
    assert "## 结果展示" in readme
    assert "## 快速上手" in readme
    assert "## 环境要求" in readme
    assert "## 数据说明" in readme
    assert "## Results" in english
    assert "## Features" in english
    assert "## Quick Start" in english
    assert "## Requirements" in english
    assert "## Data Notes" in english
    assert "外部 spawner 包名保留为仿真接口兼容项" in readme
    assert (ROOT / "README_en.md").is_file()
    for project in PROJECTS:
        assert project in readme


def test_showcase_preview_asset_exists_and_is_valid_svg():
    image = ROOT / "docs" / "images" / "manipulation-preview.svg"
    assert image.is_file()
    ET.parse(image)


def test_project_folder_names_are_pascal_case():
    for project in PROJECTS:
        assert "_" not in project
        assert project[0].isupper()
        assert (ROOT / project).is_dir()


def test_project_readmes_present_functional_entrypoints():
    for project in PROJECTS:
        readme = (ROOT / project / "README.md").read_text(encoding="utf-8")
        assert "## 快速上手" in readme
        assert "## 运行" in readme
        assert "## 结果展示" in readme
        assert ROS_PACKAGES[project] in readme
        assert (ROOT / project / "README_en.md").is_file()


def test_project_owned_ros_packages_use_functional_names():
    for project, ros_package in ROS_PACKAGES.items():
        assert (ROOT / project / ros_package).is_dir()
        assert "cw" not in ros_package


def test_ros_package_metadata_and_cmake_sources_are_consistent():
    for project, ros_package in ROS_PACKAGES.items():
        package_dir = ROOT / project / ros_package
        package_name = package_dir.name
        package_xml = ET.parse(package_dir / "package.xml")
        assert package_xml.findtext("name") == package_name

        cmake = (package_dir / "CMakeLists.txt").read_text(encoding="utf-8")
        assert f"project({package_name})" in cmake
        for source in re.findall(r"src/[A-Za-z0-9_]+\\.cpp", cmake):
            assert (package_dir / source).is_file()


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
