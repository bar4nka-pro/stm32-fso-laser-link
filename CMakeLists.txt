cmake_minimum_required(VERSION 3.20)
project(fso_laser_link C ASM)

set(CMAKE_C_STANDARD 11)

include(cmake/stm32f411_firmware.cmake)

add_subdirectory(diplom_laser)
add_subdirectory(diplom_laser_receive)
