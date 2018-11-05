
cmake_minimum_required(VERSION 3.4.1)

add_library(
             avcodec-58
             SHARED
             IMPORTED )
set_target_properties( avcodec-58
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libavcodec-58.so )

add_library(
             avdevice-58
             SHARED
             IMPORTED )
set_target_properties( avdevice-58
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libavdevice-58.so )

add_library(
             avfilter-7
             SHARED
             IMPORTED )
set_target_properties( avfilter-7
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libavfilter-7.so )

add_library(
             avformat-58
             SHARED
             IMPORTED )
set_target_properties( avformat-58
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libavformat-58.so )

add_library(
             avutil-56
             SHARED
             IMPORTED )
set_target_properties( avutil-56
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libavutil-56.so )

add_library(
             swresample-3
             SHARED
             IMPORTED )
set_target_properties( swresample-3
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libswresample-3.so )

add_library(
             swscale-5
             SHARED
             IMPORTED )
set_target_properties( swscale-5
                       PROPERTIES IMPORTED_LOCATION
                       ../../../../libs/armeabi-v7a/libswscale-5.so )

find_library(
              log-lib
              log )

find_library(
              android-lib
              android )

add_library(
             ffmpeg-lib
             SHARED
             src/main/cpp/ffmpeg-lib.cpp )

target_link_libraries( ffmpeg-lib
                       avcodec-58 avdevice-58 avfilter-7 avformat-58 avutil-56 swresample-3 swscale-5 ${log-lib} ${android-lib})



include_directories( include )

