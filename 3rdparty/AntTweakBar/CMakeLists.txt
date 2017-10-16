add_library(AntTweakBar
  src/TwColors.cpp
  src/TwFonts.cpp
  src/TwOpenGL.cpp
  src/TwOpenGLCore.cpp
  src/TwBar.cpp
  src/TwMgr.cpp
  src/TwPrecomp.cpp
  src/LoadOGL.cpp
  src/LoadOGLCore.cpp
  src/TwEventGLFW.c
  src/TwEventGLUT.c
  src/TwEventSDL.c
#  src/TwEventSDL12.c
  src/TwEventSDL13.c
  src/TwEventSFML.cpp
  src/TwEventX11.c
)

target_compile_definitions(AntTweakBar PRIVATE _UNIX __PLACEMENT_NEW_INLINE)
target_include_directories(AntTweakBar PUBLIC include)