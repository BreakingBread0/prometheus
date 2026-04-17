cmake_minimum_required(VERSION 3.28)

message("Prometheus-Generator")

project(Prometheus-Generator
        DESCRIPTION "Generator for C++ files."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

set(DIR_BIN ${CMAKE_BINARY_DIR}/bin/gen)
set(DIR_LIB ${CMAKE_BINARY_DIR}/lib/gen)

add_executable(Prometheus-Generator
        $<TARGET_OBJECTS:Prometheus-Atlas-Extensions-Obj>
        "Generator.h"
        "Generator.cpp"
)

target_include_directories(Prometheus-Generator PRIVATE ${PROJECT_SOURCE_DIR}/)

# Project Config
Pro_ProjectVersion(Prometheus-Generator 1 0 0 0)
Pro_ApplyPlatform(Prometheus-Generator)
Pro_SetOutput(Prometheus-Generator ${DIR_BIN} ${DIR_LIB})

# Exe Config
set_target_properties(Prometheus-Generator PROPERTIES OUTPUT_NAME "PrometheusGenerator")

# Dependencies
target_link_libraries(Prometheus-Generator PRIVATE Prometheus-Atlas-Extensions)

target_link_libraries(Prometheus-Generator PRIVATE Prometheus-Generation)

