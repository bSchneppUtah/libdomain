cmake_minimum_required(VERSION 3.10.0)
project(domain CUDA CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(CUDA REQUIRED)
find_package(Threads REQUIRED)
include_directories("${CUDA_INCLUDE_DIRS}")
include_directories("include")
add_library(domain 
	Lib.cpp
)

target_link_libraries(domain mpfr gmp Threads::Threads)

add_executable(test1 tests/simple.cpp)
target_link_libraries(test1 domain)

add_executable(val_test tests/val_test.cpp)
target_link_libraries(val_test domain)

add_executable(ltr-125-pt tests/ltr-125-pt.cpp)
target_link_libraries(ltr-125-pt domain)

add_executable(balanced-125-pt tests/balanced-125-pt.cpp)
target_link_libraries(balanced-125-pt domain)

add_executable(ltr-5-pt tests/ltr-5-pt.cpp)
target_link_libraries(ltr-5-pt domain)

add_executable(balanced-5-pt tests/balanced-5-pt.cpp)
target_link_libraries(balanced-5-pt domain)

add_executable(bgrt-ltr-5-pt tests/bgrt-ltr-5-pt.cpp)
target_link_libraries(bgrt-ltr-5-pt domain)

add_executable(bgrt-ltr-125-pt tests/bgrt-ltr-125-pt.cpp)
target_link_libraries(bgrt-ltr-125-pt domain)