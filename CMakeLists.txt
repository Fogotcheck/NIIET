cmake_minimum_required(VERSION 3.22)

project(exmp VERSION 0.3.0 LANGUAGES CXX C ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")
set(CPACK_PACKAGE_CHECKSUM MD5)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")

include(FetchContent)
include(CPack)
include(Cmake/opts/custom_build_opts.cmake)
include(Cmake/utils/pre-commit.cmake)
include(Cmake/utils/change_log.cmake)

add_subdirectory(Chip)
add_subdirectory(Lib)
add_subdirectory(AppMain)

target_install_binary(${PROJECT_NAME})
