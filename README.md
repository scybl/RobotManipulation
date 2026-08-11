# RobotManipulation

[English](README_en.md)

RobotManipulation 是一个 ROS 2 Humble 机器人操作工作空间，整合 `PickPlace` 和 `ShapeSorting` 两个仿真操作演示。项目覆盖点云感知、目标定位、MoveIt 运动规划、夹爪控制、场景重扫、形状识别和分类放置。

外部 spawner 包名保留为仿真接口兼容项；自有 ROS 包使用功能命名：`pick_place_solution` 和 `shape_sorting_solution`。

| 子项目 | 任务 | 感知输入 | 规划与执行 |
| --- | --- | --- | --- |
| `PickPlace` | 将目标物体抓取并放入指定篮筐 | Realsense 点云、仿真场景 | 位姿估计、MoveIt 抓取规划、夹爪控制 |
| `ShapeSorting` | 识别 nought/cross 并分类放置 | 场景点云、STL/PCD 参考形状 | 形状匹配、障碍注册、避障规划和分类放置 |

## 功能说明

| 模块 | 能力 |
| --- | --- |
| `PickPlace` | 从 Realsense 点云中过滤桌面、定位目标和篮筐，规划抓取与放置动作 |
| `ShapeSorting` | 识别 nought/cross 物体，估计位姿，注册障碍物并完成分类放置 |
| 运动规划 | 使用 MoveIt 位姿目标、笛卡尔接近/撤离路径和失败回退 |
| 稳定性处理 | 通过重扫、重试、放置偏移和回 home 行为降低仿真时序波动 |
| 项目入口 | 根级 `scripts/run_project.sh` 统一转发两个子演示，`summary` 模式不依赖 ROS |

## 运行过程展示

| 子项目 | 运行流程 | 异常处理 |
| --- | --- | --- |
| `PickPlace` | 采集点云 → 过滤桌面 → 定位目标与篮筐 → 规划接近和抓取 → 放置目标 | 执行失败时重新扫描、调整放置偏移并回到 home 位姿 |
| `ShapeSorting` | 扫描场景 → 分割目标 → 匹配 nought/cross 参考形状 → 注册障碍物 → 规划分类放置 | 识别或规划失败时重扫、重试，并使用备用规划路径 |

## 结果展示

| 子项目 | 运行场景 | 关键输出 | 本地记录表现 |
| --- | --- | --- | --- |
| `PickPlace` | 已知物体抓放、扫描定位抓放、重复抓放 | 目标点云簇、篮筐位置、抓取位姿、放置结果 | 三个场景在课程验收阶段均记录为可完成 |
| `ShapeSorting` | 单物体识别、参考形状匹配、避障分类放置 | nought/cross 分类、障碍注册、分类篮筐放置结果 | 三个场景在课程验收阶段均记录为可完成 |

> **结果数据说明**：上表"本地记录表现"来自课程作业验收阶段的运行记录（详见 `archive/`），并非本仓库自动生成或可一键复现的指标——早期版本曾写"稳定性超过90%"这类具体数字，但仓库内没有配套的日志、统计脚本或视频能重新推导出该数字，因此这里改为如实描述"记录为可完成"，不再给出未经验证的百分比。若需要量化复现，需要在下方"环境要求"里提到的仿真依赖齐备的前提下重新采集数据。

详细结果摘要保存在 `docs/results/manipulation_summary.md`。仿真运行过程本身用上述表格和文字描述，不内嵌截图——因为 `PickPlace`/`ShapeSorting` 依赖的私有仿真包缺失，无法在当前环境重新生成真实的运行截图。

不过 `ShapeSorting` 实际用于形状匹配的参考点云数据是随仓库提供的，不依赖 ROS/Gazebo/私有包即可读取，因此下图是真实数据：

<p align="center">
  <img src="docs/images/reference-shapes-pointcloud.png" alt="ShapeSorting 真实 nought/cross 参考点云（x-z 正面投影）" width="760">
</p>

上图由 `scripts/generate_reference_shape_visuals.py` 直接解析 `ShapeSorting/src/shape_sorting_solution/data/{nought,cross}_40mm.pcd`（二进制 PCD v0.7，每个约10万点）生成，取 x-z 正面投影（y 为约40mm的挤出厚度），可复现，不是仿真截图，也不是手绘示意图。

## 快速上手

不依赖 ROS 的项目摘要：

```bash
bash scripts/run_project.sh summary
```

构建并运行 PickPlace：

```bash
cd PickPlace
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```

构建并运行 ShapeSorting：

```bash
cd ShapeSorting
bash scripts/setup_environment.sh
bash scripts/run_demo.sh launch
```

也可以通过根级入口转发：

```bash
bash scripts/run_project.sh pick-place launch
bash scripts/run_project.sh shape-sorting launch
```

触发仿真场景服务：

```bash
bash scripts/run_project.sh pick-place task 1
bash scripts/run_project.sh pick-place task 2
bash scripts/run_project.sh pick-place task 3
bash scripts/run_project.sh shape-sorting task 1
bash scripts/run_project.sh shape-sorting task 2
bash scripts/run_project.sh shape-sorting task 3
```

## 环境要求

- ROS 2 Humble
- MoveIt
- Gazebo
- PCL、TF2、ros2_control 相关依赖
- 对应的 world spawner 仿真接口包：`PickPlace` 依赖 `cw1_world_spawner`，`ShapeSorting` 依赖 `cw2_world_spawner`。**这两个包是课程私有资产，未包含在本仓库中**，外部环境无法直接获取；因此本仓库定位为"代码架构与实现思路展示"，不保证第三方能够开箱即用地完整运行仿真。若需要复现，需要自行实现或替换等效的仿真世界生成节点。

## 数据说明

项目使用仿真场景和本地形状资产，不依赖外部数据集。`ShapeSorting/src/shape_sorting_solution/data/` 包含 nought/cross 的 STL 与 PCD 参考资产；完整运行需要本地 ROS 2 和仿真器环境，但读取并可视化这两个 PCD 文件本身不需要 ROS，运行 `python scripts/generate_reference_shape_visuals.py`（仅依赖 numpy/matplotlib）即可重新生成上面"结果展示"里的参考点云图。

## 目录结构

```text
PickPlace/                              抓取放置演示
ShapeSorting/                           形状分类放置演示
scripts/run_project.sh                  根级运行入口
docs/images/                            真实参考点云图（README内嵌）与历史演示资产
docs/results/manipulation_summary.md    结果摘要
tests/                                  无 ROS 结构测试
archive/                                原始材料归档
```

## 测试

```bash
pytest tests/ -q
```
