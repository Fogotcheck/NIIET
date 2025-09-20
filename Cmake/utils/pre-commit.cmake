cmake_minimum_required(VERSION 3.22)

find_program(VIRTUALENV virtualenv)
find_program(SHELL PowerShell)

set(VENV_DIR "${CMAKE_SOURCE_DIR}/.venv")
set(MODULE_NAME UTILS_${PROJECT_NAME})
set(REQUIREMENTS_FILE "${CMAKE_SOURCE_DIR}/requirements-dev.txt")

if(NOT VIRTUALENV)
    message(WARNING "Could not find `virtualenv` in PATH")
else()
    add_custom_target(
        ${MODULE_NAME}_VIRTUAL_ENV
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND virtualenv ${VENV_DIR}
    )

    if(SHELL)
        add_custom_command(
            TARGET ${MODULE_NAME}_VIRTUAL_ENV
            POST_BUILD
            COMMAND ${SHELL} -Command "& { . '${CMAKE_SOURCE_DIR}/.venv/Scripts/activate.ps1'; pip install -r '${REQUIREMENTS_FILE}' }"
            COMMAND ${SHELL} -Command "& { . '${CMAKE_SOURCE_DIR}/.venv/Scripts/activate.ps1'; pre-commit install }"
        )
    else()
        add_custom_command(
            TARGET ${MODULE_NAME}_VIRTUAL_ENV
            POST_BUILD
            COMMAND bash -c "${CMAKE_SOURCE_DIR}/.venv/bin/pip install -r '${REQUIREMENTS_FILE}'"
            COMMAND bash -c "${CMAKE_SOURCE_DIR}/.venv/bin/pre-commit install"
        )
    endif()
endif()
