cmake_minimum_required(VERSION 3.31)

message("Prometheus-Typing")

message(CHECK_START "Finding IDAClang")
if(NOT DEFINED ENV{IDAPATH})
    message(CHECK_FAIL "Not Found")
    message(NOTICE "Failed to obtain IDA path. Please specify 'IDAPATH' if you want to generate IDA typing.")
    return()
endif()

find_program(IDACLANG "idaclang"
        PATHS $ENV{IDAPATH}/tools/idaclang/)

if("${IDACLANG}" STREQUAL "IDACLANG-NOTFOUND")
    message(CHECK_FAIL "Not Found")
    message(NOTICE "IDAClang could not be located. Please ensure you have installed IdaClang at $ENV{IDAPATH}/tools/idaclang/")
    return()
endif()

message(CHECK_PASS "Found")

project(Prometheus-Typing
        DESCRIPTION "Project typing & IDA Til Generation."
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

string(TOLOWER "x86_64-pc-${CMAKE_SYSTEM_NAME}" IDACLANG_TUPLE)

message("Configuring target: Prometheus-Typing with options:")
message(STATUS "Using Environments IDAPATH  : '$ENV{IDAPATH}'")
message(STATUS "Using IDAClang              : '${IDACLANG}'")
message(STATUS "Target Triple               : '${IDACLANG_TUPLE}'")

#Dummy project, this doesn't actually compile anything. Just fixes ide's being annoying
add_library(Prometheus-Typing SHARED Include.h Include.cpp)
set_target_properties(Prometheus-Typing PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Prometheus-Typing PRIVATE ${PROJECT_SOURCE_DIR}/)

#IDA TIL gen
add_custom_command(TARGET Prometheus-Typing POST_BUILD
        COMMAND ${IDACLANG}
            -x c++
            -target ${IDACLANG_TUPLE}
            --idaclang-tilname "PrometheusIDA"
            --idaclang-tildesc "Prometheus Overwatch 0.8 Library"
            Include.h
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin/ida"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/Prometheus-Typing/PrometheusIDA.til" "${CMAKE_BINARY_DIR}/bin/ida/PrometheusIDA.til"
        COMMAND ${CMAKE_COMMAND} -E rm "${CMAKE_SOURCE_DIR}/Prometheus-Typing/PrometheusIDA.til"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Prometheus-Typing
        BYPRODUCTS ${CMAKE_BINARY_DIR}/bin/ida/PrometheusIDA.til
        COMMENT "Building IDA TIL..."
)