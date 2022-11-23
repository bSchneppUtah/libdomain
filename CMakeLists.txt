cmake_minimum_required(VERSION 3.10.0)
project(domain CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
include_directories("include")
add_library(domain 
	Lib.cpp
)

# MPI is OPTIONAL.
# But if it exists, we can also get the MPI test code.
find_package(MPI)
if (MPI_FOUND)
	include_directories(${MPI_INCLUDE_PATH})
endif()

target_link_libraries(domain mpfr gmp Threads::Threads)
option(DOMAIN_BUILD_TESTS "build tests for libdomain" ON)

if (DOMAIN_BUILD_TESTS)
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

	add_executable(bgrt-ltr-7-pt tests/bgrt-ltr-7-pt.cpp)
	target_link_libraries(bgrt-ltr-7-pt domain)

	add_executable(bgrt-ltr-9-pt tests/bgrt-ltr-9-pt.cpp)
	target_link_libraries(bgrt-ltr-9-pt domain)

	add_executable(bgrt-ltr-13-pt tests/bgrt-ltr-13-pt.cpp)
	target_link_libraries(bgrt-ltr-13-pt domain)

	add_executable(bgrt-ltr-25-pt tests/bgrt-ltr-25-pt.cpp)
	target_link_libraries(bgrt-ltr-25-pt domain)

	add_executable(bgrt-ltr-27-pt tests/bgrt-ltr-27-pt.cpp)
	target_link_libraries(bgrt-ltr-27-pt domain)

	add_executable(bgrt-ltr-125-pt tests/bgrt-ltr-125-pt.cpp)
	target_link_libraries(bgrt-ltr-125-pt domain)

	add_executable(bgrt-ltr-poisson tests/bgrt-ltr-poisson.cpp)
	target_link_libraries(bgrt-ltr-poisson domain)

	add_executable(bgrt-balanced-5-pt tests/bgrt-balanced-5-pt.cpp)
	target_link_libraries(bgrt-balanced-5-pt domain)

	add_executable(bgrt-balanced-7-pt tests/bgrt-balanced-7-pt.cpp)
	target_link_libraries(bgrt-balanced-7-pt domain)

	add_executable(bgrt-balanced-9-pt tests/bgrt-balanced-9-pt.cpp)
	target_link_libraries(bgrt-balanced-9-pt domain)

	add_executable(bgrt-balanced-13-pt tests/bgrt-balanced-13-pt.cpp)
	target_link_libraries(bgrt-balanced-13-pt domain)

	add_executable(bgrt-balanced-25-pt tests/bgrt-balanced-25-pt.cpp)
	target_link_libraries(bgrt-balanced-25-pt domain)

	add_executable(bgrt-balanced-27-pt tests/bgrt-balanced-27-pt.cpp)
	target_link_libraries(bgrt-balanced-27-pt domain)

	add_executable(bgrt-balanced-125-pt tests/bgrt-balanced-125-pt.cpp)
	target_link_libraries(bgrt-balanced-125-pt domain)

	add_executable(bgrt-balanced-poisson tests/bgrt-balanced-poisson.cpp)
	target_link_libraries(bgrt-balanced-poisson domain)

	add_executable(double-bgrt-ltr-5-pt tests/double/bgrt-ltr-5-pt.cpp)
	target_link_libraries(double-bgrt-ltr-5-pt domain)

	add_executable(double-bgrt-ltr-7-pt tests/double/bgrt-ltr-7-pt.cpp)
	target_link_libraries(double-bgrt-ltr-7-pt domain)

	add_executable(double-bgrt-ltr-9-pt tests/double/bgrt-ltr-9-pt.cpp)
	target_link_libraries(double-bgrt-ltr-9-pt domain)

	add_executable(double-bgrt-ltr-13-pt tests/double/bgrt-ltr-13-pt.cpp)
	target_link_libraries(double-bgrt-ltr-13-pt domain)

	add_executable(double-bgrt-ltr-25-pt tests/double/bgrt-ltr-25-pt.cpp)
	target_link_libraries(double-bgrt-ltr-25-pt domain)

	add_executable(double-bgrt-ltr-27-pt tests/double/bgrt-ltr-27-pt.cpp)
	target_link_libraries(double-bgrt-ltr-27-pt domain)

	add_executable(double-bgrt-ltr-125-pt tests/double/bgrt-ltr-125-pt.cpp)
	target_link_libraries(double-bgrt-ltr-125-pt domain)

	add_executable(double-bgrt-ltr-poisson tests/double/bgrt-ltr-poisson.cpp)
	target_link_libraries(double-bgrt-ltr-poisson domain)

	add_executable(double-bgrt-balanced-5-pt tests/double/bgrt-balanced-5-pt.cpp)
	target_link_libraries(double-bgrt-balanced-5-pt domain)

	add_executable(double-bgrt-balanced-7-pt tests/double/bgrt-balanced-7-pt.cpp)
	target_link_libraries(double-bgrt-balanced-7-pt domain)

	add_executable(double-bgrt-balanced-9-pt tests/double/bgrt-balanced-9-pt.cpp)
	target_link_libraries(double-bgrt-balanced-9-pt domain)

	add_executable(double-bgrt-balanced-13-pt tests/double/bgrt-balanced-13-pt.cpp)
	target_link_libraries(double-bgrt-balanced-13-pt domain)

	add_executable(double-bgrt-balanced-25-pt tests/double/bgrt-balanced-25-pt.cpp)
	target_link_libraries(double-bgrt-balanced-25-pt domain)

	add_executable(double-bgrt-balanced-27-pt tests/double/bgrt-balanced-27-pt.cpp)
	target_link_libraries(double-bgrt-balanced-27-pt domain)

	add_executable(double-bgrt-balanced-125-pt tests/double/bgrt-balanced-125-pt.cpp)
	target_link_libraries(double-bgrt-balanced-125-pt domain)

	add_executable(double-bgrt-balanced-poisson tests/double/bgrt-balanced-poisson.cpp)
	target_link_libraries(double-bgrt-balanced-poisson domain)


	if (MPI_FOUND)
		add_executable(bgrt-balanced-125-pt-mpi tests/bgrt-balanced-125-pt-mpi.cpp)
		target_link_libraries(bgrt-balanced-125-pt-mpi domain ${MPI_LIBRARIES})
	endif()

endif()
