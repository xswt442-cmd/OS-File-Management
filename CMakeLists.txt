cmake_minimum_required(VERSION 3.14)
project(OSFileManagement VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 源文件
set(HEADERS
    include/fs_core.h
    include/super_block.h
    include/fat.h
    include/bitmap.h
    include/directory.h
    include/file_op.h
    include/disk_io.h
    include/ui.h
    include/utils.h
    include/command.h
    include/shell.h
)

set(SOURCES
    src/main.cpp
    src/fs_core.cpp
    src/super_block.cpp
    src/fat.cpp
    src/bitmap.cpp
    src/directory.cpp
    src/file_op.cpp
    src/disk_io.cpp
    src/ui.cpp
    src/utils.cpp
    src/commands.cpp
    src/shell.cpp
)

# 可执行文件
add_executable(osfs ${SOURCES} ${HEADERS})

# 包含路径
target_include_directories(osfs PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 平台相关
if(MSVC)
    # MSVC: 禁用 warning C4996 (不安全函数), 启用多核编译
    target_compile_options(osfs PRIVATE /W4 /wd4996 /MP)
else()
    target_compile_options(osfs PRIVATE -Wall -Wextra -pedantic)
endif()

# ── 创建 data/ 和 example/ 目录 ──────────────────
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/data)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/example)
