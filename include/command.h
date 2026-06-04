#pragma once
#include "fs_core.h"
#include <string>
#include <vector>
#include <memory>

// 命令基类
class Command {
public:
    Command(std::string name, std::string usage, std::string desc);
    virtual ~Command() = default;

    virtual bool execute(FileSystem& fs, const std::vector<std::string>& args) = 0;

    const std::string& name()  const { return name_; }
    const std::string& usage() const { return usage_; }
    const std::string& desc()  const { return desc_; }

protected:
    std::string name_, usage_, desc_;
};

// 别名支持：返回额外匹配的字符串数组
using CmdPtr = std::unique_ptr<Command>;

class CmdFormat : public Command {
public: CmdFormat(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdMkdir : public Command {
public: CmdMkdir(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdRmdir : public Command {
public: CmdRmdir(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdLs : public Command {
public: CmdLs(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdCd : public Command {
public: CmdCd(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdCreate : public Command {
public: CmdCreate(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdOpen : public Command {
public: CmdOpen(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdClose : public Command {
public: CmdClose(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdRead : public Command {
public: CmdRead(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdWrite : public Command {
public: CmdWrite(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdDelete : public Command {
public: CmdDelete(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdCat : public Command {
public: CmdCat(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdDf : public Command {
public: CmdDf(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdDu : public Command {
public: CmdDu(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdRename : public Command {
public: CmdRename(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdStat : public Command {
public: CmdStat(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdTree : public Command {
public: CmdTree(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdHelp : public Command {
public: CmdHelp(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};

class CmdExit : public Command {
public: CmdExit(); bool execute(FileSystem& fs, const std::vector<std::string>& args) override;
};
