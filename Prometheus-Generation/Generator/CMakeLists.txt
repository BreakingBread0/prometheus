cmake_minimum_required(VERSION 3.31)

message("Prometheus-Generator")

project(Prometheus-Generator
        DESCRIPTION "Generator for C++ files."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

set(DIR_BIN ${CMAKE_BINARY_DIR}/bin/core)
set(DIR_LIB ${CMAKE_BINARY_DIR}/lib/core)

add_executable(Prometheus-Generator
        "Generator.h"
        "Generator.cpp"
)

target_include_directories(Prometheus-Generator PRIVATE ${PROJECT_SOURCE_DIR}/)

Pro_ProjectVersion(Prometheus-Generator 1 0 0 0)
Pro_ApplyPlatform(Prometheus-Generator)
Pro_SetOutput(Prometheus-Generator ${DIR_BIN} ${DIR_LIB})

set_target_properties(Prometheus-Generator PROPERTIES OUTPUT_NAME "PrometheusGenerator")

target_link_libraries(Prometheus-Generator PRIVATE Prometheus-Generation)