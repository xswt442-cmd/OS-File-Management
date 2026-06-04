#include "super_block.h"
#include <cstring>

void sb_read(const uint8_t* disk, SuperBlock& sb) {
    std::memcpy(&sb, disk + SUPER_BLOCK * BLOCK_SIZE, sizeof(SuperBlock));
}

void sb_write(uint8_t* disk, const SuperBlock& sb) {
    std::memcpy(disk + SUPER_BLOCK * BLOCK_SIZE, &sb, sizeof(SuperBlock));
}
