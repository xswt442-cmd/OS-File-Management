#pragma once
#include "utils.h"
#include "fat.h"
#include "bitmap.h"
#include "directory.h"
#include <cstdint>
#include <vector>

class FileOperator {
public:
    FileOperator(uint8_t* disk, FatTable& fat, Bitmap& bitmap, DirectoryMgr& dir_mgr);

    int  create(uint32_t dir_block, const std::string& name);
    int  open(uint32_t dir_block, const std::string& name);
    bool close(int fd);
    int  read(int fd, void* buf, uint32_t size);
    int  write(int fd, const void* data, uint32_t size);
    bool remove(uint32_t dir_block, const std::string& name);
    bool seek(int fd, uint32_t pos);

    const OpenFile* get_file(int fd) const;
    std::string  cat(uint32_t dir_block, const std::string& name) const;

private:
    uint8_t* disk_;
    FatTable& fat_;
    Bitmap& bitmap_;
    DirectoryMgr& dir_mgr_;

    std::vector<OpenFile> sys_files_;   // 系统打开文件表
    std::vector<UserFd>   user_fds_;    // 进程 fd 表
    int next_fd_ = 0;
};
