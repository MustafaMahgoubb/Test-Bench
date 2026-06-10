cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(TestBench)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE 
    src/main.c
    src/testbench_pwmOUT.c
    src/testbench_pwmIN.c
    src/testbench_uart.c
)
