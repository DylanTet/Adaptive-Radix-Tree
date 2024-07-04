cmake_minimum_required(VERSION 2.27)

project(ART_DS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(art INTERFACE)
target_include_directories(
    art
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)
### EXECUTABLES ###

# Main executable
add_executable(main 
  "${PROJECT_SOURCE_DIR}/src/example.cpp"
  )
target_link_libraries(main)
