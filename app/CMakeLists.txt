#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

#导入外部模块
set(ZEPHYR_EXTRA_MODULES "../../common")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(MY_APP_PRJ)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
  src/gui.c
)

add_subdirectory(platform_nrf)

