cmake_minimum_required(VERSION 3.20.0)       # Set the minimum required CMake version

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE}) # Find and load the Zephyr build system

project(test_bench)                          # Define the project name

target_sources(app PRIVATE                   # Add source files to the Zephyr application target
    src/main.c                               # Add the main application source file
    src/modules/dio/dio_service.c            # Add the DIO service source file
)
