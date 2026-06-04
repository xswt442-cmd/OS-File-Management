#pragma once
#include <string>

namespace ui {

constexpr const char* RESET   = "\033[0m";
constexpr const char* RED     = "\033[31m";
constexpr const char* GREEN   = "\033[32m";
constexpr const char* YELLOW  = "\033[33m";
constexpr const char* BLUE    = "\033[34m";
constexpr const char* CYAN    = "\033[36m";
constexpr const char* WHITE   = "\033[37m";
constexpr const char* BOLD    = "\033[1m";
constexpr const char* DIM     = "\033[2m";

void console_init();
void splash();
void show_prompt(const std::string& cwd);
void show_help();
void print_error(const std::string& msg);
void print_ok(const std::string& msg);
void print_info(const std::string& msg);

} // namespace ui
