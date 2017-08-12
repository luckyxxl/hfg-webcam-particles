#pragma once

#include <algorithm>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <experimental/filesystem>


#include <glm/glm.hpp>

#include <json.hpp>
using json = nlohmann::json;

#include <SDL.h>
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

constexpr auto PI = 3.14159265f;
