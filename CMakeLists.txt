cmake_minimum_required(VERSION 3.1)
project(ucuetis)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

option(BUILD_RELEASE "build with optimizations on" off)

if(BUILD_RELEASE)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Z3")
  else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  endif(MSVC)
else(BUILD_RELEASE)
  if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
  else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic -Wall")
  endif(MSVC)
endif(BUILD_RELEASE)

add_library(file src/file/file.cpp)
add_library(arena src/mem/arena.cpp)
add_library(scanner src/scanner/scanner.cpp)
add_library(parser src/parser/parser.cpp)

add_executable(ucuetis src/main.cpp)

target_link_libraries(scanner arena)
target_link_libraries(parser arena scanner)
target_link_libraries(ucuetis file scanner parser)
