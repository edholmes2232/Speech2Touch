cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_NUCLEO_64 
	USE_HAL_DRIVER 
	STM32WB55xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_SOURCE_DIR}/Core/Inc
    ${CMAKE_SOURCE_DIR}/USB_Device/App
    ${CMAKE_SOURCE_DIR}/USB_Device/Target
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Inc/Legacy
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/P-NUCLEO-WB55.Nucleo
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32WBxx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_SOURCE_DIR}/USB_Device/Target/usbd_conf.c
    ${CMAKE_SOURCE_DIR}/USB_Device/App/usb_device.c
    ${CMAKE_SOURCE_DIR}/USB_Device/App/usbd_desc.c
    ${CMAKE_SOURCE_DIR}/USB_Device/App/usbd_custom_hid_if.c
    ${CMAKE_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_SOURCE_DIR}/Core/Src/gpio.c
    ${CMAKE_SOURCE_DIR}/Core/Src/memorymap.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32wbxx_it.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32wbxx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/Core/Src/sysmem.c
    ${CMAKE_SOURCE_DIR}/Core/Src/syscalls.c
    ${CMAKE_SOURCE_DIR}/startup_stm32wb55xx_cm4.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_SOURCE_DIR}/Core/Src/system_stm32wbxx.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pcd_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_usb.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_ll_rcc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_rcc_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_flash_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_gpio.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_hsem.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_dma_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_pwr_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_cortex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_exti.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_usart.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_usart_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32WBxx_HAL_Driver/Src/stm32wbxx_hal_uart.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/P-NUCLEO-WB55.Nucleo/stm32wbxx_nucleo.c
)

# Drivers Midllewares


set(USB_Device_Library_Src
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    USB_Device_Library	
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Create USB_Device_Library static library
add_library(USB_Device_Library OBJECT)
target_sources(USB_Device_Library PRIVATE ${USB_Device_Library_Src})
target_link_libraries(USB_Device_Library PUBLIC stm32cubemx)

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
