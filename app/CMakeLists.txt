#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(MY_APP_PRJ)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
)

add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    ${CMAKE_CURRENT_BINARY_DIR}/common # 指定一个二进制目录
)

target_sources(app PRIVATE
  platform_nrf/bus/m_spi.c
)

target_include_directories(app PRIVATE
  platform_nrf/bus
)
# NORDIC SDK APP END
