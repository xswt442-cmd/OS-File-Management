#include "directory.h"
#include <cstring>

DirectoryMgr::DirectoryMgr(uint8_t* disk, FatTable& fat, Bitmap& bitmap)
    : disk_(disk), fat_(fat), bitmap_(bitmap) {}

void DirectoryMgr::foreach_entry(uint32_t dir_block,
                                  bool (*callback)(const DirEntry&, uint32_t, uint32_t, void*),
                                  void* ctx) const {
    uint32_t block = dir_block;
    while (block < BLOCK_COUNT) {
        uint8_t buf[BLOCK_SIZE];
        std::memcpy(buf, disk_ + block * BLOCK_SIZE, BLOCK_SIZE);
        DirEntry* entries = reinterpret_cast<DirEntry*>(buf);
        for (uint32_t i = 0; i < ENTRIES_PER_BLOCK; i++) {
            if (entries[i].name[0] == '\0') continue;
            uint32_t offset = block * BLOCK_SIZE + i * sizeof(DirEntry);
            if (callback(entries[i], block, offset, ctx))
                return;
        }
        int32_t next = fat_.get(block);
        if (next == FAT_EOF || next == FAT_FREE) break;
        block = static_cast<uint32_t>(next);
    }
}

// 回调辅助结构
struct FindCtx {
    const char* name;
    DirEntry* result;
};

static bool find_callback(const DirEntry& e, uint32_t, uint32_t, void* ctx) {
    FindCtx* fc = static_cast<FindCtx*>(ctx);
    if (std::strncmp(e.name, fc->name, MAX_FILENAME) == 0) {
        if (fc->result) std::memcpy(fc->result, &e, sizeof(DirEntry));
        return true;
    }
    return false;
}

const DirEntry* DirectoryMgr::find(uint32_t dir_block, const std::string& name) const {
    static DirEntry found;
    std::memset(&found, 0, sizeof(found));  // 每次查找前清空，避免残留上次结果
    FindCtx ctx;
    ctx.name = name.c_str();
    ctx.result = &found;
    foreach_entry(dir_block, find_callback, &ctx);
    if (ctx.result == &found && found.name[0] != '\0')
        return &found;
    return nullptr;
}

struct ListCtx {
    std::vector<DirEntry>* entries;
};

static bool list_callback(const DirEntry& e, uint32_t, uint32_t, void* ctx) {
    ListCtx* lc = static_cast<ListCtx*>(ctx);
    // 跳过 . 和 ..
    if (std::strncmp(e.name, ".", MAX_FILENAME) == 0 ||
        std::strncmp(e.name, "..", MAX_FILENAME) == 0)
        return false;
    lc->entries->push_back(e);
    return false;
}

std::vector<DirEntry> DirectoryMgr::list(uint32_t dir_block) const {
    std::vector<DirEntry> result;
    ListCtx ctx = { &result };
    foreach_entry(dir_block, list_callback, &ctx);
    return result;
}

bool DirectoryMgr::add_entry(uint32_t dir_block, const DirEntry& entry) {
    return write_entry_to_dir(dir_block, entry);
}

struct RemoveCtx {
    const char* name;
    uint32_t found_block;
    uint32_t found_offset;
    bool removed;
};

static bool remove_callback(const DirEntry& e, uint32_t block, uint32_t offset, void* ctx) {
    RemoveCtx* rc = static_cast<RemoveCtx*>(ctx);
    if (std::strncmp(e.name, rc->name, MAX_FILENAME) == 0) {
        rc->found_block = block;
        rc->found_offset = offset;
        return true;
    }
    return false;
}

bool DirectoryMgr::remove_entry(uint32_t dir_block, const std::string& name) {
    RemoveCtx ctx;
    ctx.name = name.c_str();
    ctx.found_block = 0;
    ctx.found_offset = 0;
    ctx.removed = false;
    foreach_entry(dir_block, remove_callback, &ctx);
    if (ctx.found_block == 0 && ctx.found_offset == 0)
        return false;
    char zero = 0;
    std::memcpy(disk_ + ctx.found_offset, &zero, 1);
    return true;
}

struct RenameCtx {
    const char* old_name;
    const char* new_name;
    uint8_t* disk;
    bool done;
};

