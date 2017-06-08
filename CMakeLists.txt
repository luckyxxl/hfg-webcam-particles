cmake_minimum_required(VERSION 3.0)

project(hfg-webcam-particles LANGUAGES CXX)

list(APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -pthread")

add_executable(webcam-particles
  source/Application.cpp
  source/Application.hpp
  source/main.cpp
  source/main.hpp
  source/Resources.cpp
  source/Resources.hpp
  source/ThreadSyncTripleBuffer.hpp
  source/Webcam.cpp
  source/Webcam.hpp
)

target_include_directories(webcam-particles PRIVATE /usr/include/SDL2)
target_link_libraries(webcam-particles opencv_highgui opencv_imgproc opencv_core SDL2 GL m)

add_custom_command(TARGET webcam-particles POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:webcam-particles>")
