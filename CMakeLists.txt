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
  source/graphics/Framebuffer.cpp
  source/graphics/Framebuffer.hpp
  source/graphics/ParticleBuffer.cpp
  source/graphics/ParticleBuffer.hpp
  source/graphics/Pipeline.cpp
  source/graphics/Pipeline.hpp
  source/graphics/ScreenRectBuffer.cpp
  source/graphics/ScreenRectBuffer.hpp
  source/graphics/Texture.cpp
  source/graphics/Texture.hpp
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
set(EXECUTABLE_NAME "webcam-particles")

add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE source 3rdparty ${SDL2_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${OpenCV_LIBS} ${SDL2_LIBRARY} ${OPENGL_gl_LIBRARY} m)

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:webcam-particles>/resource")

add_custom_target(update-format
  clang-format -i -style=LLVM ${SOURCE_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Format all source files. Requires clang-format to be installed"
)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# CPack packaging
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
include(CPack)
