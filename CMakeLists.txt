cmake_minimum_required(VERSION 3.12...4.0)

project(
    Cini
    VERSION 0.1.0
    DESCRIPTION "INI-Parser written in C"
    LANGUAGES C
)

add_library(
    cini
    STATIC src-c/ inc-c/
)

set_target_properties(
    cini
    PROPERTIES LINKER_LANGUAGE C
)
target_compile_features(
    cini
    PUBLIC c_std_99
)

target_include_directories(
    cini
    PUBLIC inc-c/
)

