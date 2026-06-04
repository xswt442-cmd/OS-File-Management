#pragma once
#include "utils.h"
#include "fat.h"
#include "bitmap.h"
#include <string>
#include <vector>

class DirectoryMgr {
public:
    DirectoryMgr(uint8_t* disk, FatTable& fat, Bitmap& bitmap);

    // 在当前目录中查找目录项，返回指针（指向内部缓存），未找到返回 nullptr
    const DirEntry* find(uint32_t dir_block, const std::string& name) const;

    // 列出目录中所有有效条目
    std::vector<DirEntry> list(uint32_t dir_block) const;

    // 添加/删除目录项
    bool add_entry(uint32_t dir_block, const DirEntry& entry);
    bool remove_entry(uint32_t dir_block, const std::string& name);

    // 重命名
    bool rename_entry(uint32_t dir_block, const std::string& old_name, const std::string& new_name);

    // 初始化目录 (创建 . 和 ..)
    void init_dir(uint32_t dir_block, uint32_t parent_block);

    // 判断目录是否为空 (仅有 . 和 ..)
    bool is_empty(uint32_t dir_block) const;

    // 获取目录的首块号 (通过 "." 条目)
    uint32_t self_block(uint32_t dir_block) const;

    // 获取父目录的首块号 (通过 ".." 条目)
    uint32_t parent_block(uint32_t dir_block) const;

private:
    // 扫描目录所有块，对每一条有效条目执行回调；回调返回 true 时停止
    void foreach_entry(uint32_t dir_block,
                       bool (*callback)(const DirEntry& e, uint32_t block, uint32_t offset, void* ctx),
                       void* ctx) const;

    // 在目录中找一个空闲槽位写入；若无空闲块则分配新块
    bool write_entry_to_dir(uint32_t dir_block, const DirEntry& entry);

    uint8_t* disk_;
    FatTable& fat_;
    Bitmap& bitmap_;
};
