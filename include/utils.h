#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ctime>

constexpr uint32_t BLOCK_SIZE    = 1024;
constexpr uint32_t BLOCK_COUNT   = 1024;
constexpr uint32_t DISK_SIZE     = BLOCK_SIZE * BLOCK_COUNT;

constexpr uint32_t SUPER_BLOCK   = 0;
constexpr uint32_t FAT_START     = 1;
constexpr uint32_t FAT_BLOCKS    = 4;
constexpr uint32_t BITMAP_BLOCK  = 5;
constexpr uint32_t DATA_START    = 6;
constexpr uint32_t DATA_BLOCKS   = BLOCK_COUNT - DATA_START;

constexpr int32_t FAT_EOF  =  0;
constexpr int32_t FAT_FREE = -1;

constexpr uint8_t ATTR_FILE = 0x00;
constexpr uint8_t ATTR_DIR  = 0x01;

constexpr uint32_t OSFS_MAGIC = 0x4F534653;
constexpr uint32_t MAX_FILENAME = 56;
constexpr uint32_t ENTRIES_PER_BLOCK = BLOCK_SIZE / 76;  // sizeof(DirEntry)=76

struct DirEntry {
    char     name[56];
    uint32_t first_block;
    uint32_t size;
    uint32_t create_time;
    uint32_t modify_time;
    uint8_t  attr;
    uint8_t  _pad[3];
};

struct OpenFile {
    uint32_t first_block;
    uint32_t size;
    uint32_t cursor;
    uint32_t ref_count;
};

struct UserFd {
    int      fd;
    uint32_t file_id;
};

uint32_t current_timestamp();
std::string format_size(uint32_t bytes);
std::string format_time(uint32_t ts);
std::string trim(const std::string& s);
std::vector<std::string> split_path(const std::string& path);
