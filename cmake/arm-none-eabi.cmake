#bare-metal ARM
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

#compilers C/C++/ASM
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

#where to loc tool
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

#util variables
set(CMAKE_SIZE arm-none-eabi-size)

#compiling into static lib instead of exe
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
