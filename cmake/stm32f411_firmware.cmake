function(stm32f411_add_firmware TARGET)
    # diplom_laser.elf -> diplom_laser, for .bin/.hex/.map
    get_filename_component(BASE ${TARGET} NAME_WE)

    set(MCU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

    file(GLOB VENDOR_SOURCES CONFIGURE_DEPENDS
        Drivers/STM32F4xx_HAL_Driver/Src/*.c
        USB_DEVICE/App/*.c
        USB_DEVICE/Target/*.c
        Middlewares/ST/STM32_USB_Device_Library/Core/Src/*.c
        Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/*.c
    )

    add_executable(${TARGET}
        Core/Src/main.c
        Core/Src/stm32f4xx_hal_msp.c
        Core/Src/stm32f4xx_it.c
        Core/Src/syscalls.c
        Core/Src/sysmem.c
        Core/Src/system_stm32f4xx.c
        Core/Startup/startup_stm32f411ceux.s
        ${VENDOR_SOURCES}
    )

    target_include_directories(${TARGET} PRIVATE
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

    target_compile_definitions(${TARGET} PRIVATE
        USE_HAL_DRIVER
        STM32F411xE
    )

    target_compile_options(${TARGET} PRIVATE
        ${MCU_FLAGS}
        -Wall
        -O0
        -g3
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(${TARGET} PRIVATE
        ${MCU_FLAGS}
        -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F411CEUX_FLASH.ld
        --specs=nano.specs
        --specs=nosys.specs
        -Wl,--gc-sections
        -Wl,-Map=${BASE}.map
    )

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_SIZE} ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -O binary ${TARGET} ${BASE}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex   ${TARGET} ${BASE}.hex
    )

set(PROTO_DIR ${CMAKE_SOURCE_DIR}/shared/laser_proto)

target_sources(${TARGET} PRIVATE ${PROTO_DIR}/laser_proto.c)

target_include_directories(${TARGET} PRIVATE ${PROTO_DIR})

endfunction()
