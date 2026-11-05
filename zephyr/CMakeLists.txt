# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

zephyr_compile_options(-mcpu=cortex-a55)

project(fp16-test)

target_include_directories(app PRIVATE ../include ../common/include)

target_sources(app PRIVATE ../common/src/convld.c src/main.c)
