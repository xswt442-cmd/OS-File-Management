#pragma once
#include <cstdint>
#include "utils.h"
#include <string>

class DiskIO {
public:
    DiskIO();

    bool create();                          // 创建空白磁盘并格式化
    bool load(const std::string& path);     // 从文件加载
    bool save(const std::string& path);     // 保存到文件
    void format();                          // 格式化磁盘

    uint8_t* data();                        // 获取磁盘缓冲区指针
    const uint8_t* data() const;

    bool read_block(uint32_t block, void* buf);
    bool write_block(uint32_t block, const void* buf);

    bool read_bytes(uint32_t offset, void* buf, uint32_t size);
    bool write_bytes(uint32_t offset, const void* buf, uint32_t size);

private:
    uint8_t disk_[DISK_SIZE];
};
