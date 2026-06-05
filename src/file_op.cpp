#include "file_op.h"
#include <cstring>

FileOperator::FileOperator(uint8_t* disk, FatTable& fat, Bitmap& bitmap, DirectoryMgr& dir_mgr)
    : disk_(disk), fat_(fat), bitmap_(bitmap), dir_mgr_(dir_mgr) {}

// 在指定目录下创建新文件
int FileOperator::create(uint32_t dir_block, const std::string& name) {
    // 检查是否存在同名文件/目录
    if (dir_mgr_.find(dir_block, name) != nullptr) return -1;
    // 分配新块
    int free_blk = bitmap_.find_free();
    if (free_blk < 0) return -1;
    uint32_t block = static_cast<uint32_t>(free_blk);
    bitmap_.mark_used(block);
    fat_.set(block, FAT_EOF);
    // 创建目录项
    DirEntry entry;
    std::memset(&entry, 0, sizeof(entry));
    std::strncpy(entry.name, name.c_str(), MAX_FILENAME - 1);
    entry.first_block = block;
    entry.size        = 0;
    entry.attr        = ATTR_FILE;
    entry.create_time = current_timestamp();
    entry.modify_time = entry.create_time;
    if (!dir_mgr_.add_entry(dir_block, entry)) {
        bitmap_.mark_free(block);
        fat_.set(block, FAT_FREE);
        return -1;
    }
    return 0;  // 返回 0 表示成功（不自动打开）
}

// 打开
int FileOperator::open(uint32_t dir_block, const std::string& name) {
    const DirEntry* e = dir_mgr_.find(dir_block, name);
    if (!e || e->attr != ATTR_FILE) return -1;
    // 检查是否已打开（ref_count > 0 才复用）
    for (size_t i = 0; i < sys_files_.size(); i++) {
        if (sys_files_[i].first_block == e->first_block && sys_files_[i].ref_count > 0) {
            sys_files_[i].ref_count++;
            int fd = next_fd_++;
            user_fds_.push_back({fd, static_cast<uint32_t>(i)});
            return fd;
        }
    }
    // 新建打开文件表项
    OpenFile of;
    std::memset(&of, 0, sizeof(of));
    of.first_block = e->first_block;
    of.size        = e->size;
    of.cursor      = 0;
    of.ref_count   = 1;
    of.dir_block   = dir_block;
    std::strncpy(of.name, name.c_str(), MAX_FILENAME - 1);
    sys_files_.push_back(of);
    uint32_t fid = static_cast<uint32_t>(sys_files_.size() - 1);
    int fd = next_fd_++;
    user_fds_.push_back({fd, fid});
    return fd;
}

// 关闭
bool FileOperator::close(int fd) {
    for (size_t i = 0; i < user_fds_.size(); i++) {
        if (user_fds_[i].fd == fd) {
            uint32_t fid = user_fds_[i].file_id;
            if (fid < sys_files_.size()) {
                auto& of = sys_files_[fid];
                if (of.ref_count > 0)
                    of.ref_count--;
                // 把 size 写回磁盘目录项
                if (of.dir_block < BLOCK_COUNT) {
                    uint32_t blk = of.dir_block;
                    bool found = false;
                    while (!found && blk < BLOCK_COUNT) {
                        auto* entries = reinterpret_cast<DirEntry*>(disk_ + blk * BLOCK_SIZE);
                        for (uint32_t j = 0; j < ENTRIES_PER_BLOCK; j++) {
                            if (std::strncmp(entries[j].name, of.name, MAX_FILENAME) == 0) {
                                entries[j].size = of.size;
                                entries[j].modify_time = current_timestamp();
                                found = true;
                                break;
                            }
                        }
                        int32_t next = fat_.get(blk);
                        if (next == FAT_EOF || next == FAT_FREE) break;
                        blk = static_cast<uint32_t>(next);
                    }
                }
            }
            user_fds_.erase(user_fds_.begin() + i);
            return true;
        }
    }
    return false;
}

