# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

set(BOARD m5stack_core2/esp32/procpu)
set(CONF_FILE prj.conf overlay-wifi.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(weather_station)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE 
    src/wifi.c
    src/sockets.c
    src/weather_station.c
    src/main.c
)
target_sources_ifdef(CONFIG_WIFI app PRIVATE src/wifi.c)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)

generate_inc_file_for_target(
    app
    src/globalsign_r1.der
    ${gen_dir}/globalsign_r1.der.inc
    )
