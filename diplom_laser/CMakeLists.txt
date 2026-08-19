cmake_minimum_required(VERSION 3.20)
project(diplom_laser C ASM)

set(CMAKE_C_STANDARD 11)

#core flags (for compile / linking)
set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

file(GLOB VENDOR_SOURCES CONFIGURE_DEPENDS
Drivers/STM32F4xx_HAL_Driver/Src/*.c
USB_DEVICE/App/*.c
USB_DEVICE/Target/*.c
Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/*.c
)

add_executable(diplom_laser.elf
Core/Src/main.c
Core/Src/stm32f4xx_hal_msp.c
Core/Src/stm32f4xx_it.c
Core/Src/syscalls.c
Core/Src/sysmem.c
Core/Src/system_stm32f4xx.c
Core/Startup/startup_stm32f411ceux.s
${VENDOR_SOURCES}
)

target_include_directories(diplom_laser.elf PRIVATE
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    USB_DEVICE/App
    USB_DEVICE/Target
    Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
)

target_compile_definitions(diplom_laser.elf PRIVATE
    USE_HAL_DRIVER
    STM32F411xE
)

target_compile_options(diplom_laser.elf PRIVATE
    ${MCU_FLAGS}
    -Wall
    -O0
    -g3
    -ffunction-sections
    -fdata-sections
)

target_link_options(diplom_laser.elf PRIVATE
    ${MCU_FLAGS}
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F411CEUX_FLASH.ld
    --specs=nano.specs
    --specs=nosys.specs
    -Wl,--gc-sections
    -Wl,-Map=diplom_laser.map
)

add_custom_command(TARGET diplom_laser.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} diplom_laser.elf
    COMMAND ${CMAKE_OBJCOPY} -O binary diplom_laser.elf diplom_laser.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex diplom_laser.elf diplom_laser.hex
)
