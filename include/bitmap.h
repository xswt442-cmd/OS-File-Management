#pragma once
#include <cstdint>

class Bitmap {
public:
    explicit Bitmap(uint8_t* disk);

    void   init();                         // 全部标记为空闲
    bool   is_free(uint32_t block) const;
    void   mark_used(uint32_t block);
    void   mark_free(uint32_t block);
    int    find_free();                    // 返回第一个空闲块号，无则返回 -1
    uint32_t used_count() const;           // 已用块数
    uint32_t free_count() const;           // 空闲块数

private:
    uint8_t* disk_;
};