// 读取
int FileOperator::read(int fd, void* buf, uint32_t size) {
    // 查找 fd
    OpenFile* of = nullptr;
    for (auto& uf : user_fds_) {
        if (uf.fd == fd && uf.file_id < sys_files_.size()) {
            of = &sys_files_[uf.file_id];
            break;
        }
    }
    if (!of) return -1;

    uint32_t remaining = size;
    if (of->cursor + remaining > of->size)
        remaining = of->size - of->cursor;
    uint32_t total_read = remaining;

    uint8_t* out = static_cast<uint8_t*>(buf);
    uint32_t pos = of->cursor;

    while (remaining > 0) {
        uint32_t blk_idx = pos / BLOCK_SIZE;
        uint32_t blk_off = pos % BLOCK_SIZE;
        uint32_t to_read = (blk_off + remaining > BLOCK_SIZE) ? (BLOCK_SIZE - blk_off) : remaining;

        uint32_t block = fat_.nth_block(of->first_block, blk_idx);
        if (block == static_cast<uint32_t>(-1)) break;

        uint32_t disk_off = block * BLOCK_SIZE + blk_off;
        std::memcpy(out, disk_ + disk_off, to_read);

        out += to_read;
        pos += to_read;
        remaining -= to_read;
    }

    of->cursor += (total_read - remaining);
    return static_cast<int>(total_read - remaining);
}

// 写入
int FileOperator::write(int fd, const void* data, uint32_t size) {
    OpenFile* of = nullptr;
    for (auto& uf : user_fds_) {
        if (uf.fd == fd && uf.file_id < sys_files_.size()) {
            of = &sys_files_[uf.file_id];
            break;
        }
    }
    if (!of) return -1;

    const uint8_t* in = static_cast<const uint8_t*>(data);
    uint32_t remaining = size;
    uint32_t pos = of->cursor;

    while (remaining > 0) {
        uint32_t blk_idx = pos / BLOCK_SIZE;
        uint32_t blk_off = pos % BLOCK_SIZE;
        uint32_t to_write = (blk_off + remaining > BLOCK_SIZE) ? (BLOCK_SIZE - blk_off) : remaining;

        uint32_t block = fat_.nth_block(of->first_block, blk_idx);
        // 如需新块
        if (block == static_cast<uint32_t>(-1) || block >= BLOCK_COUNT) {
            // 找到链尾
            uint32_t prev = of->first_block;
            while (true) {
                int32_t nxt = fat_.get(prev);
                if (nxt == FAT_EOF || nxt == FAT_FREE) break;
                prev = static_cast<uint32_t>(nxt);
            }
            block = fat_.alloc_block(prev);
            if (block == static_cast<uint32_t>(-1)) break;
            bitmap_.mark_used(block);
        }

        uint32_t disk_off = block * BLOCK_SIZE + blk_off;
        std::memcpy(disk_ + disk_off, in, to_write);

        in += to_write;
        pos += to_write;
        remaining -= to_write;
    }

    uint32_t written = size - remaining;
    of->cursor += written;
    if (of->cursor > of->size) of->size = of->cursor;
    return static_cast<int>(written);
}

// 删除
bool FileOperator::remove(uint32_t dir_block, const std::string& name) {
    const DirEntry* e = dir_mgr_.find(dir_block, name);
    if (!e || e->attr != ATTR_FILE) return false;
    // 回收 FAT 链上的所有块
    fat_.free_chain(e->first_block);
    // 在位图中标记为空闲
    uint32_t block = e->first_block;
    while (block < BLOCK_COUNT) {
        bitmap_.mark_free(block);
        int32_t next = fat_.get(block);
        fat_.set(block, FAT_FREE);
        if (next == FAT_EOF) break;
        if (next == FAT_FREE) break;
        block = static_cast<uint32_t>(next);
    }
    // 从目录中移除条目
    return dir_mgr_.remove_entry(dir_block, name);
}

// 查找
bool FileOperator::seek(int fd, uint32_t pos) {
    for (auto& uf : user_fds_) {
        if (uf.fd == fd && uf.file_id < sys_files_.size()) {
            if (pos <= sys_files_[uf.file_id].size) {
                sys_files_[uf.file_id].cursor = pos;
                return true;
            }
        }
    }
    return false;
}

// 获取文件信息
const OpenFile* FileOperator::get_file(int fd) const {
    for (auto& uf : user_fds_) {
        if (uf.fd == fd && uf.file_id < sys_files_.size())
            return &sys_files_[uf.file_id];
    }
    return nullptr;
}

// 显示文件内容
std::string FileOperator::cat(uint32_t dir_block, const std::string& name) const {
    const DirEntry* e = dir_mgr_.find(dir_block, name);
    if (!e || e->attr != ATTR_FILE) return "";
    std::string result;
    result.resize(e->size);
    uint32_t remaining = e->size;
    uint32_t block = e->first_block;
    uint32_t offset = 0;
    while (remaining > 0 && block < BLOCK_COUNT) {
        uint32_t to_read = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        std::memcpy(&result[offset], disk_ + block * BLOCK_SIZE, to_read);
        offset += to_read;
        remaining -= to_read;
        int32_t next = fat_.get(block);
        if (next == FAT_EOF) break;
        block = static_cast<uint32_t>(next);
    }
    return result;
}
