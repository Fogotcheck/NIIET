set(CMAKE_SYSTEM_PROCESSOR "riscv32" CACHE STRING "")
set(CMAKE_SYSTEM_NAME "Generic" CACHE STRING "")

set(TOOLCHAIN_PREFIX "riscv-none-elf-")

find_program(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++" HINTS ${TOOLCHAIN_PATH})

find_program(CMAKE_AR "${TOOLCHAIN_PREFIX}ar" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_LINKER "${TOOLCHAIN_PREFIX}ld" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}objcopy" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_RANLIB "${TOOLCHAIN_PREFIX}ranlib" HINTS ${TOOLCHAIN_PATH})
find_program(CMAKE_STRIP "${TOOLCHAIN_PREFIX}strip" HINTS ${TOOLCHAIN_PATH})

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")

add_compile_options(
    -march=rv32imfc_zba_zbb_zbc_zbs_zicsr
    -mabi=ilp32f
    -nostartfiles
    -mstrict-align
    -mno-save-restore
    -ffunction-sections
    -fdata-sections
    -fno-builtin
    -Wall
)

add_link_options(
    --specs=nosys.specs
    -march=rv32imfc_zba_zbb_zbc_zbs_zicsr
    -mabi=ilp32f
    -nostartfiles

    -ffunction-sections
    -fdata-sections
    -Wl,--gc-sections

    -Wl,-Map=${PROJECT_NAME}.map,--cref,--print-memory-usage
    -Wl,--start-group
    -lc
    -lm
    -lgcc
    -Wl,--end-group
)
