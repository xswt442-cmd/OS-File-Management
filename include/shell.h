#pragma once
#include "command.h"
#include <vector>
#include <memory>
#include <string>

class Shell {
public:
    explicit Shell(FileSystem& fs);
    void run();

    const std::vector<CmdPtr>& commands() const { return commands_; }

private:
    FileSystem& fs_;
    std::vector<CmdPtr> commands_;
    bool running_ = false;

    void register_commands();
    Command* find(const std::string& name);
    std::vector<std::string> tokenize(const std::string& line);
    void show_help();
};
