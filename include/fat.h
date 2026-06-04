#pragma once
#include <cstdint>

class FatTable {
public:
    explicit FatTable(uint8_t* disk);

    void   init();                         // 全部初始化为 FAT_FREE
    int32_t get(uint32_t block) const;     // 读取第 block 块的表项
    void   set(uint32_t block, int32_t val);

    // 分配一个新块，链接到 prev_block 后面；返回新块号
    uint32_t alloc_block(uint32_t prev_block);

    // 回收整条 FAT 链上的所有块
    void   free_chain(uint32_t first_block);

    // 获取链上第 n 块 (0-based)
    uint32_t nth_block(uint32_t first_block, uint32_t n) const;

    // 统计链上的块数
    uint32_t chain_size(uint32_t first_block) const;

private:
    uint8_t* disk_;
};
