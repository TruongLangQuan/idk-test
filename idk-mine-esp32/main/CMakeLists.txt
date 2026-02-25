# ===========================================================================
# idk-nagger-4 : Main Component CMakeLists.txt
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# This file registers all source files under main/ as part of the
# esp-idf MAIN component. Sub-directories mirror the firmware's
# logical layers: crypto, miner, net, system.
# ===========================================================================

# Collect all .c source files from subdirectories
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "../config"
    REQUIRES
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        esp_timer
        esp_system
        lwip
        mbedtls
        json
        freertos
        driver
        esp_http_server
)

# ---------------------------------------------------------------------------
# Generate build_info.c with compile-time metadata
# ---------------------------------------------------------------------------
# Determine git commit hash (or "unknown" if not in a repo)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_COMMIT)
    set(GIT_COMMIT "unknown")
endif()

string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%S")

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    BUILD_GIT_COMMIT=\"${GIT_COMMIT}\"
    BUILD_DATE=\"${BUILD_DATE}\"
)
