#include "fs_core.h"
#include "super_block.h"
#include "ui.h"
#include <cstring>
#include <iostream>
#include <filesystem>

FileSystem::FileSystem()
    : fat_(disk_.data())
    , bitmap_(disk_.data())
    , dir_mgr_(disk_.data(), fat_, bitmap_)
    , file_op_(disk_.data(), fat_, bitmap_, dir_mgr_)
    , cur_block_(DATA_START)
{
}

// 启动文件系统：加载持久化镜像 or 创建新磁盘
bool FileSystem::start() {
    const std::string img_path = "data/fs.fsimg";
    namespace fs = std::filesystem;
    fs::create_directories("data");

    if (fs::exists(img_path)) {
        if (disk_.load(img_path)) {
            // 从持久化镜像恢复
            SuperBlock sb;
            sb_read(disk_.data(), sb);
            cur_block_ = sb.root_block;
            path_stack_.clear();
            path_stack_.push_back("/");
            return true;
        }
    }
    // 新建虚拟磁盘
    disk_.create();
    cur_block_ = DATA_START;
    path_stack_.clear();
    path_stack_.push_back("/");
    disk_.save(img_path);
    return true;
}

void FileSystem::format() {
    disk_.create();  // 无条件创建全新磁盘
    cur_block_ = DATA_START;
    path_stack_.clear();
    path_stack_.push_back("/");
    disk_.save("data/fs.fsimg");
}

// 退出文件系统：更新超级块并保存镜像
bool FileSystem::exit_fs() {
    // 更新超级块
    SuperBlock sb;
    sb_read(disk_.data(), sb);
    sb.free_blocks = bitmap_.free_count();
    sb_write(disk_.data(), sb);
    if (disk_.save("data/fs.fsimg")) {
        ui::print_ok("虚拟磁盘已保存到 data/fs.fsimg");
        return true;
    }
    ui::print_error("保存失败");
    return false;
}

// 路径解析：支持绝对路径 (/a/b) 和相对路径 (a/b 或 ..)
uint32_t FileSystem::resolve_path(const std::string& path) const {
    if (path.empty()) return cur_block_;
    std::string p = path;
    // 统一分隔符
    for (auto& c : p) if (c == '\\') c = '/';

    uint32_t start_block;
    size_t pos = 0;

    if (p[0] == '/') {
        SuperBlock sb;
        sb_read(disk_.data(), sb);
        start_block = sb.root_block;
        pos = 1;
    } else {
        start_block = cur_block_;
    }

    uint32_t cur = start_block;
    while (pos < p.length()) {
        size_t next_slash = p.find('/', pos);
        std::string seg = (next_slash == std::string::npos)
                          ? p.substr(pos)
                          : p.substr(pos, next_slash - pos);
        pos = (next_slash == std::string::npos) ? p.length() : next_slash + 1;

        if (seg.empty() || seg == ".") continue;
        if (seg == "..") {
            cur = dir_mgr_.parent_block(cur);
            continue;
        }
        const DirEntry* e = dir_mgr_.find(cur, seg);
        if (!e || e->attr != ATTR_DIR) return static_cast<uint32_t>(-1);
        cur = e->first_block;
    }
    return cur;
}

// 获取当前路径字符串
std::string FileSystem::pwd() const {
    if (path_stack_.empty()) return "/";
    if (path_stack_.size() == 1 && path_stack_[0] == "/") return "/";
    std::string result;
    // path_stack_[0] is always "/"
    for (size_t i = 1; i < path_stack_.size(); i++) {
        result += "/" + path_stack_[i];
    }
    return result;
}

// 目录操作s

