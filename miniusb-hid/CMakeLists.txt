cmake_minimum_required(VERSION 3.13)

# Khởi tạo Pico SDK
include(pico_sdk_import.cmake)

project(miniusb_hid C CXX ASM)

pico_sdk_init()

add_executable(miniusb_hid
    main.c
    usb_descriptors.c
)

# Thêm thư mục hiện tại vào danh sách include để trình biên dịch tìm thấy tusb_config.h
target_include_directories(miniusb_hid PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Liên kết với các thư viện cần thiết của Pico SDK và TinyUSB
target_link_libraries(miniusb_hid
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_pio
)

# Cấu hình TinyUSB hoạt động ở chế độ HID
target_compile_definitions(miniusb_hid PRIVATE
    CFG_TUD_HID=1
)

# Tạo các file output (.uf2 để nạp vào RP2350)
pico_enable_stdio_usb(miniusb_hid 0)
pico_enable_stdio_uart(miniusb_hid 1)
pico_add_extra_outputs(miniusb_hid)
