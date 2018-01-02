#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++0x")
include_directories(${PROJECT_SOURCE_DIR}/player)
include_directories(${PROJECT_SOURCE_DIR}/util)
aux_source_directory(. DIR_WORLD_SRCS)
add_library(player STATIC ${DIR_WORLD_SRCS})