// mkdir: 创建新目录
bool FileSystem::mkdir(const std::string& name) {
    if (name.empty() || name.find('/') != std::string::npos) {
        ui::print_error("目录名不能为空或含 '/'");
        return false;
    }
    if (dir_mgr_.find(cur_block_, name)) {
        ui::print_error("'" + name + "' 已存在");
        return false;
    }
    int free_blk = bitmap_.find_free();
    if (free_blk < 0) {
        ui::print_error("磁盘已满");
        return false;
    }
    uint32_t block = static_cast<uint32_t>(free_blk);
    bitmap_.mark_used(block);
    fat_.set(block, FAT_EOF);
    dir_mgr_.init_dir(block, cur_block_);

    DirEntry entry;
    std::memset(&entry, 0, sizeof(entry));
    std::strncpy(entry.name, name.c_str(), MAX_FILENAME - 1);
    entry.first_block = block;
    entry.attr        = ATTR_DIR;
    entry.create_time = current_timestamp();
    entry.modify_time = entry.create_time;
    dir_mgr_.add_entry(cur_block_, entry);
    return true;
}

// rmdir: 删除目录（必须为空）
bool FileSystem::rmdir(const std::string& name) {
    if (name == "." || name == "..") {
        ui::print_error("不能删除 '.' 或 '..'");
        return false;
    }
    const DirEntry* e = dir_mgr_.find(cur_block_, name);
    if (!e || e->attr != ATTR_DIR) {
        ui::print_error("'" + name + "' 不是有效目录");
        return false;
    }
    if (!dir_mgr_.is_empty(e->first_block)) {
        ui::print_error("目录 '" + name + "' 非空");
        return false;
    }
    // 回收目录块
    fat_.free_chain(e->first_block);
    bitmap_.mark_free(e->first_block);
    dir_mgr_.remove_entry(cur_block_, name);
    return true;
}

// cd: 切换目录
bool FileSystem::cd(const std::string& path) {
    uint32_t target = resolve_path(path);
    if (target == static_cast<uint32_t>(-1)) {
        ui::print_error("路径 '" + path + "' 不存在");
        return false;
    }
    cur_block_ = target;
    // 更新路径栈
    if (path[0] == '/') {
        path_stack_.clear();
        path_stack_.push_back("/");
        auto parts = split_path(path);
        for (auto& part : parts) {
            if (part == "..") {
                if (path_stack_.size() > 1) path_stack_.pop_back();
            } else {
                path_stack_.push_back(part);
            }
        }
    } else {
        auto parts = split_path(path);
        for (auto& part : parts) {
            if (part == "..") {
                if (path_stack_.size() > 1) path_stack_.pop_back();
            } else {
                path_stack_.push_back(part);
            }
        }
    }
    return true;
}

// ls: 列出当前目录内容
std::vector<DirEntry> FileSystem::ls() const {
    return dir_mgr_.list(cur_block_);
}

// 文件操作s

// create: 创建新文件
int FileSystem::create(const std::string& name) {
    return file_op_.create(cur_block_, name);
}

// open: 打开文件，返回文件描述符
int FileSystem::open(const std::string& name) {
    return file_op_.open(cur_block_, name);
}

// close: 关闭文件描述符
bool FileSystem::close(int fd) {
    return file_op_.close(fd);
}

// read: 从文件描述符读取数据
int FileSystem::read(int fd, void* buf, uint32_t size) {
    return file_op_.read(fd, buf, size);
}

// write: 向文件描述符写入数据
int FileSystem::write(int fd, const void* data, uint32_t size) {
    return file_op_.write(fd, data, size);
}

// remove: 删除文件
bool FileSystem::remove(const std::string& name) {
    return file_op_.remove(cur_block_, name);
}

// 扩展命令

// df: 显示磁盘使用情况
void FileSystem::df() const {
    SuperBlock sb;
    sb_read(disk_.data(), sb);
    uint32_t total = sb.total_blocks * sb.block_size;
    uint32_t used_blocks = bitmap_.used_count();
    uint32_t free_blocks = bitmap_.free_count();
    uint32_t used  = used_blocks * sb.block_size;
    uint32_t free  = free_blocks * sb.block_size;
    int pct = total > 0 ? static_cast<int>(used * 100 / total) : 0;

    std::cout << "\n";
    std::cout << "  磁盘大小: " << format_size(total) << "\n";
    std::cout << "  已用空间: " << format_size(used) << "  (" << used_blocks << " 块)\n";
    std::cout << "  剩余空间: " << format_size(free) << "  (" << free_blocks << " 块)\n";
    std::cout << "  使用率:   ";
    // 简易进度条
    int bar_len = 30;
    int filled  = pct * bar_len / 100;
    std::cout << "[";
    for (int i = 0; i < bar_len; i++)
        std::cout << (i < filled ? "#" : "-");
    std::cout << "] " << pct << "%\n\n";
}

