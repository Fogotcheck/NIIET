cmake_minimum_required(VERSION 3.22)

add_library(${PROJECT_NAME}_CHIP_INTERFACE INTERFACE)

add_subdirectory(K1921VG015)

target_link_libraries(
    ${PROJECT_NAME}_CHIP_INTERFACE
    INTERFACE
    ${PROJECT_NAME}_K1921VG015_HAL
)
