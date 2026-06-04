#pragma once
#include "utils.h"

struct SuperBlock {
    uint32_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t fat_blocks;
    uint32_t bitmap_block;
    uint32_t root_block;
    uint8_t  reserved[484];
};

// 从磁盘缓冲区读写超级块
void sb_read(const uint8_t* disk, SuperBlock& sb);
void sb_write(uint8_t* disk, const SuperBlock& sb);
