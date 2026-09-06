# This is a copy of <PICO_SDK_PATH>/external/pico_sdk_import.cmake

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif ()

if (NOT PICO_SDK_PATH)
    message(FATAL_ERROR "SDK location was not specified. Please set PICO_SDK_PATH.")
endif ()

get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH)
if (NOT EXISTS ${PICO_SDK_PATH}/pico_sdk_init.cmake)
    message(FATAL_ERROR "Directory specified by PICO_SDK_PATH (${PICO_SDK_PATH}) does not contain pico_sdk_init.cmake")
endif ()

include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
