#pragma once
#include "utils.h"
#include "disk_io.h"
#include "fat.h"
#include "bitmap.h"
#include "directory.h"
#include "file_op.h"
#include <string>
#include <vector>

class FileSystem {
public:
    FileSystem();

    bool start();   // 启动：加载已有镜像或创建新磁盘
    void format();  // 强制格式化（不加载已有镜像）

    // 目录操作
    bool mkdir(const std::string& name);
    bool rmdir(const std::string& name);
    bool cd(const std::string& path);
    std::vector<DirEntry> ls() const;
    std::string pwd() const;

    // 文件操作
    int  create(const std::string& name);
    int  open(const std::string& name);
    bool close(int fd);
    int  read(int fd, void* buf, uint32_t size);
    int  write(int fd, const void* data, uint32_t size);
    bool remove(const std::string& name);

    // 扩展命令
    void df() const;
    void du(const std::string& name) const;
    void cat(const std::string& name) const;
    bool rename(const std::string& old_name, const std::string& new_name);
    void tree() const;
    void stat(const std::string& name) const;

    bool exit_fs();

private:
    // 递归打印目录树
    void print_tree(uint32_t dir_block, const std::string& prefix, bool is_last) const;

    // 路径解析：返回目标目录块号
    uint32_t resolve_path(const std::string& path) const;

    DiskIO       disk_;
    FatTable     fat_;
    Bitmap       bitmap_;
    DirectoryMgr dir_mgr_;
    FileOperator file_op_;

    uint32_t                   cur_block_;   // 当前工作目录块号
    std::vector<std::string>   path_stack_;  // 当前路径各段
};
