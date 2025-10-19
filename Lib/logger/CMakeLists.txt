cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME ${PROJECT_NAME}_LOGGER)

add_library(${MODULE_NAME}_INTERFACE INTERFACE)

target_include_directories(
    ${MODULE_NAME}_INTERFACE
    INTERFACE
    inc
)

add_library(${MODULE_NAME})

target_sources(
    ${MODULE_NAME}
    PRIVATE
    src/print_target.c
)

target_link_libraries(
    ${MODULE_NAME}
    ${MODULE_NAME}_INTERFACE
    ${PROJECT_NAME}_CHIP_INTERFACE
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(
        ${MODULE_NAME}_INTERFACE
        INTERFACE
        DEBUG_LOG=3
    )
else()
    target_compile_definitions(
        ${MODULE_NAME}_INTERFACE
        INTERFACE
        DEBUG_LOG=1
    )
endif()
