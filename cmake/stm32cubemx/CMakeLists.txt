cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM, C and C++ languages
enable_language(C CXX ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_HAL_DRIVER 
	STM32U585xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/App
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32U5xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/CMSIS/RTOS2/Include/
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/App/app_touchgfx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/TouchGFXConfiguration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/STM32DMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/generated/OSWrappers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/TouchGFXHAL.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/STM32TouchController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../TouchGFX/target/TouchGFXGPIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/app_freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gpdma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/icache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u5xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u5xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32u5xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32u585xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32u5xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_crc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_gtzc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_icache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_rtc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/STM32U5xx_HAL_Driver/Src/stm32u5xx_hal_uart_ex.c
)

# Drivers Midllewares


set(RTOS2_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    RTOS2	
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)
# Validate that STM32CubeMX code is compatible with C++ standard
if(CMAKE_CXX_STANDARD LESS 11)
    message(ERROR "Generated code requires C++11 or higher")
endif()

# Create RTOS2 static library
add_library(RTOS2 OBJECT)
target_sources(RTOS2 PRIVATE ${RTOS2_Src})
target_link_libraries(RTOS2 PUBLIC stm32cubemx)

# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
