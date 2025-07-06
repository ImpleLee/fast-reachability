cmake_minimum_required(VERSION 3.23)
# need at least c++23 clang or whatever version of g++ is required for c++26/c++2b
project(Fast_Reachability LANGUAGES CXX)

# Create an interface target (header-only library)
add_library(Fast_Reachability INTERFACE)

# Add the current directory as an include path
target_include_directories(Fast_Reachability INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)