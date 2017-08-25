cmake_minimum_required(VERSION 3.0)

project(hfg-webcam-particles LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(OpenCV REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

list(APPEND CMAKE_CXX_FLAGS "-std=c++1z -Wall -pthread")

set(SOURCE_FILES
  source/effects/AccumulationEffect.cpp
  source/effects/AccumulationEffect.hpp
  source/effects/ConvergeCircleEffect.cpp
  source/effects/ConvergeCircleEffect.hpp
  source/effects/ConvergePointEffect.cpp
  source/effects/ConvergePointEffect.hpp
  source/effects/EffectRegistry.cpp
  source/effects/EffectRegistry.hpp
  source/effects/HueDisplaceEffect.cpp
  source/effects/HueDisplaceEffect.hpp
  source/effects/SmearEffect.cpp
  source/effects/SmearEffect.hpp
  source/effects/SmoothTrailsEffect.cpp
  source/effects/SmoothTrailsEffect.hpp
  source/effects/TrailsEffect.cpp
  source/effects/TrailsEffect.hpp
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
  source/ImageProvider.cpp
  source/ImageProvider.hpp
  source/ParticleRenderer.cpp
  source/ParticleRenderer.hpp
  source/RendererState.hpp
  source/main.cpp
  source/main.hpp
  source/Resources.cpp
  source/Resources.hpp
  source/SampleLibrary.cpp
  source/SampleLibrary.hpp
  source/ShaderBuilder.cpp
  source/ShaderBuilder.hpp
  source/SoundPlaylist.cpp
  source/SoundPlaylist.hpp
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

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resource")
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${OpenCV_DIR}/haarcascades/haarcascade_frontalface_alt.xml" "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resource")
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy "${OpenCV_DIR}/haarcascades/haarcascade_eye_tree_eyeglasses.xml" "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resource")

if(APPLE)
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES MACOSX_BUNDLE TRUE)
  set(MACOSX_BUNDLE_INFO_STRING "Narzistische Maschine")
  set(MACOSX_BUNDLE_ICON_FILE "app")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "de.hfg-karlsruhe.bielicky.narcistic-machine")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0")
  set(MACOSX_BUNDLE_BUNDLE_NAME "Narcistic Machine")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0")
  set(MACOSX_BUNDLE_COPYRIGHT "2017 Lukas + Lukas")

  file(GLOB_RECURSE RESOURCES "${PROJECT_SOURCE_DIR}/resource/*")
  foreach(FILE ${RESOURCES})
    get_filename_component(LOCATION ${FILE} DIRECTORY)
    string(REPLACE "${PROJECT_SOURCE_DIR}/" "Resources/" LOCATION ${LOCATION})
    set_source_files_properties(${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION ${LOCATION})
  endforeach()
  target_sources(${EXECUTABLE_NAME} PUBLIC ${RESOURCES})

  foreach(LIB ${OpenCV_LIBS} ${SDL2_LIBRARY})
    if(TARGET ${LIB})
      get_target_property(LIBPATH ${LIB} IMPORTED_LOCATION_RELEASE)
      list(APPEND DYLIBS "${LIBPATH}")
    elseif((IS_ABSOLUTE ${LIB}) AND (LIB MATCHES ".*\\.dylib$"))
      list(APPEND DYLIBS "${LIB}")
    endif()
  endforeach()
  foreach(DYLIB ${DYLIBS})
    set_source_files_properties(${DYLIB} PROPERTIES MACOSX_PACKAGE_LOCATION Frameworks)
  endforeach()
  target_sources(${EXECUTABLE_NAME} PUBLIC ${DYLIBS})
endif()

add_custom_target(update-format
  clang-format -i -style=LLVM ${SOURCE_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Format all source files. Requires clang-format to be installed"
)
