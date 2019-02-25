cmake_minimum_required(VERSION 3.5)

set(FFMPEG_LIB_DIR "./ThirdParty/ffmpeg")
set(X264_LIB_DIR "./ThirdParty/x264")

include_directories(${FFMPEG_LIB_DIR})
#link_directories(${FFMPEG_LIB_DIR}/libavcodec ${FFMPEG_LIB_DIR}/libavformat  )

project (transcoder)

file(GLOB SOURCES
    *.c
)


#set(CMAKE_CXX_FLAGS "-g -std=c++0x ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall ")


link_directories( 
    ${X264_LIB_DIR} 
    ${FFMPEG_LIB_DIR}/libavcodec 
    ${FFMPEG_LIB_DIR}/libavfilter  
    ${FFMPEG_LIB_DIR}/libavformat  
    ${FFMPEG_LIB_DIR}/libavdevice  
    ${FFMPEG_LIB_DIR}/libavutil 
    ${FFMPEG_LIB_DIR}/libswscale  
    ${FFMPEG_LIB_DIR}/libswresample   
    ${FFMPEG_LIB_DIR}/libpostproc    
     /usr/lib/x86_64-linux-gnu
     /usr/local/lib
     /usr/lib )

add_executable(${PROJECT_NAME} ${SOURCES})
#set (LIBS -lswscale -lavdevice -lavformat -lavcodec -lavutil)

if(APPLE)
    target_link_libraries(${PROJECT_NAME}
    "-framework AudioToolbox" 
    "-framework VideoToolbox" 
    "-framework MediaToolbox" 
    "-framework CoreMedia" 
    "-framework CoreMediaIO" 
    "-framework CoreVideo" 
    "-framework CoreAudio" 
    "-framework CoreImage" 
    "-framework OpenGL" 
    "-framework AppKit" 
    "-framework QuartzCore"
    "-framework ImageCaptureCore"
    "-framework CoreFoundation"
    "-framework Security"
    "-framework SecurityFoundation"
    "-framework SecurityInterface"
    z  
    iconv 
    bz2 )
endif(APPLE)

if(LINUX)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC dl cuda  bz2 z mp3lame x265 )
endif(LINUX)



target_link_libraries(${PROJECT_NAME}  x264 swscale avdevice avformat avfilter avcodec avutil swresample postproc  pthread  m    )