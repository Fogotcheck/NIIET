cmake_minimum_required(VERSION 3.22)

find_program(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size HINTS ${TOOLCHAIN_PATH})

function(target_post_build TargetName)
    add_custom_command(
        TARGET ${TargetName} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TargetName}> ${CMAKE_BINARY_DIR}/${TargetName}.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TargetName}> ${CMAKE_BINARY_DIR}/${TargetName}.hex
        COMMAND ${CMAKE_OBJDUMP} -S $<TARGET_FILE:${TargetName}> > ${CMAKE_BINARY_DIR}/${TargetName}.S
        COMMAND ${CMAKE_NM} -a -l -S -s $<TARGET_FILE:${TargetName}> > ${CMAKE_BINARY_DIR}/${TargetName}_sort.map
    )

    if(CMAKE_SIZE)
        add_custom_command(
            TARGET ${TargetName} POST_BUILD
            COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:${TargetName}>
            COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${TargetName}>
        )
    endif()

    add_custom_command(
        TARGET ${TargetName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Build - success"
        COMMAND ${CMAKE_COMMAND} -E echo "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}"
    )
    install(TARGETS ${TargetName} DESTINATION bin/${TargetName})
    install(FILES ${CMAKE_BINARY_DIR}/${TargetName}.bin DESTINATION bin/${TargetName})
    install(FILES ${CMAKE_BINARY_DIR}/${TargetName}.hex DESTINATION bin/${TargetName})
endfunction(target_post_build TargetName)

function(target_install_binary TargetName)
    add_custom_target(
        OPTS_${TargetName}_CPACK
        COMMAND cpack -G ZIP
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installing ${PROJECT_NAME}"
    )
endfunction(target_install_binary TargetName)
