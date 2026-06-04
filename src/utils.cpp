#include "utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

// 获取当前时间戳
uint32_t current_timestamp() {
    return static_cast<uint32_t>(std::time(nullptr));
}

// 格式化文件大小
std::string format_size(uint32_t bytes) {
    if (bytes < 1024)
        return std::to_string(bytes) + "B";
    if (bytes < 1024 * 1024)
        return std::to_string(bytes / 1024) + "." + std::to_string((bytes % 1024) / 103) + "K";
    return std::to_string(bytes / (1024 * 1024)) + "." + std::to_string((bytes % (1024 * 1024)) / 102400) + "M";
}

// 格式化时间戳
std::string format_time(uint32_t ts) {
    if (ts == 0) return "-";
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm tm_buf;
    localtime_s(&tm_buf, &t);
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M");
    return oss.str();
}

// 去除两端
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// 分割路径
std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> parts;
    if (path.empty()) return parts;
    std::string s = path;
    // 统一分隔符
    std::replace(s.begin(), s.end(), '\\', '/');
    size_t start = (s[0] == '/') ? 1 : 0;
    std::stringstream ss(s.substr(start));
    std::string item;
    while (std::getline(ss, item, '/')) {
        if (!item.empty() && item != ".")
            parts.push_back(item);
    }
    return parts;
}
