cmake_minimum_required(VERSION 3.5)

set(FFMPEG_LIB_DIR "./ThirdParty/ffmpeg")
include_directories(${FFMPEG_LIB_DIR})
link_directories(${FFMPEG_LIB_DIR})

project (transcoder)

file(GLOB SOURCES
    *.c
)


#set(CMAKE_CXX_FLAGS "-g -std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -std=c99")

set (FFMPEG_LIBRARIES_DIRS
    $(FFMPEG_LIB_DIR)/libswscale 
    $(FFMPEG_LIB_DIR)/libavdevice
    $(FFMPEG_LIB_DIR)/libavutil 
    $(FFMPEG_LIB_DIR)/libavformat
    $(FFMPEG_LIB_DIR)/libavcodec )

link_directories( ./ThirdParty/ffmpeg/libavcodec.a  ./ThirdParty/ffmpeg/libavformat  ./ThirdParty/ffmpeg/libavdevice  ./ThirdParty/ffmpeg/libavutil ./ThirdParty/ffmpeg/libswscale   $(FFMPEG_LIBRARIES_DIRS) /usr/local/lib )

add_executable(${PROJECT_NAME} ${SOURCES})
set (LIBS -lswscale -lavdevice -lavformat -lavcodec -lavutil)
target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES}  pthread  m z )