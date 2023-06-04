![0.学校logo](README.assets/0.学校logo.jpg)

## 队名：RethinkFS

我们队“RethinkFS”来自**哈尔滨工业大学（深圳）**，基本情况如下：

| 赛题     | [proj117-smart-flash-fs](https://github.com/oscomp/proj117-smart-flash-fs) |
| -------- | ------------------------------------------------------------ |
| 小组成员 | 李羿廷、梁鑫嵘                                               |
| 指导老师 | 夏文、李诗逸                                                 |

## 文档信息

- 主要文档位置：[在线浏览](https://rethinkfs.github.io/docs/)
- 进度相关信息：[进度报告](https://rethinkfs.github.io/docs/%E8%BF%9B%E5%BA%A6%E6%8A%A5%E5%91%8A/index.html)
- 初赛说明文档：[TeX 项目](https://github.com/RethinkFS/paper)
- 使用文档：[Get Started](https://rethinkfs.github.io/docs/%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3/GetStarted.html)

## 赛题完成情况

我们在初赛阶段对基于 ZNS 的文件系统 ZenFS 进行改进，拟实现一个更加智能得到文件系统 AquaFS。

初赛阶段完成情况如下：

| 改进内容                  | 完成情况                                                  | 说明                                                         |
| ------------------------- | --------------------------------------------------------- | ------------------------------------------------------------ |
| 智能调参模块              | 完成大部分                                                |                                                              |
| 基于 Zone 的动态分区 RAID | 全盘 RAID 模式、<br />分区 RAID 模式等<br />整体进度约70% | 1️⃣ 全盘 RAID 模式支持 RAID0、RAID1<br/>2️⃣ 分区 RAID 模式在建立文件系统时或空间不足时自动分配新的 Zone 映射和 RAID 逻辑<br />3️⃣ RAID0 对单线程 IO 读写有加速效果<br />4️⃣ RAID1 对读有加速效果，同时发生数据损坏时可以屏蔽坏块并自动恢复数据<br /> |

## 项目结构

```
   ┌─────────┐ ┌───────┐
   │ RocksDB │ │  App  │
   └────────┬┘ └┬──────┘
    FS/POSIX│   │VFS/FUSE
┌─AquaFS────┼───┼──────────┬──────┐
│           │   │          │Turner│
│     ┌─────┴───┴────────┐ ├──────┴─────┐
│     │   Data Router    │ │Configurator│
│     └──┬──────────────┬┘ └──────┬─────┘
│    SST │              │ Data    │
│ ┌──────┴───┐ inode ┌──┴───────┐ │
│ │ AquaZFS  │◄──────┤  ExtFS   │ │
│ └──┬───────┘       └───┬──────┘ │
│    │RAID         Extent│        │
│    │   ┌───────────────┤Data    │
│ ┌──┼───┼───────────────┼──────┐ │
│ │  │   │Zones Allocator│      │ │
│ ├──▼───▼───────────────▼──────┤ │
│ │       io_uring/xNVME        │ │
│ ├──────────────┬──────────────┤ │
│ │ Seq Zones    │ Conv Zones   │ │
└─┴──────────────┴──────────────┴─┘
```

[详细信息](https://rethinkfs.github.io/docs/%E7%A0%94%E7%A9%B6%E6%96%B9%E5%90%91/%E6%95%B4%E4%BD%93%E6%9E%B6%E6%9E%84.html)

### 文件夹说明

1. `db`：与 RocksDB 数据库相关转换接口
2. `programs`：AquaFS 相关可执行程序，如文件系统创建和检查、备份和还原、单元测试等
3. `src`：
   1. 框架源代码
   2. 提供模块之间的接口调用逻辑以及数据路由逻辑等
4. `aquafs-plugin`：
   1. 基于原 [ZenFS](https://github.com/westerndigitalcorporation/zenfs) 项目继续开发
   2. 作为我们修改后的 [RocksDB](https://github.com/RethinkFS/rocksdb) 的插件存在，便于运行数据库测试
5. `AquaZFS`：
   1. 完全抽离 RocksDB 相关依赖的独立文件系统
   2. 精简修改相关逻辑，便于系统集成
6. `ExtFS`：
   1. 使用 Rust 开发的一个简单 Ext2 rev0.0 兼容文件系统
   2. 集成进 AquaFS，作为普通数据存储后端
   3. 暂时使用 FUSE 接口
7. `docs`：通过 mdbook 构建的在线文档
8. `paper`：通过 TeX 构建的比赛提交文档