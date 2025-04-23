#pragma once
#define NOMINMAX
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <functional>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <filesystem>

#include "Math.h"
#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"
#include "Graphics/TerraGL.h"
#include "ParrallelPP.h"

#define FLOAT_LOWEST 0.0000001f

#ifdef SSE_SIMD_42_SUPPORT
#include "Intrinsics/VectorExHelper.h"
#endif

// The entire premise of this project is to build a game engine without using any libraries, but the std library and the WinAPI. 
// GdiPP is more or less straight WinAPI, but you do have to link with a dll which is sort of cheating but it's about the only way to do graphics without any libs like DX or OpenGl
// Direct2D and DirectWrite are justified the same way

// Windows only include
#include <Psapi.h>

//     TO DO 
// 3. Revamp Material and texture system ie put all materials in the mesh and pointers to them in the triangle
// 4. Caclulate Vertex norms for all 3 vertices and store them
// 5. figure out a better way of per pixel shading
// 6. redisign engine into a more API-like manner
// X. Audio system 
// X. Voice 
// X. More light types
// X. Ambient occlusion 
// X. Global illumination
// X. FXAA, TSAA, Etc
// X. Proper UI system
// X. Probably migrate to unicode

