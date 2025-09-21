cmake_minimum_required(VERSION 3.22)

set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "GCC_RISC_V" CACHE STRING "" FORCE)

add_library(freertos_config INTERFACE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(
        freertos_config
        INTERFACE
        "configRECORD_STACK_HIGH_ADDRESS=1"
        # "configGENERATE_RUN_TIME_STATS=1"
        # "configUSE_TRACE_FACILITY=1"
    )
endif()

target_include_directories(
    freertos_config
    SYSTEM INTERFACE
    config
    custom
)

target_link_libraries(
    freertos_config
    INTERFACE
    ${PROJECT_NAME}_CHIP_INTERFACE
)

FetchContent_Declare(freertos_kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG main
)
FetchContent_MakeAvailable(freertos_kernel)

target_sources(
    freertos_kernel
    PRIVATE
    custom/freeRTOS_RiscV_provider.c
)
