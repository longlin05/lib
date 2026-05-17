cmake_minimum_required(VERSION 3.20)
project(lib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 开启调试和UTF-8支持
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

# 1. 把 lib 定义为静态库，只放核心业务文件
add_library(lib STATIC
        mem_pool.c
        event_cb.c
        reg.c
        ring_fifo.c
        critical.c
        reg.c
)

# 2. 把 test_reg 做成独立的可执行文件，并链接 lib 库
add_executable(test_reg test_reg.c)
target_link_libraries(test_reg lib)

# 3. 把 test_ring_fifo 也做成独立的可执行文件
add_executable(test_ring_fifo test_ring_fifo.c)
target_link_libraries(test_ring_fifo lib)