cmake_minimum_required(VERSION 3.10)
project(lib_project C)

# 设置C标准
set(CMAKE_C_STANDARD 11)

# 开启调试和UTF-8支持
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# 包含所有头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/api
    ${PROJECT_SOURCE_DIR}/app
    ${PROJECT_SOURCE_DIR}/bsp
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/service
)

# 收集核心库源文件（不包含测试文件）
file(GLOB_RECURSE LIB_SOURCES
    api/*.c
    app/*.c
    bsp/*.c
    common/*.c
    service/*.c
)

# 生成静态库
add_library(lib STATIC ${LIB_SOURCES})

# ===================== 独立测试可执行文件 =====================

# 测试程序1：frame_parser
if(EXISTS ${PROJECT_SOURCE_DIR}/test/test_frame_parser.c)
    add_executable(test_frame_parser test/test_frame_parser.c)
    target_link_libraries(test_frame_parser PRIVATE lib)
    list(APPEND TEST_TARGETS test_frame_parser)
endif()

# 测试程序2：protocal
if(EXISTS ${PROJECT_SOURCE_DIR}/test/test_protocal.c)
    add_executable(test_protocal test/test_protocal.c)
    target_link_libraries(test_protocal PRIVATE lib)
    list(APPEND TEST_TARGETS test_protocal)
endif()

# 测试程序3：serial_monitor (仅在文件存在时添加)
if(EXISTS ${PROJECT_SOURCE_DIR}/test/test_serial_monitor.c)
    add_executable(test_serial_monitor test/test_serial_monitor.c)
    target_link_libraries(test_serial_monitor PRIVATE lib)
    list(APPEND TEST_TARGETS test_serial_monitor)
endif()

# 可选：定义一个自定义目标，用于一次性构建所有测试
if(TEST_TARGETS)
    add_custom_target(all_tests DEPENDS ${TEST_TARGETS})
endif()
