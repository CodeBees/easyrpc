cmake_minimum_required(VERSION 2.8)
project(server)

set(OUTPUTNAME server)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated -Wno-unused-variable -g -O2 -std=c++14")

aux_source_directory(. DIR_SRCS)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../..)
include_directories(${PROJECT_SOURCE_DIR}/../protocol)
include_directories(${PROJECT_SOURCE_DIR}/../../easyrpc)
include_directories(${PROJECT_SOURCE_DIR}/../../easypack)
include_directories(${PROJECT_SOURCE_DIR}/../../spdlog/include)
include_directories(${PROJECT_SOURCE_DIR}/../googletest/googletest/include)
include_directories($ENV{BOOST_INCLUDE_PATH})

link_directories($ENV{BOOST_LIB_PATH})
link_directories(${PROJECT_SOURCE_DIR}/../googletest/googlemork/gtest)

add_executable(${OUTPUTNAME} ${DIR_SRCS})

target_link_libraries(${OUTPUTNAME} boost_serialization)
target_link_libraries(${OUTPUTNAME} boost_system)
target_link_libraries(${OUTPUTNAME} gtest)
target_link_libraries(${OUTPUTNAME} gtest_main)
target_link_libraries(${OUTPUTNAME} pthread)
