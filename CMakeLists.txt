cmake_minimum_required(VERSION 3.28)

project(Prometheus
        DESCRIPTION "Research and Reverse-Engineering project of Overwatch!"
        HOMEPAGE_URL "https://github.com/saturn-xvi/prometheus")

include(CMake/Platform/${PRO_PLATFORM}.cmake)
include(CMake/Configure.cmake)
include(CMake/Version.cmake)

message("Binary output root directory: ${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)

execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(
        GIT_HASH=${GIT_HASH}
        GIT_BRANCH=${GIT_BRANCH}
)

#Atlas
add_subdirectory(Prometheus-Atlas)

#Atlas-Extensions
add_subdirectory(Prometheus-Atlas/Extensions)

#Atlas-Ida
add_subdirectory(Prometheus-Atlas/Ida)

##Generation
add_subdirectory(Prometheus-Generation)

##Generator
add_subdirectory(Prometheus-Generation/Generator)

#Core
add_subdirectory(Prometheus-Core)

#Patcher (can just build from vs/rider tho)
add_custom_target("Prometheus-Patcher"
        COMMAND dotnet restore
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_SOURCE_DIR}/Prometheus-Patcher/Prometheus-Patcher.sln /t:Build /p:Configuration=${CMAKE_BUILD_TYPE} /p:Platform="x64"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Prometheus-Patcher
        COMMENT "Building patcher..."
)