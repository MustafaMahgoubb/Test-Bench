cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(TestBench)

target_include_directories(include)
target_sources(${PROJECT_NAME} 
            src/main.c
            src/testbench_pwmOUT.c
)
