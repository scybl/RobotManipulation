# RobotManipulation

[English](README_en.md)

RobotManipulation 是一个 ROS 2 Humble 机器人操作与感知展示仓库，包含两个相互独立的仿真项目。原始提交归档保留在 `archive/`，展示入口统一使用功能命名。

| 项目 | 重点能力 | 主演示 | 环境 |
| --- | --- | --- | --- |
| `PickPlace` | 目标检测、定位、抓取与篮筐放置 | Pick-and-place 任务 1-3 | ROS 2 Humble |
| `ShapeSorting` | 点云形状识别、场景扫描与物体分类放置 | Nought/cross 操作任务 1-3 | ROS 2 Humble |

## 快速上手索引

| 目标 | 入口 |
| --- | --- |
| PickPlace 环境配置 | `cd PickPlace && bash scripts/setup_environment.sh` |
| PickPlace 启动演示 | `cd PickPlace && bash scripts/run_demo.sh launch` |
| ShapeSorting 环境配置 | `cd ShapeSorting && bash scripts/setup_environment.sh` |
| ShapeSorting 启动演示 | `cd ShapeSorting && bash scripts/run_demo.sh launch` |
| 无 ROS 结构测试 | `conda run -n codex_python pytest tests/ -q` |

## 仓库说明

- 每个子项目都有独立 README、环境脚本、运行脚本和 ROS 工作空间目录。
- 项目自有 ROS 包使用功能命名：`pick_place_solution` 与 `shape_sorting_solution`。
- 外部依赖 `cw1_world_spawner` 和 `cw2_world_spawner` 是课程仿真器暴露的服务包名，作为兼容接口保留。
- 对外展示和作品集链接请使用 `RobotManipulation`、`PickPlace`、`ShapeSorting` 这些功能名。

## 快速命令

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

## 结果快照

| 子项目 | 展示效果 |
| --- | --- |
| `PickPlace` | 从点云中检测目标，规划抓取动作，并完成放置任务 |
| `ShapeSorting` | 识别 nought/cross 形状，处理场景障碍，并完成分类放置 |
