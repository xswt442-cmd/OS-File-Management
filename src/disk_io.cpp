#include "disk_io.h"
#include "super_block.h"
#include "fat.h"
#include "bitmap.h"
#include <cstring>
#include <fstream>
#include <iostream>

DiskIO::DiskIO() {
    std::memset(disk_, 0, DISK_SIZE);
}

// 创建新磁盘
bool DiskIO::create() {
    format();
    return true;
}

// 初始化
void DiskIO::format() {
    std::memset(disk_, 0, DISK_SIZE);

    // 写入超级块
    SuperBlock sb;
    std::memset(&sb, 0, sizeof(sb));
    sb.magic        = OSFS_MAGIC;
    sb.block_size   = BLOCK_SIZE;
    sb.total_blocks = BLOCK_COUNT;
    sb.free_blocks  = DATA_BLOCKS - 1;  // 预留给根目录
    sb.fat_blocks   = FAT_BLOCKS;
    sb.bitmap_block = BITMAP_BLOCK;
    sb.root_block   = DATA_START;
    sb_write(disk_, sb);

    // 初始化 FAT
    FatTable fat(disk_);
    fat.init();

    // 初始化位图
    Bitmap bmp(disk_);
    bmp.init();
    // 标记元数据块已占用
    bmp.mark_used(SUPER_BLOCK);
    for (uint32_t i = FAT_START; i < FAT_START + FAT_BLOCKS; i++)
        bmp.mark_used(i);
    bmp.mark_used(BITMAP_BLOCK);
    // 根目录占一块
    bmp.mark_used(DATA_START);
    fat.set(DATA_START, FAT_EOF);

    // 初始化根目录
    uint8_t dir_block_buf[BLOCK_SIZE];
    std::memset(dir_block_buf, 0, BLOCK_SIZE);
    DirEntry dot, dotdot;
    std::memset(&dot, 0, sizeof(dot));
    std::memset(&dotdot, 0, sizeof(dotdot));
    std::strncpy(dot.name, ".", MAX_FILENAME - 1);
    dot.first_block = DATA_START;
    dot.attr        = ATTR_DIR;
    dot.create_time = current_timestamp();
    dot.modify_time = dot.create_time;
    std::strncpy(dotdot.name, "..", MAX_FILENAME - 1);
    dotdot.first_block = DATA_START;
    dotdot.attr        = ATTR_DIR;
    dotdot.create_time = dot.create_time;
    dotdot.modify_time = dot.create_time;
    std::memcpy(dir_block_buf, &dot, sizeof(DirEntry));
    std::memcpy(dir_block_buf + sizeof(DirEntry), &dotdot, sizeof(DirEntry));
    write_block(DATA_START, dir_block_buf);
}

// 加载
bool DiskIO::load(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    in.read(reinterpret_cast<char*>(disk_), DISK_SIZE);
    if (in.gcount() != DISK_SIZE) {
        std::cerr << "警告: 镜像文件大小不匹配，已用空磁盘替代" << std::endl;
        return false;
    }
    // 校验魔数
    SuperBlock sb;
    sb_read(disk_, sb);
    if (sb.magic != OSFS_MAGIC) {
        std::cerr << "警告: 镜像文件损坏 (魔数不匹配)，已用空磁盘替代" << std::endl;
        return false;
    }
    return true;
}

// 保存
bool DiskIO::save(const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;
    out.write(reinterpret_cast<const char*>(disk_), DISK_SIZE);
    return out.good();
}

uint8_t* DiskIO::data()        { return disk_; }
const uint8_t* DiskIO::data() const { return disk_; }

// 块级读写
bool DiskIO::read_block(uint32_t block, void* buf) {
    if (block >= BLOCK_COUNT) return false;
    std::memcpy(buf, disk_ + block * BLOCK_SIZE, BLOCK_SIZE);
    return true;
}
bool DiskIO::write_block(uint32_t block, const void* buf) {
    if (block >= BLOCK_COUNT) return false;
    std::memcpy(disk_ + block * BLOCK_SIZE, buf, BLOCK_SIZE);
    return true;
}

// 字节级读写
bool DiskIO::read_bytes(uint32_t offset, void* buf, uint32_t size) {
    if (offset + size > DISK_SIZE) return false;
    std::memcpy(buf, disk_ + offset, size);
    return true;
}
bool DiskIO::write_bytes(uint32_t offset, const void* buf, uint32_t size) {
    if (offset + size > DISK_SIZE) return false;
    std::memcpy(disk_ + offset, buf, size);
    return true;
}
