#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
#PROJECT(IntelligentDevice)
#SET(CMAKE_SYSTEM_NAME Linux)

#Head file path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/rtmp)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/util)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/audio)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/player)
#使用boost
SET(Boost_USE_STATIC_LIBS        ON) # only find static libs
SET(Boost_USE_MULTITHREADED      ON)

SET(Boost_LIBRARIES boost_system boost_date_time)

#library path
#LINK_DIRECTORIES(../third_lib/faad/lib)


LINK_DIRECTORIES(/mnt/hgfs/ffmpeg/rtmp/rtmp-aduio-player/third_lib/faad/lib)
#TBD_MGR_COMM_SRC SDK公共源码
#################################################################
#AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} SOURCE_FILES)
#ADD_EXECUTABLE(IntelligentDevice ${SOURCE_FILES})
ADD_EXECUTABLE(rtmp-player  main.c)

TARGET_LINK_LIBRARIES(rtmp-player libmp4ff.a libfaad.a audio rtmp_src util player rtmp pthread )

