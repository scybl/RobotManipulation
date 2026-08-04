# PickPlace

[English](README_en.md)

PickPlace 是一个 ROS 2 Humble 机器人操作项目，用于在仿真工作站中完成目标检测、定位、抓取规划和放置执行。项目结合点云滤波、MoveIt 规划、夹爪控制和重复场景扫描，覆盖三个 pick-and-place 任务。

源码包位于 `src/pick_place_solution/`。项目目录和 README 使用功能命名；外部 simulator spawner 包名仅在课程仿真服务接口要求时保留。

## 亮点

| 方向 | 实现 |
| --- | --- |
| 场景感知 | Realsense 点云处理、滤波、聚类、平面/篮筐检测 |
| 运动规划 | MoveIt 位姿目标、笛卡尔接近/撤离路径和失败回退 |
| 机器人操作 | 夹爪宽度控制、抓取偏移、释放偏移和安全回 home |
| 稳定性 | 对仿真时序敏感任务提供重扫和重试控制 |

## 一键配置

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
bash scripts/run_demo.sh 1
```

## 结果快照

| 任务 | 展示行为 | 记录稳定性 |
| --- | --- | --- |
| Task 1 | 抓取并放置已知物体 | WSL/VM 运行中成功率高 |
| Task 2 | 扫描、定位并操作物体 | WSL/VM 运行中成功率高 |
| Task 3 | 带重扫的重复抓取放置 | 超过 90%，偶有仿真时序/颜色不匹配 |

## 结构

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