// du: 显示文件大小和占用情况
void FileSystem::du(const std::string& name) const {
    const DirEntry* e = dir_mgr_.find(cur_block_, name);
    if (!e) {
        ui::print_error("'" + name + "' 不存在");
        return;
    }
    uint32_t chain_sz = fat_.chain_size(e->first_block);
    uint32_t actual = chain_sz * BLOCK_SIZE;
    uint32_t logical = e->size;
    std::cout << "\n";
    std::cout << "  文件: " << e->name << "\n";
    std::cout << "  逻辑大小:  " << format_size(logical) << "\n";
    std::cout << "  实际占用:  " << format_size(actual) << "  (" << chain_sz << " 块 × "
              << BLOCK_SIZE << "B)\n";
    if (actual > logical)
        std::cout << "  内部碎片:  " << format_size(actual - logical) << "\n";
    std::cout << "\n";
}

// cat: 显示文件内容
void FileSystem::cat(const std::string& name) const {
    std::string content = file_op_.cat(cur_block_, name);
    if (content.empty()) {
        const DirEntry* e = dir_mgr_.find(cur_block_, name);
        if (!e)
            ui::print_error("'" + name + "' 不存在");
        // 文件内容为空是正常的
    }
    std::cout << content;
    if (!content.empty() && content.back() != '\n')
        std::cout << "\n";
}

// rename: 重命名文件或目录
bool FileSystem::rename(const std::string& old_name, const std::string& new_name) {
    if (new_name.find('/') != std::string::npos) {
        ui::print_error("新名称不能含 '/'");
        return false;
    }
    if (dir_mgr_.find(cur_block_, new_name)) {
        ui::print_error("'" + new_name + "' 已存在");
        return false;
    }
    return dir_mgr_.rename_entry(cur_block_, old_name, new_name);
}

// tree: 以树状结构显示当前目录及子目录内容
void FileSystem::tree() const {
    SuperBlock sb;
    sb_read(disk_.data(), sb);
    std::cout << "\n  /\n";
    print_tree(sb.root_block, "", true);
    std::cout << "\n";
}

// 打印目录树
void FileSystem::print_tree(uint32_t dir_block, const std::string& prefix, bool) const {
    auto entries = dir_mgr_.list(dir_block);
    for (size_t i = 0; i < entries.size(); i++) {
        bool last = (i == entries.size() - 1);
        std::cout << "  " << prefix << (last ? "  └── " : "  ├── ");
        if (entries[i].attr == ATTR_DIR) {
            std::cout << "\033[1;34m" << entries[i].name << "/\033[0m\n";
            std::string new_prefix = prefix + (last ? "      " : "  │   ");
            print_tree(entries[i].first_block, new_prefix, last);
        } else {
            std::cout << entries[i].name << "  (" << format_size(entries[i].size) << ")\n";
        }
    }
}

// stat: 显示文件或目录的详细信息
void FileSystem::stat(const std::string& name) const {
    const DirEntry* e = dir_mgr_.find(cur_block_, name);
    if (!e) {
        ui::print_error("'" + name + "' 不存在");
        return;
    }
    std::cout << "\n";
    std::cout << "  名称:     " << e->name << "\n";
    std::cout << "  类型:     " << (e->attr == ATTR_DIR ? "目录" : "文件") << "\n";
    std::cout << "  首块号:   " << e->first_block << "\n";
    std::cout << "  大小:     " << format_size(e->size) << "  (" << e->size << " B)\n";
    std::cout << "  占用块数: " << fat_.chain_size(e->first_block) << "\n";
    std::cout << "  创建时间: " << format_time(e->create_time) << "\n";
    std::cout << "  修改时间: " << format_time(e->modify_time) << "\n";
    std::cout << "\n";
}
