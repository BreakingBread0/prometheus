cmake_minimum_required(VERSION 3.31)

project(Prometheus-Generation
        DESCRIPTION "Provides RTTI probing & extraction."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

add_library(Prometheus-Generation INTERFACE
        "Include/Probing/Probing.h"
        "Include/Probing/Probe_STU.h"
)

target_include_directories(Prometheus-Generation INTERFACE ${PROJECT_SOURCE_DIR}/Include)
target_link_libraries(Prometheus-Generation INTERFACE Prometheus-Atlas-Extensions)