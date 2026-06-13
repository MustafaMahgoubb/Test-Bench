cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(dut_firmware)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE
    src/main.c
    src/dut_dio.c
    src/dut_spi.c
    src/dut_uart.c
    src/dut_pwm_out.c
    src/dut_pwm_in.c
)

