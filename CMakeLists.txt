cmake_minimum_required(VERSION 3.5)

project(purrdbg
    VERSION 0.0.0
    LANGUAGES CXX
    DESCRIPTION "The nyasm debugger *purr*"
)

add_executable(purrdbg 
    main.cpp
)

target_compile_options(purrdbg PRIVATE -lncurses -ldl)
