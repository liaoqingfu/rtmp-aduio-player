#cmake version request
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#project name
#PROJECT(IntelligentDevice)
#SET(CMAKE_SYSTEM_NAME Linux)

#Head file path
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/src)

#使用boost
SET(Boost_USE_STATIC_LIBS        ON) # only find static libs
SET(Boost_USE_MULTITHREADED      ON)

SET(Boost_LIBRARIES boost_system boost_date_time)

#library path
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/third/lib/alsa)

#TBD_MGR_COMM_SRC SDK公共源码
#################################################################
#AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} SOURCE_FILES)
#ADD_EXECUTABLE(IntelligentDevice ${SOURCE_FILES})
ADD_EXECUTABLE(rtmp-player  main.c)

TARGET_LINK_LIBRARIES(rtmp-player rtmp_src rtmp)

