# ShapeSorting

[English](README_en.md)

ShapeSorting 是一个 ROS 2 Humble 机器人感知与操作项目，用于从点云中识别 nought 和 cross 物体、估计位姿，并用仿真机械臂完成分类放置。项目结合模型资产、点云签名、场景障碍处理、MoveIt 规划和任务化抓取流程。

源码包位于 `src/shape_sorting_solution/`。项目目录和 README 使用功能命名；外部 simulator spawner 包名仅在课程仿真服务接口要求时保留。

## 功能说明

| 方向 | 实现 |
| --- | --- |
| 形状资产 | nought/cross 物体的 STL 与 PCD 参考 |
| 场景理解 | 点云聚类、成对形状签名和障碍注册 |
| 运动规划 | 顶部抓取位姿、搬运路径和 MoveIt 轨迹 |
| 任务执行 | 三个 service callback 覆盖识别、分类和复杂场景 |

## 快速上手

```bash
bash scripts/setup_environment.sh
```

脚本会 source ROS 2 Humble，设置测试运行需要的环境变量，并用 `colcon` 构建包。

## 运行

一个终端启动仿真器和解法节点：

```bash
bash scripts/run_demo.sh launch
```

另一个终端触发任务：

```bash
bash scripts/run_demo.sh task 1
bash scripts/run_demo.sh task 2
bash scripts/run_demo.sh task 3
```

也可以直接传任务编号：

```bash
bash scripts/run_demo.sh 2
```

## 结果展示

| 任务 | 展示行为 | 记录稳定性 |
| --- | --- | --- |
| Task 1 | 形状识别与操作 | 超过 90% |
| Task 2 | 确定性物体处理 | 记录运行中 100% |
| Task 3 | 避障场景下抓取放置 | 超过 90% |

## 目录结构

```text
.
|-- README.md
|-- README_en.md
|-- scripts/
|   |-- setup_environment.sh
|   `-- run_demo.sh
`-- src/shape_sorting_solution/
    |-- CMakeLists.txt
    |-- package.xml
    |-- data/
    |-- launch/
    |-- include/
    |-- scripts/
    |-- src/
    `-- srv/
```
