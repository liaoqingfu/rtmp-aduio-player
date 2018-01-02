#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include_directories(${PROJECT_SOURCE_DIR}/audio)
include_directories(${PROJECT_SOURCE_DIR}/util)
include_directories(${PROJECT_SOURCE_DIR}/third_lib/faad/include)

aux_source_directory(. DIR_WORLD_SRCS)
add_library(audio STATIC ${DIR_WORLD_SRCS})




