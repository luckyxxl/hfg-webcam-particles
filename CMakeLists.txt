cmake_minimum_required(VERSION 3.0)

project(hfg-webcam-particles LANGUAGES CXX)

find_package(OpenCV REQUIRED)

list(APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -pthread")

add_executable(webcam-particles
  source/effects/ConvergeCircleEffect.cpp
  source/effects/ConvergeCircleEffect.hpp
  source/effects/EffectRegistry.cpp
  source/effects/EffectRegistry.hpp
  source/effects/HueDisplaceEffect.cpp
  source/effects/HueDisplaceEffect.hpp
  source/graphics/ParticleBuffer.cpp
  source/graphics/ParticleBuffer.hpp
  source/graphics/Pipeline.cpp
  source/graphics/Pipeline.hpp
  source/graphics/Window.cpp
  source/graphics/Window.hpp
  source/sound/Renderer.hpp
  source/sound/Renderer.cpp
  source/sound/SampleBuffer.hpp
  source/sound/SampleBuffer.cpp
  source/Application.cpp
  source/Application.hpp
  source/main.cpp
  source/main.hpp
  source/Resources.cpp
  source/Resources.hpp
  source/ShaderBuilder.cpp
  source/ShaderBuilder.hpp
  source/ThreadSyncTripleBuffer.hpp
  source/Webcam.cpp
  source/Webcam.hpp
)

target_include_directories(webcam-particles PRIVATE source /usr/include/SDL2)
target_link_libraries(webcam-particles ${OpenCV_LIBS} SDL2 GL m)

add_custom_command(TARGET webcam-particles POST_BUILD COMMAND ln -sf "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:webcam-particles>")
