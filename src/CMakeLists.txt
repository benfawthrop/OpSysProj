cmake_minimum_required(VERSION 3.10)

# Project name
project(cpu_scheduler)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable target
add_executable(MAIN main.cpp fcfs.cpp rr.cpp)

# Add compile options
target_compile_options(MAIN PRIVATE -Wall -Werror -g)

# Custom target to run the executable and redirect stdout to a file
add_custom_target(run_with_redirect
        COMMAND MAIN 3 1 32 0.001 1024 4 0.75 256 > student1.txt
        DEPENDS MAIN
        COMMENT "Running MAIN and redirecting output to output.txt"
)