static bool rename_callback(const DirEntry& e, uint32_t, uint32_t offset, void* ctx) {
    RenameCtx* rc = static_cast<RenameCtx*>(ctx);
    if (std::strncmp(e.name, rc->old_name, MAX_FILENAME) == 0) {
        char new_name_buf[MAX_FILENAME];
        std::memset(new_name_buf, 0, MAX_FILENAME);
        std::strncpy(new_name_buf, rc->new_name, MAX_FILENAME - 1);
        std::memcpy(rc->disk + offset, new_name_buf, MAX_FILENAME);
        rc->done = true;
        return true;
    }
    return false;
}

bool DirectoryMgr::rename_entry(uint32_t dir_block, const std::string& old_name, const std::string& new_name) {
    RenameCtx ctx;
    ctx.old_name = old_name.c_str();
    ctx.new_name = new_name.c_str();
    ctx.disk = disk_;
    ctx.done = false;
    foreach_entry(dir_block, rename_callback, &ctx);
    return ctx.done;
}

void DirectoryMgr::init_dir(uint32_t dir_block, uint32_t parent_block) {
    uint8_t buf[BLOCK_SIZE];
    std::memset(buf, 0, BLOCK_SIZE);
    DirEntry dot, dotdot;
    std::memset(&dot, 0, sizeof(dot));
    std::memset(&dotdot, 0, sizeof(dotdot));
    std::strncpy(dot.name, ".", MAX_FILENAME - 1);
    dot.first_block = dir_block;
    dot.attr = ATTR_DIR;
    dot.create_time = current_timestamp();
    dot.modify_time = dot.create_time;
    std::strncpy(dotdot.name, "..", MAX_FILENAME - 1);
    dotdot.first_block = parent_block;
    dotdot.attr = ATTR_DIR;
    dotdot.create_time = dot.create_time;
    dotdot.modify_time = dot.create_time;
    std::memcpy(buf, &dot, sizeof(DirEntry));
    std::memcpy(buf + sizeof(DirEntry), &dotdot, sizeof(DirEntry));
    std::memcpy(disk_ + dir_block * BLOCK_SIZE, buf, BLOCK_SIZE);
}

struct EmptyCtx {
    int count;
};

static bool empty_callback(const DirEntry& e, uint32_t, uint32_t, void* ctx) {
    EmptyCtx* ec = static_cast<EmptyCtx*>(ctx);
    if (std::strncmp(e.name, ".", MAX_FILENAME) == 0 ||
        std::strncmp(e.name, "..", MAX_FILENAME) == 0)
        return false;
    ec->count++;
    return true;  // 只要发现一个就停
}

bool DirectoryMgr::is_empty(uint32_t dir_block) const {
    EmptyCtx ctx = { 0 };
    foreach_entry(dir_block, empty_callback, &ctx);
    return ctx.count == 0;
}

uint32_t DirectoryMgr::self_block(uint32_t dir_block) const {
    const DirEntry* e = find(dir_block, ".");
    return e ? e->first_block : dir_block;
}

uint32_t DirectoryMgr::parent_block(uint32_t dir_block) const {
    const DirEntry* e = find(dir_block, "..");
    return e ? e->first_block : dir_block;
}

bool DirectoryMgr::write_entry_to_dir(uint32_t dir_block, const DirEntry& entry) {
    uint32_t block = dir_block;
    while (true) {
        uint8_t buf[BLOCK_SIZE];
        std::memcpy(buf, disk_ + block * BLOCK_SIZE, BLOCK_SIZE);
        DirEntry* entries = reinterpret_cast<DirEntry*>(buf);
        for (uint32_t i = 0; i < ENTRIES_PER_BLOCK; i++) {
            if (entries[i].name[0] == '\0') {
                std::memcpy(&entries[i], &entry, sizeof(DirEntry));
                std::memcpy(disk_ + block * BLOCK_SIZE, buf, BLOCK_SIZE);
                return true;
            }
        }
        int32_t next = fat_.get(block);
        if (next == FAT_EOF) {
            // 目录已满，分配新块
            uint32_t new_block = fat_.alloc_block(block);
            if (new_block == static_cast<uint32_t>(-1)) return false;
            std::memset(buf, 0, BLOCK_SIZE);
            std::memcpy(buf, &entry, sizeof(DirEntry));
            std::memcpy(disk_ + new_block * BLOCK_SIZE, buf, BLOCK_SIZE);
            bitmap_.mark_used(new_block);
            return true;
        }
        if (next == FAT_FREE) break;
        block = static_cast<uint32_t>(next);
    }
    return false;
}
