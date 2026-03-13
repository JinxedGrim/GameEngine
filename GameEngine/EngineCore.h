#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <functional>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <filesystem>

// Windows only include
#include <Windows.h>
#include <Psapi.h>

#include "Math.h"

#ifdef SSE_SIMD_42_SUPPORT
#endif

#ifdef GPU_SUPPORT
#include "../../../Source/repos/GlLoader/GlLoader/GlLoader.h"
#endif

namespace TerraPGE::Core
{
	CHAR cwd[MAX_PATH + 1] = "";
	DWORD len = GetCurrentDirectoryA(MAX_PATH, cwd);

	std::string CWD = cwd;

	// move all to TPGE
	static bool FpsEngineCounter = true;
	static bool DoMultiThreading = true;
	static bool SimdAcceleration = true;
	static bool GpuAcceleration = false;

	static ThreadManager ThreadPool;

	static bool HasOpenConsole = false;

	// move to TPGE keep all and add a switch for updating keyboard input (allow user to disable all input)
	static bool UpdateMouseIn = true;

	static constexpr int TPGE_LOG_ERROR_RENDERER_NON_FATAL = 0;
	static constexpr int TPGE_LOG_ERROR_RENDERER_FATAL = 1;

	__inline void _LogCout(std::string Message)
	{
		std::cout << Message << std::endl;
	}

	__inline void _LogCErr(std::string Message)
	{
		std::cout << Message << std::endl;
	}

	__inline void Log(std::string Message, bool IsError = false)
	{
		if (HasOpenConsole)
		{
			_LogCout(Message);
		}
	}

	__inline void LogInfo(std::string CallerTag, std::string Message)
	{
		Core::Log("[I] (" + CallerTag + ") " + Message);
	}
	

	__inline void LogWarning(std::string CallerTag, std::string Message)
	{
		Core::Log("[W] (" + CallerTag + ") " + Message);
	}


	__inline void LogError(std::string CallerTag, std::string Message, int Level)
	{
		Core::Log("[E] (" + CallerTag + ") " + Message);
	}
}

