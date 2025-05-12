cmake_minimum_required(VERSION 3.14)
project(BLang VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main library
add_library(blang_lib
    # Source files will be added as they are implemented in the lessons
    # src/lexer.cpp
    # src/parser.cpp
    # src/ast.cpp
    # src/semantics.cpp
    # src/interpreter.cpp
)

# Main executable
add_executable(blang src/main.cpp)
target_link_libraries(blang blang_lib)

# Enable testing
enable_testing()
add_subdirectory(test) 