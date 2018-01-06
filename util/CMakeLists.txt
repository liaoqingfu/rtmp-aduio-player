#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/rtmp)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/util)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/audio)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/player)
INCLUDE_DIRECTORIES("/usr/local/ffmpeg/include")
link_directories("/usr/local/ffmpeg/lib")

aux_source_directory(. DIR_WORLD_SRCS)
add_library(util STATIC ${DIR_WORLD_SRCS})




