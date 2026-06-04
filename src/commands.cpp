#include "command.h"
#include "ui.h"
#include <iostream>
#include <sstream>

// 命令集
Command::Command(
    std::string name, 
    std::string usage, 
    std::string desc)
    : name_(std::move(name)), usage_(std::move(usage)), desc_(std::move(desc)) {}

CmdFormat::CmdFormat() : Command("format", "", "格式化虚拟磁盘，清除所有数据") {}

bool CmdFormat::execute(FileSystem& fs, const std::vector<std::string>&) {
    std::cout << "  确定格式化磁盘? 将清空所有数据 (y/n): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm == "y" || confirm == "Y") {
        fs.format();
        ui::print_ok("磁盘已格式化");
    }
    return true;
}

CmdMkdir::CmdMkdir() : Command("mkdir", "<name>", "创建子目录") {}

bool CmdMkdir::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: mkdir <目录名>"); return false; }
    if (fs.mkdir(args[1]))
        ui::print_ok("目录 '" + args[1] + "' 已创建");
    else
        ui::print_error("创建失败");
    return true;
}

CmdRmdir::CmdRmdir() : Command("rmdir", "<name>", "删除空目录") {}

bool CmdRmdir::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: rmdir <目录名>"); return false; }
    if (fs.rmdir(args[1]))
        ui::print_ok("目录 '" + args[1] + "' 已删除");
    else
        ui::print_error("删除失败");
    return true;
}

CmdLs::CmdLs() : Command("ls", "[-l]", "列出当前目录内容") {}

bool CmdLs::execute(FileSystem& fs, const std::vector<std::string>& args) {
    bool long_fmt = (args.size() >= 2 && args[1] == "-l");
    auto entries = fs.ls();
    if (entries.empty()) {
        std::cout << "  (空)\n";
    } else {
        for (auto& e : entries) {
            bool is_dir = (e.attr == ATTR_DIR);
            if (is_dir) {
                std::cout << "  \033[1;34m" << e.name << "/\033[0m";
                if (long_fmt)
                    std::cout << "  <DIR>     " << format_time(e.modify_time);
            } else {
                std::cout << "  " << e.name;
                if (long_fmt)
                    std::cout << "  " << format_size(e.size) << "     " << format_time(e.modify_time);
            }
            std::cout << "\n";
        }
    }
    return true;
}

CmdCd::CmdCd() : Command("cd", "<path>", "切换工作目录") {}

bool CmdCd::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "  " << fs.pwd() << "\n";
    } else {
        fs.cd(args[1]);
    }
    return true;
}

CmdCreate::CmdCreate() : Command("create", "<name>", "创建空文件") {}

bool CmdCreate::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: create <文件名>"); return false; }
    int r = fs.create(args[1]);
    if (r == 0)
        ui::print_ok("文件 '" + args[1] + "' 已创建");
    else
        ui::print_error("创建失败（可能已存在或磁盘满）");
    return true;
}

CmdOpen::CmdOpen() : Command("open", "<name>", "打开文件，返回 fd") {}

bool CmdOpen::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: open <文件名>"); return false; }
    int fd = fs.open(args[1]);
    if (fd >= 0)
        std::cout << "  fd = " << fd << "\n";
    else
        ui::print_error("打开失败");
    return true;
}

CmdClose::CmdClose() : Command("close", "<fd>", "关闭文件") {}

bool CmdClose::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: close <fd>"); return false; }
    int fd = std::stoi(args[1]);
    if (fs.close(fd))
        ui::print_ok("fd " + std::to_string(fd) + " 已关闭");
    else
        ui::print_error("关闭失败");
    return true;
}

CmdRead::CmdRead() : Command("read", "<fd> <size>", "从文件读取指定字节数") {}

bool CmdRead::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 3) { ui::print_error("用法: read <fd> <字节数>"); return false; }
    int fd = std::stoi(args[1]);
    int size = std::stoi(args[2]);
    std::vector<char> buf(size + 1);
    int n = fs.read(fd, buf.data(), static_cast<uint32_t>(size));
    if (n >= 0) {
        buf[n] = '\0';
        std::cout << "  读取 " << n << " 字节: \"" << buf.data() << "\"\n";
    } else {
        ui::print_error("读取失败");
    }
    return true;
}

CmdWrite::CmdWrite() : Command("write", "<fd> <text>", "向文件写入文本") {}

bool CmdWrite::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 3) { ui::print_error("用法: write <fd> <文本>"); return false; }
    int fd = std::stoi(args[1]);
    std::string text;
    for (size_t i = 2; i < args.size(); i++) {
        if (i > 2) text += " ";
        text += args[i];
    }
    int n = fs.write(fd, text.c_str(), static_cast<uint32_t>(text.size()));
    if (n >= 0)
        ui::print_ok("写入 " + std::to_string(n) + " 字节");
    else
        ui::print_error("写入失败");
    return true;
}

CmdDelete::CmdDelete() : Command("delete", "<name>", "删除文件") {}

bool CmdDelete::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: delete <文件名>"); return false; }
    if (fs.remove(args[1]))
        ui::print_ok("文件 '" + args[1] + "' 已删除");
    else
        ui::print_error("删除失败");
    return true;
}

CmdCat::CmdCat() : Command("cat", "<name>", "打印文件内容") {}

bool CmdCat::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: cat <文件名>"); return false; }
    fs.cat(args[1]);
    return true;
}

CmdDf::CmdDf() : Command("df", "", "查看磁盘空间（总量/已用/剩余/使用率）") {}

bool CmdDf::execute(FileSystem& fs, const std::vector<std::string>&) {
    fs.df();
    return true;
}

CmdDu::CmdDu() : Command("du", "<name>", "查看文件实际占用的块数和字节数") {}

bool CmdDu::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: du <文件名>"); return false; }
    fs.du(args[1]);
    return true;
}

CmdRename::CmdRename() : Command("rename", "<old> <new>", "重命名文件或目录") {}

bool CmdRename::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 3) { ui::print_error("用法: rename <旧名> <新名>"); return false; }
    if (fs.rename(args[1], args[2]))
        ui::print_ok("已重命名");
    else
        ui::print_error("重命名失败");
    return true;
}

CmdStat::CmdStat() : Command("stat", "<name>", "显示文件/目录详细信息") {}

bool CmdStat::execute(FileSystem& fs, const std::vector<std::string>& args) {
    if (args.size() < 2) { ui::print_error("用法: stat <名称>"); return false; }
    fs.stat(args[1]);
    return true;
}

CmdTree::CmdTree() : Command("tree", "", "以树形结构展示整个目录") {}

bool CmdTree::execute(FileSystem& fs, const std::vector<std::string>&) {
    fs.tree();
    return true;
}

CmdHelp::CmdHelp() : Command("help", "", "显示帮助信息") {}

bool CmdHelp::execute(FileSystem&, const std::vector<std::string>&) {
    // help 的实现放在 Shell::show_help 中，这里只是占位
    // 实际由 Shell::dispatch 拦截调用 show_help
    return true;
}

CmdExit::CmdExit() : Command("exit", "", "退出并持久化保存") {}

bool CmdExit::execute(FileSystem& fs, const std::vector<std::string>&) {
    fs.exit_fs();
    std::cout << "  再见!\n";
    return true;
}
