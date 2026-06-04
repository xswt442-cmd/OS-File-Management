#include "fs_core.h"
#include "shell.h"
#include "ui.h"
#include <iostream>
#include <filesystem>

int main() {
    ui::console_init();
    ui::splash();

    FileSystem fs;
    namespace fsys = std::filesystem;
    const std::string img = "data/fs.fsimg";
    bool loaded = false;

    if (fsys::exists(img)) {
        std::cout << "\n  检测到已有磁盘镜像: data/fs.fsimg\n\n";
        std::cout << "  [1] 加载已有磁盘，恢复上次状态\n";
        std::cout << "  [2] 格式化新磁盘，丢弃旧数据\n";
        std::cout << "\n  请选择 (1/2): ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1") {
            loaded = fs.start();
            if (!loaded) ui::print_info("加载失败，将创建新磁盘");
        } else {
            std::cout << "  确认格式化? (y/n): ";
            std::string cfm;
            std::getline(std::cin, cfm);
            if (cfm == "y" || cfm == "Y") {
                fs.format();
                loaded = true;
            }
        }
    }

    if (!loaded) {
        std::cout << "\n  没有可用磁盘，自动创建新磁盘...\n";
        fs.format();
    }

    std::cout << "\n  工作目录: " << fs.pwd() << "\n";
    std::cout << "  可用块数: " << (BLOCK_COUNT - 6 - 1) << "\n\n";

    Shell shell(fs);
    shell.run();
    return 0;
}
