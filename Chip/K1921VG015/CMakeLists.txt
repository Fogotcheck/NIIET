cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME ${PROJECT_NAME}_K1921VG015_HAL)

set(MCU_APP_LINKER_SCRIPT
    "${CMAKE_CURRENT_SOURCE_DIR}/k1921vg015_flash.ld"
    CACHE STRING "" FORCE
)

FetchContent_Declare(
    niat
    GIT_REPOSITORY https://gitflic.ru/project/niiet/niiet_riscv_sdk.git
    GIT_TAG 4b592265fa66c9ec01e9c741d5926a9a297ec30e
)

FetchContent_MakeAvailable(niat)

add_library(${MODULE_NAME}_INTERFACE INTERFACE)

target_include_directories(
    ${MODULE_NAME}_INTERFACE
    INTERFACE
    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/include
    ${niat_SOURCE_DIR}/platform/plib015/inc
)

target_sources(
    ${MODULE_NAME}_INTERFACE
    INTERFACE
    startup_k1921vg015.S
)

target_compile_definitions(
    ${MODULE_NAME}_INTERFACE
    INTERFACE
    SYSCLK_PLL
    HSECLK_VAL=16000000
    RETARGET
    CKO_PLL0
)

# file(GLOB_RECURSE K1921VG015_CORE
#     ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/*.c
# )
# file(GLOB_RECURSE K1921VG015_PLIB015
#     ${niat_SOURCE_DIR}/platform/plib015/src/*.c
# )

set(
    K1921VG015_CORE
    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/mtimer.c
    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/plic.c
    # ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/printf.c
    # ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/retarget.c

    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/startup_k1921vg015.S
    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/sys_init.c
    ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/system_k1921vg015.c

    # ${niat_SOURCE_DIR}/platform/Device/K1921VG015/source/riscv-irq.c
    custom/src/riscv-irq.c
)

set(
    K1921VG015_PLIB015
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_adcsar.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_adcsd.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_crc.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_crypto.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_dma.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_gpio.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_hash.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_i2c.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_qspi.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_rcu.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_spi.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_trng.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_uart.c
    ${niat_SOURCE_DIR}/platform/plib015/src/plib015_wdt.c

    # ${niat_SOURCE_DIR}/platform/plib015/src/plib015_flash.c
    custom/src/plib015_flash.c
)

add_library(${MODULE_NAME})

target_sources(
    ${MODULE_NAME}
    PRIVATE
    ${K1921VG015_CORE}
    ${K1921VG015_PLIB015}
)

target_link_libraries(
    ${MODULE_NAME}
    ${MODULE_NAME}_INTERFACE
)
