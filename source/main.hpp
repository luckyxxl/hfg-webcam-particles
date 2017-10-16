#pragma once

#include <algorithm>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <random>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

#include <json.hpp>
using json = nlohmann::json;

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

#include <opencv2/imgproc/imgproc.hpp>
#if CV_VERSION_EPOCH < 3
#include <opencv2/highgui/highgui.hpp>
#else
#include <opencv2/videoio.hpp>
#endif
#include <opencv2/objdetect/objdetect.hpp>

#if WITH_EDIT_TOOLS
#include <AntTweakBar.h>
#endif

constexpr auto PI = 3.14159265f;
