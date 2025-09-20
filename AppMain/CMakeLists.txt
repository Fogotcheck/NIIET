cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME ${PROJECT_NAME})

add_executable(
    ${MODULE_NAME}
    AppMain.c
)

target_link_options(
    ${MODULE_NAME}
    PRIVATE
    -T${MCU_APP_LINKER_SCRIPT}
)

target_link_libraries(
    ${MODULE_NAME}
    ${PROJECT_NAME}_CHIP_INTERFACE
    ${PROJECT_NAME}_LIB_INTERFACE
)


target_post_build(${MODULE_NAME})
