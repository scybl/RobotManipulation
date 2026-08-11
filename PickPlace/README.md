# PickPlace

[English](README_en.md)

PickPlace 是一个 ROS 2 Humble 抓取放置演示，用于在仿真工作站中完成目标检测、定位、抓取规划和篮筐放置。项目结合点云滤波、平面分割、颜色/几何线索、MoveIt 规划、夹爪控制和重复场景重扫。

源码包位于 `src/pick_place_solution/`。项目目录和 README 使用功能命名；外部 simulator spawner 包名仅在仿真服务接口要求时保留。

## 功能说明

| 方向 | 实现 |
| --- | --- |
| 场景感知 | Realsense 点云处理、滤波、聚类、平面检测和篮筐检测 |
| 运动规划 | MoveIt 位姿目标、笛卡尔接近/撤离路径和失败回退 |
| 机器人操作 | 夹爪宽度控制、抓取偏移、释放偏移和安全回 home |
| 稳定性 | 对仿真时序敏感场景提供重扫、重试和放置后复检 |

## 快速上手

```bash
bash scripts/setup_environment.sh
```

脚本会 source ROS 2 Humble，设置运行环境变量，并用 `colcon` 构建 `pick_place_solution`。

## 运行

一个终端启动仿真器和解法节点：

```bash
bash scripts/run_demo.sh launch
```

另一个终端触发场景服务：

```bash
bash scripts/run_demo.sh task 1
bash scripts/run_demo.sh task 2
bash scripts/run_demo.sh task 3
```

也可以直接传场景编号：

```bash
bash scripts/run_demo.sh 1
```

## 结果展示

| 场景 | 展示行为 | 运行记录 |
| --- | --- | --- |
| 场景 1 | 抓取并放置已知物体 | 稳定完成 |
| 场景 2 | 扫描、定位并操作物体 | 稳定完成 |
| 场景 3 | 带重扫的重复抓取放置 | 记录稳定性超过 90%，对仿真时序和颜色观测更敏感 |

运行流程图见 `../docs/images/pick-place-run.svg`。

## 目录结构

```text
.
|-- README.md
|-- README_en.md
|-- scripts/
|   |-- setup_environment.sh
|   `-- run_demo.sh
`-- src/pick_place_solution/
    |-- CMakeLists.txt
    |-- package.xml
    |-- launch/
    |-- include/
    |-- src/
    `-- srv/
```
