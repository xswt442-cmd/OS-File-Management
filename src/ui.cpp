#include "ui.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace ui {

void console_init() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
}

// 初始 UI 界面
void splash() {
    std::cout << "\033[96m";
    std::cout << "  ╔══════════════════════════════════════════╗\n";
    std::cout << "  ║     OS-File-Management v1.0              ║\n";
    std::cout << "  ║     简易文件系统模拟 FAT + Bitmap        ║\n";
    std::cout << "  ║     Virtual Disk: 1MB | Blocks: 1024     ║\n";
    std::cout << "  ╚══════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";
}

// 显示命令提示符
void show_prompt(const std::string& cwd) {
    std::cout << CYAN << "osfs:" << cwd << "$ " << RESET;
}

// 显示帮助信息
void show_help() {
    std::cout << BOLD << "\n  可用命令:\n\n" << RESET;
    std::cout << "  " << GREEN << "format" << RESET << "                格式化虚拟磁盘\n";
    std::cout << "  " << GREEN << "mkdir <name>" << RESET << "           创建子目录\n";
    std::cout << "  " << GREEN << "rmdir <name>" << RESET << "           删除空目录\n";
    std::cout << "  " << GREEN << "ls [-l]" << RESET << "               列出当前目录内容\n";
    std::cout << "  " << GREEN << "cd <path>" << RESET << "             切换工作目录\n";
    std::cout << "  " << GREEN << "create <name>" << RESET << "         创建空文件\n";
    std::cout << "  " << GREEN << "open <name>" << RESET << "           打开文件，返回 fd\n";
    std::cout << "  " << GREEN << "close <fd>" << RESET << "            关闭文件\n";
    std::cout << "  " << GREEN << "read <fd> <size>" << RESET << "      从文件读取指定字节数\n";
    std::cout << "  " << GREEN << "write <fd> <text>" << RESET << "      向文件写入文本\n";
    std::cout << "  " << GREEN << "delete <name>" << RESET << "         删除文件\n";
    std::cout << "  " << GREEN << "cat <name>" << RESET << "            打印文件内容\n";
    std::cout << "  " << GREEN << "rename <old> <new>" << RESET << "    重命名文件/目录\n";
    std::cout << "  " << GREEN << "df" << RESET << "                    查看磁盘空间\n";
    std::cout << "  " << GREEN << "du <name>" << RESET << "            查看文件占用\n";
    std::cout << "  " << GREEN << "stat <name>" << RESET << "          显示文件详细信息\n";
    std::cout << "  " << GREEN << "tree" << RESET << "                  目录树形展示\n";
    std::cout << "  " << GREEN << "help" << RESET << "                  显示本帮助\n";
    std::cout << "  " << GREEN << "exit" << RESET << "                  退出并持久化\n";
    std::cout << "\n";
}

// 打印错误信息
void print_error(const std::string& msg) {
    std::cout << RED << "  [错误] " << msg << RESET << "\n";
}

// 打印成功信息
void print_ok(const std::string& msg) {
    std::cout << GREEN << "  [成功] " << msg << RESET << "\n";
}

// 打印提示信息
void print_info(const std::string& msg) {
    std::cout << YELLOW << "  [提示] " << msg << RESET << "\n";
}

} // namespace
