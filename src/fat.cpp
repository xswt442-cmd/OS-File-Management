#include "fat.h"
#include "utils.h"
#include <cstring>

FatTable::FatTable(uint8_t* disk) : disk_(disk) {}

// 初始化 FAT 表
void FatTable::init() {
    int32_t free = FAT_FREE;
    for (uint32_t i = 0; i < BLOCK_COUNT; i++) {
        uint32_t offset = (FAT_START * BLOCK_SIZE) + i * sizeof(int32_t);
        std::memcpy(disk_ + offset, &free, sizeof(int32_t));
    }
}

// 获取 FAT 表项
int32_t FatTable::get(uint32_t block) const {
    if (block >= BLOCK_COUNT) return FAT_FREE;
    uint32_t offset = (FAT_START * BLOCK_SIZE) + block * sizeof(int32_t);
    int32_t val;
    std::memcpy(&val, disk_ + offset, sizeof(int32_t));
    return val;
}

// 设置 FAT 表项
void FatTable::set(uint32_t block, int32_t val) {
    if (block >= BLOCK_COUNT) return;
    uint32_t offset = (FAT_START * BLOCK_SIZE) + block * sizeof(int32_t);
    std::memcpy(disk_ + offset, &val, sizeof(int32_t));
}

uint32_t FatTable::alloc_block(uint32_t prev_block) {
    // 遍历 FAT 找空闲块
    for (uint32_t i = DATA_START; i < BLOCK_COUNT; i++) {
        if (get(i) == FAT_FREE) {
            set(i, FAT_EOF);
            if (prev_block != static_cast<uint32_t>(-1) && prev_block < BLOCK_COUNT) {
                set(prev_block, static_cast<int32_t>(i));
            }
            return i;
        }
    }
    return static_cast<uint32_t>(-1);  // 磁盘满
}

// 释放 FAT 链
void FatTable::free_chain(uint32_t first_block) {
    uint32_t block = first_block;
    while (block != static_cast<uint32_t>(FAT_EOF) && block < BLOCK_COUNT) {
        int32_t next = get(block);
        set(block, FAT_FREE);
        if (next == FAT_EOF) break;
        block = static_cast<uint32_t>(next);
    }
}

// 获取下一个块号
uint32_t FatTable::nth_block(uint32_t first_block, uint32_t n) const {
    uint32_t block = first_block;
    for (uint32_t i = 0; i < n; i++) {
        int32_t next = get(block);
        if (next <= 0) return static_cast<uint32_t>(-1);
        block = static_cast<uint32_t>(next);
    }
    return block;
}

// 获取 FAT 链长度（占用块数）
uint32_t FatTable::chain_size(uint32_t first_block) const {
    uint32_t count = 0;
    uint32_t block = first_block;
    while (block < BLOCK_COUNT) {
        count++;
        int32_t next = get(block);
        if (next == FAT_EOF) break;
        if (next == FAT_FREE) break;
        block = static_cast<uint32_t>(next);
    }
    return count;
}
