# **[${PROJECT_NAME}-${PROJECT_VERSION}](https://github.com/Fogotcheck/NIIET)**

## *Инфо*
```
Project         ::  ${PROJECT_NAME}
Data            ::  ${TODAY}
Version         ::  ${PROJECT_VERSION}
Commit          ::  ${GIT_REV_PARSE}
Compiler        ::  ${CMAKE_C_COMPILER_ID}:${C_COMPILER_NAME}-${CMAKE_C_COMPILER_VERSION}
Build type      ::  ${CMAKE_BUILD_TYPE}
```

*Добавлено*
- 2025_20_09
    1. базовая структура проекта
- 2025_21_09
    1. изменена сборка библиотеки freeRTOS
    2. добавлены файлы провайдера для RISC-V
- 2025_25_09
    1. задействована память RAM1 под стэк и кучу
    2. расширены возможности freeRTOS за счёт конфига
