cmake_minimum_required(VERSION 3.10)

# Project name
project(MyProject)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable target
add_executable(MAIN main.cpp fcfs.cpp srt.cpp sjf.cpp)

# Add compile options
target_compile_options(MAIN PRIVATE -Wall -Werror -g)

# Link libraries (if any)
# target_link_libraries(MAIN <library_name>)
