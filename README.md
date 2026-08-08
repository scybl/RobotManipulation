# RobotManipulation

[English](README_en.md)

RobotManipulation 包含两个独立的 ROS 2 Humble 机器人操作项目：`PickPlace` 和 `ShapeSorting`。

![RobotManipulation 演示预览](docs/images/manipulation-preview.svg)

## 功能说明

- `PickPlace`：从点云中定位目标，规划抓取动作，并放入指定篮筐。
- `ShapeSorting`：识别 nought/cross 物体，扫描场景并完成分类放置。
- 自有 ROS 包使用功能命名：`pick_place_solution` 和 `shape_sorting_solution`。
- 外部 spawner 包名保留为仿真接口兼容项。

## 结果展示

| 子项目 | 输出 |
| --- | --- |
| `PickPlace` | 点云目标检测、抓取和放置 |
| `ShapeSorting` | 形状识别、场景扫描和分类放置 |

## 快速上手

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

## 环境要求

- ROS 2 Humble
- MoveIt
- Gazebo
- 项目对应的 world spawner 包

## 数据说明

项目使用仿真场景，不依赖外部数据集。完整运行需要本地 ROS 2 和仿真器环境。

## 目录结构

```text
PickPlace/              抓取放置项目
ShapeSorting/           形状分类放置项目
docs/images/            README 结果图
tests/                  无 ROS 结构测试
archive/                原始材料归档
```

## 测试

```bash
pytest tests/ -q
```
