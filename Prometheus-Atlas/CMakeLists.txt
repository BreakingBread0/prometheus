cmake_minimum_required(VERSION 3.31)

project(Prometheus-Atlas
        DESCRIPTION "Prometheus typing library."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

file(GLOB_RECURSE ATLAS_HEADERS ${PROJECT_SOURCE_DIR}/Include/Atlas/*.h)

add_library(Prometheus-Atlas INTERFACE ${ATLAS_HEADERS})

target_include_directories(Prometheus-Atlas INTERFACE ${PROJECT_SOURCE_DIR}/Include)