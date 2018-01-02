#cloud terminal
#cmake config file
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (rtmp-player)

#使用C++编译器
SET(CMAKE_C_COMPILER g++)			
SET(CMAK_CXX_COMPILER g++)
#支持C++ 11
add_compile_options(-std=c++11)

# Add header file include directories


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(audio)
ADD_SUBDIRECTORY(rtmp)
ADD_SUBDIRECTORY(util)
ADD_SUBDIRECTORY(player)
