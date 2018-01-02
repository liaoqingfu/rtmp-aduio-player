#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include_directories(./  ../player)

aux_source_directory(. DIR_WORLD_SRCS)
add_library( rtmp_src ${DIR_WORLD_SRCS})




