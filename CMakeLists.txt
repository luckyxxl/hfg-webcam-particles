cmake_minimum_required(VERSION 3.0)

project(hfg-webcam-particles LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

list(APPEND CMAKE_CXX_FLAGS "-std=c++1z -Wall -pthread")

set(SOURCE_FILES
  source/effects/ConvergeCircleEffect.cpp
  source/effects/ConvergeCircleEffect.hpp
  source/effects/ConvergePointEffect.cpp
  source/effects/ConvergePointEffect.hpp
  source/effects/EffectRegistry.cpp
  source/effects/EffectRegistry.hpp
  source/effects/HueDisplaceEffect.cpp
  source/effects/HueDisplaceEffect.hpp
  source/effects/WaveEffect.cpp
  source/effects/WaveEffect.hpp
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
  source/ParticleRenderer.cpp
  source/ParticleRenderer.hpp
  source/RendererState.hpp
  source/main.cpp
  source/main.hpp
  source/Resources.cpp
  source/Resources.hpp
  source/ShaderBuilder.cpp
  source/ShaderBuilder.hpp
  source/Template.hpp
  source/ThreadSyncTripleBuffer.hpp
  source/Timeline.cpp
  source/Timeline.hpp
  source/Webcam.cpp
  source/Webcam.hpp
)

add_executable(webcam-particles ${SOURCE_FILES})

target_include_directories(webcam-particles PRIVATE source 3rdparty ${SDL2_INCLUDE_DIR})
target_link_libraries(webcam-particles ${OpenCV_LIBS} ${SDL2_LIBRARY} ${OPENGL_gl_LIBRARY} m)

add_custom_command(TARGET webcam-particles POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:webcam-particles>/resource")

add_custom_target(update-format
  clang-format -i -style=LLVM ${SOURCE_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Format all source files. Requires clang-format to be installed"
)
