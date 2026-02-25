# ===========================================================================
# idk-nagger-4 : ESP32 Cryptocurrency Miner — Root CMakeLists.txt
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
# Tested with: esp-idf v5.2.2 (xtensa-esp32 target)
# ===========================================================================

cmake_minimum_required(VERSION 3.16)

# Extra component directories
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/components")

# Pull in esp-idf build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(idk-nagger-4)
