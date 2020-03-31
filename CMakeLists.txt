cmake_minimum_required(VERSION 3.9)
project(psdd)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
include_directories(include)
include_directories("/opt/Xilinx/Vivado/2018.3.op2405991/include/")
include_directories("/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx/libs/")
include_directories("/usr/include/")

if(APPLE)
  link_directories(lib/darwin)
endif(APPLE)

if(UNIX AND NOT APPLE)
  link_directories(lib/linux)
endif(UNIX AND NOT APPLE)

set(gtest_main "libgtest_main.a")
set(gtest "libgtest.a")
set(sdd "libsdd.a")
set(gmock "libgmock.a")
set(gmock_main "libgmock_main.a")
find_package(OpenCL REQUIRED)

file(GLOB PSDD_TEST_SRC_FILES "test/*.cpp")
file(GLOB LIB_PSDD_SRC_FILES "src/*.h" "src/*.cpp" "src/*.c")
file(GLOB LIB_XCL_SRC_FILES "../../../libs/xcl2/*.hpp" "../../../libs/xcl2/*.cpp" )

#add_executable(psdd_test ${PSDD_TEST_SRC_FILES})
add_library(psdd ${LIB_PSDD_SRC_FILES} )
add_library(xcl2 ${LIB_XCL_SRC_FILES} )

add_executable(psdd_inference src/psdd_inference_main.cpp)
#target_link_libraries(psdd_test psdd ${gtest} ${gtest_main} ${gmock} ${gmock_main} ${sdd} gmp pthread)
target_link_libraries(psdd_inference psdd sdd gmp OpenCL xcl2 )
