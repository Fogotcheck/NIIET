cmake_minimum_required(VERSION 3.22)

find_file(CHANGELOG_FILE CHANGELOG.md ${CMAKE_SOURCE_DIR})

if(NOT CHANGELOG_FILE)
    message(WARNING "Could not find CHANGELOG.md file in PATH::${CMAKE_SOURCE_DIR}")
else()
    get_filename_component(CXX_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
    get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)

    execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_REV_PARSE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(TIMESTAMP TODAY "%Y_%m_%d %H:%M")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md"
        "${CMAKE_BINARY_DIR}/CHANGELOG.md")

    install(FILES ${CMAKE_BINARY_DIR}/CHANGELOG.md DESTINATION .)
endif()
