cmake_minimum_required(VERSION 3.28)

project(Prometheus-Atlas-Extensions
        DESCRIPTION "Prometheus typing library runtime extensions."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

message("Prometheus-Atlas-Extensions")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ATLAS_EXT_HEADERS
        ${PROJECT_SOURCE_DIR}/Src/AtlasExt/*.h
        ${PROJECT_SOURCE_DIR}/Src/AtlasExt/*.cpp)

add_library(Prometheus-Atlas-Extensions INTERFACE)
target_include_directories(Prometheus-Atlas-Extensions INTERFACE ${PROJECT_SOURCE_DIR}/Src)
target_compile_definitions(Prometheus-Atlas-Extensions INTERFACE ATLAS_EXTENSIONS)
target_link_libraries(Prometheus-Atlas-Extensions INTERFACE Prometheus-Atlas)

add_library(Prometheus-Atlas-Extensions-Obj OBJECT ${ATLAS_EXT_HEADERS})
target_include_directories(Prometheus-Atlas-Extensions-Obj PUBLIC ${PROJECT_SOURCE_DIR}/Src)
target_compile_definitions(Prometheus-Atlas-Extensions-Obj PUBLIC ATLAS_EXTENSIONS)
target_link_libraries(Prometheus-Atlas-Extensions-Obj PUBLIC Prometheus-Atlas)

Pro_ApplyPlatform(Prometheus-Atlas-Extensions-Obj)