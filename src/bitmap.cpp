#include "bitmap.h"
#include "utils.h"
#include <cstring>

Bitmap::Bitmap(uint8_t* disk) : disk_(disk) {}

// 初始化位图
void Bitmap::init() {
    uint8_t* bmp = disk_ + BITMAP_BLOCK * BLOCK_SIZE;
    std::memset(bmp, 0, BLOCK_COUNT / 8);
}

// 检查块是否空闲
bool Bitmap::is_free(uint32_t block) const {
    if (block >= BLOCK_COUNT) return false;
    uint8_t* bmp = disk_ + BITMAP_BLOCK * BLOCK_SIZE;
    uint32_t byte_idx = block / 8;
    uint32_t bit_idx  = block % 8;
    return !(bmp[byte_idx] & (1 << bit_idx));
}

// 标记块为已用
void Bitmap::mark_used(uint32_t block) {
    if (block >= BLOCK_COUNT) return;
    uint8_t* bmp = disk_ + BITMAP_BLOCK * BLOCK_SIZE;
    bmp[block / 8] |= (1 << (block % 8));
}

// 标记块为自由
void Bitmap::mark_free(uint32_t block) {
    if (block >= BLOCK_COUNT) return;
    uint8_t* bmp = disk_ + BITMAP_BLOCK * BLOCK_SIZE;
    bmp[block / 8] &= ~(1 << (block % 8));
}

// 查找第一个空闲块
int Bitmap::find_free() {
    for (uint32_t i = DATA_START; i < BLOCK_COUNT; i++) {
        if (is_free(i)) return static_cast<int>(i);
    }
    return -1;
}

// 统计已用块数
uint32_t Bitmap::used_count() const {
    uint32_t count = 0;
    for (uint32_t i = 0; i < BLOCK_COUNT; i++)
        if (!is_free(i)) count++;
    return count;
}

// 统计空闲块数
uint32_t Bitmap::free_count() const {
    return BLOCK_COUNT - used_count();
}
