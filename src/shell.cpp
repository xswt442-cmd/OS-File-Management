#include "shell.h"
#include "ui.h"
#include "utils.h"
#include <iostream>

// Shell类
Shell::Shell(FileSystem& fs) : fs_(fs) {
    register_commands();
}

void Shell::register_commands() {
    commands_.push_back(std::make_unique<CmdFormat>());
    commands_.push_back(std::make_unique<CmdMkdir>());
    commands_.push_back(std::make_unique<CmdRmdir>());
    commands_.push_back(std::make_unique<CmdLs>());
    commands_.push_back(std::make_unique<CmdCd>());
    commands_.push_back(std::make_unique<CmdCreate>());
    commands_.push_back(std::make_unique<CmdOpen>());
    commands_.push_back(std::make_unique<CmdClose>());
    commands_.push_back(std::make_unique<CmdRead>());
    commands_.push_back(std::make_unique<CmdWrite>());
    commands_.push_back(std::make_unique<CmdDelete>());
    commands_.push_back(std::make_unique<CmdCat>());
    commands_.push_back(std::make_unique<CmdDf>());
    commands_.push_back(std::make_unique<CmdDu>());
    commands_.push_back(std::make_unique<CmdRename>());
    commands_.push_back(std::make_unique<CmdStat>());
    commands_.push_back(std::make_unique<CmdTree>());
    commands_.push_back(std::make_unique<CmdHelp>());
    commands_.push_back(std::make_unique<CmdExit>());
}

// 查找命令
Command* Shell::find(const std::string& name) {
    for (auto& cmd : commands_) {
        if (cmd->name() == name) return cmd.get();
    }
    // 别名
    if (name == "dir")  return find("ls");
    if (name == "rm")   return find("delete");
    if (name == "ren")  return find("rename");
    if (name == "mv")   return find("rename");
    if (name == "quit") return find("exit");
    return nullptr;
}

// 解析输入行
std::vector<std::string> Shell::tokenize(const std::string& line) {
    std::vector<std::string> args;
    std::string cur;
    bool in_quote = false;
    for (char c : line) {
        if (c == '"') {
            in_quote = !in_quote;
        } else if (c == ' ' && !in_quote) {
            if (!cur.empty()) { args.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) args.push_back(cur);
    return args;
}

// 显示帮助信息
void Shell::show_help() {
    std::cout << ui::BOLD << "\n  可用命令:\n\n" << ui::RESET;
    for (auto& cmd : commands_) {
        std::string sig = cmd->name();
        if (!cmd->usage().empty()) sig += " " + cmd->usage();
        std::cout << "  " << ui::GREEN << sig << ui::RESET;
        // 对齐
        size_t pad = (sig.size() < 24) ? (24 - sig.size()) : 1;
        std::cout << std::string(pad, ' ') << cmd->desc() << "\n";
    }
    std::cout << "\n";
}

// 运行 Shell
void Shell::run() {
    running_ = true;
    std::string line;

    while (running_) {
        ui::show_prompt(fs_.pwd());
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        auto args = tokenize(line);
        if (args.empty()) continue;

        std::string cmd_name = args[0];
        Command* cmd = find(cmd_name);

        if (!cmd) {
            ui::print_error("未知命令: " + cmd_name + "（输入 help 查看帮助）");
            continue;
        }

        if (cmd->name() == "help") {
            show_help();
            continue;
        }

        if (cmd->name() == "exit") {
            cmd->execute(fs_, args);
            running_ = false;
            break;
        }

        cmd->execute(fs_, args);
    }
}
