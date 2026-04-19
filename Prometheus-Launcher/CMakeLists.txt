cmake_minimum_required(VERSION 3.28)
project(Prometheus-Launcher
        DESCRIPTION "Pro Launcher"
        HOMEPAGE_URL "https://github.com/BreakingBread0/prometheus"
        LANGUAGES C CXX)

message("Prometheus-Launcher")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file("prometheus-nixos.sh" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prometheus-nixos.sh" COPYONLY)
configure_file("prometheus.sh" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prometheus.sh" COPYONLY)

add_executable(Prometheus-Launcher
        "main.cpp"
        simple_imgui.h
        simple_imgui.cpp
        signature.h
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/*.cpp"
        "${PROJECT_SOURCE_DIR}/*.h")

add_dependencies(Prometheus-Launcher Prometheus-Core)

target_include_directories(Prometheus-Launcher PRIVATE ${PROJECT_SOURCE_DIR}/)

# Project Config
# Pro_ProjectVersion(Prometheus-Launcher 1 0 0 0)
# Pro_ApplyPlatform(Prometheus-Launcher)
target_link_options(Prometheus-Launcher PRIVATE
        -lopengl32
        -lgdi32
        -ldwmapi
        -static
        -static-libgcc
        -static-libstdc++)

# Dll Config
set_target_properties(Prometheus-Launcher PROPERTIES OUTPUT_NAME "Prometheus")

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(Prometheus-Launcher PRIVATE nlohmann_json::nlohmann_json)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(Prometheus-Launcher PRIVATE imgui::imgui)

find_package(OpenGL REQUIRED)
target_link_libraries(Prometheus-Launcher PRIVATE OpenGL::GL)

find_package(nfd REQUIRED)
target_link_libraries(Prometheus-Launcher PRIVATE nfd::nfd)

find_package(OpenSSL REQUIRED)
target_link_libraries(Prometheus-Launcher PRIVATE OpenSSL::SSL)
target_link_libraries(Prometheus-Launcher PRIVATE OpenSSL::Crypto)

# Not using platform-folders since this will return location in wine's virtual drive.
# find_package(platform_folders REQUIRED)
# target_link_libraries(Prometheus-Launcher PRIVATE sago::platform_folders)

# not using crypt32 because windows platform security is garbage
# Just using an MD5 for now. Why use many word when few word do trick?
target_link_libraries(Prometheus-Launcher PRIVATE dwmapi wintrust)



