cmake_minimum_required(VERSION 3.31)
include(CMake/Platform/${PRO_PLATFORM}.cmake)

project(Prometheus
        DESCRIPTION "Research and Reverse-Engineering project of Overwatch!"
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus")

include_directories(external)

include(CMake/Configure.cmake)
include(CMake/Version.cmake)

#Core
add_subdirectory(Prometheus-Core)

#Patcher (can just build from vs/rider tho)
add_custom_target("Prometheus-Patcher"
        COMMAND dotnet restore
        COMMAND ${CMAKE_VS_MSBUILD_COMMAND} ${CMAKE_SOURCE_DIR}/Prometheus-Patcher/Prometheus-Patcher.sln /t:Build /p:Configuration=${CMAKE_BUILD_TYPE} /p:Platform="x64"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Prometheus-Patcher
        COMMENT "Building patcher..."
)

#IDA
add_subdirectory(Prometheus-Typing)