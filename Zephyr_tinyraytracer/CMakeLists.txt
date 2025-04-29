# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tinyraytracer)

target_sources(app PRIVATE src/tinyraytracer.c)

zephyr_library_compile_options(-std=gnu2x -Wno-double-promotion)

# Link the math library
target_link_libraries(app PRIVATE m)
