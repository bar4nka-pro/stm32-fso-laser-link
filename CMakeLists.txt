cmake_minimum_required(VERSION 3.20)
project(fso_laser_link C ASM)

set(CMAKE_C_STANDARD 11)

if(CMAKE_CROSSCOMPILING)

#----------STM32----------

message(STATUS "STM32F411")

    include(cmake/stm32f411_firmware.cmake)

    add_subdirectory(diplom_laser)
    add_subdirectory(diplom_laser_receive)

else()

#----------HOST-----------

 message(STATUS "Режим: хостовые тесты")

    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    include(CTest)

    add_subdirectory(tests)

endif()
