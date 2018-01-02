#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
include_directories(${PROJECT_SOURCE_DIR}/audio)
include_directories(${PROJECT_SOURCE_DIR}/player)
include_directories(${PROJECT_SOURCE_DIR}/util)

aux_source_directory(. DIR_WORLD_SRCS)
add_library(rtmp_src STATIC ${DIR_WORLD_SRCS})




