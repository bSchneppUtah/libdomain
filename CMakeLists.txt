cmake_minimum_required(VERSION 3.10.0)
project(libdomain CUDA CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(CUDA REQUIRED)
include_directories("${CUDA_INCLUDE_DIRS}")
include_directories("include")
add_library(domain 
	Lib.cpp
)

target_link_libraries(domain mpfr)

add_executable(test1 tests/simple.cpp)
add_executable(val_test tests/val_test.cpp)

target_link_libraries(test1 domain)
target_link_libraries(val_test domain)