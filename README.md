# OS-File-Management

简易文件系统模拟 — 操作系统课程项目。在内存中实现一个完整的虚拟文件系统，支持 FAT 显式链接分配、位图空闲空间管理、多级树状目录。

## 快速开始

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
./osfs.exe
```

或在 VS Code 中用 **CMake Tools** 插件：`Ctrl+Shift+P` → CMake: Configure → CMake: Build。

## 命令列表

### 基础

| 命令 | 用法 | 说明 |
|------|------|------|
| `format` | `format` | 格式化虚拟磁盘，清除所有数据 |
| `help` | `help` | 显示全部命令及用法 |
| `exit` | `exit` | 退出并保存虚拟磁盘到 `data/fs.fsimg` |

### 目录操作

| 命令 | 用法 | 说明 |
|------|------|------|
| `mkdir` | `mkdir <name>` | 创建子目录 |
| `rmdir` | `rmdir <name>` | 删除空目录（非空目录拒绝删除） |
| `ls` | `ls [-l]` | 列出当前目录内容，`-l` 显示大小和时间 |
| `cd` | `cd <path>` | 切换工作目录，支持 `..` 和绝对路径 `/a/b` |

### 文件操作

| 命令 | 用法 | 说明 |
|------|------|------|
| `create` | `create <name>` | 创建空文件 |
| `open` | `open <name>` | 打开文件，返回文件描述符 fd |
| `close` | `close <fd>` | 关闭文件描述符，自动回写大小到磁盘 |
| `read` | `read <fd> <size>` | 从文件读取指定字节数 |
| `write` | `write <fd> <text>` | 向文件写入文本，支持含空格内容（用引号括起） |
| `delete` | `delete <name>` | 删除文件，回收全部数据块 |

### 查询

| 命令 | 用法 | 说明 |
|------|------|------|
| `cat` | `cat <name>` | 打印文件全部内容到控制台 |
| `stat` | `stat <name>` | 显示文件/目录的详细信息（首块号、大小、块数、时间） |
| `df` | `df` | 查看磁盘空间：总大小、已用、剩余、使用率（含进度条） |
| `du` | `du <name>` | 查看文件实际占用块数/字节数及内部碎片 |
| `tree` | `tree` | 以树形结构递归展示整个目录 |

### 修改

| 命令 | 用法 | 说明 |
|------|------|------|
| `rename` | `rename <old> <new>` | 重命名文件或目录，别名 `ren` / `mv` |

## 技术规格

```
虚拟磁盘:  1MB (1024 blocks × 1024 bytes)
物理结构:  FAT 显式链接 (FAT[block] = next | EOF | FREE)
空闲管理:  Bitmap (1 bit per block)
目录结构:  多级树状目录，目录项含 name/first_block/size/time
持久化:    data/fs.fsimg — 1MB 二进制镜像，退出时保存，启动时恢复
```

```
block 0        block 1~4      block 5       block 6~1023
[SuperBlock]   [FAT 表]       [Bitmap]      [数据区 1018 blocks]
```

## 项目结构

```
include/         头文件 (9 个模块)
src/             源文件 (12 个 .cpp)
  main.cpp       入口
  shell.cpp      Shell 交互主循环 (OOP)
  commands.cpp   19 条命令实现 (Command 模式)
  fs_core.cpp    FileSystem 核心调度
  disk_io.cpp    虚拟磁盘 + 持久化
  fat.cpp        FAT 表操作
  bitmap.cpp     位图操作
  directory.cpp  目录管理
  file_op.cpp    文件操作 + fd 管理
  ui.cpp         终端 ANSI 美化
  utils.cpp      工具函数
example/         可选测试用例
images/          运行截图
```

