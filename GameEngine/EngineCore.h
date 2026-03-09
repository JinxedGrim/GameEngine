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
	// move all to TPGE
	static bool FpsEngineCounter = true;
	static bool DoMultiThreading = false;
	static bool SimdAcceleration = true;
	static bool GpuAcceleration = false;

	static int CpuCores = 0;
	static int GPUDevCount = 0;
	static std::vector<std::wstring> GPUDevNames = {};
	static std::wstring PrimaryGPUDevName;
	static SIZE_T MaxMemoryMB = 0;
	std::string CpuName;

	static ThreadManager ThreadPool;
	static CPUID CpuId(1);
	static SupportedInstructions SimdInfo = { 0 };

	static bool HasOpenConsole = false;

	// move to TPGE keep all and add a switch for updating keyboard input (allow user to disable all input)
	static bool UpdateMouseIn = true;

	static constexpr const char* TPGE_LOG_ERROR_RENDERER_FATAL = "\\^c";
	static constexpr const char* TPGE_LOG_ERROR_RENDERER_NON_FATAL = "\\^c";

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


	double GetCpuUsageInfo()
	{
		FILETIME creation, exit, kernel, user;
		GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user);

		ULARGE_INTEGER k, u;
		k.LowPart = kernel.dwLowDateTime;
		k.HighPart = kernel.dwHighDateTime;
		u.LowPart = user.dwLowDateTime;
		u.HighPart = user.dwHighDateTime;

		return (k.QuadPart + u.QuadPart) * 100; // FILETIME = 100ns units
	}


	double CalculateCpuUsage(double  cpuTimeDeltaNs, double  wallTimeDeltaNs, int coreCount)
	{
		return (double)cpuTimeDeltaNs / (double)(wallTimeDeltaNs * coreCount) * 100.0;
	}


	SIZE_T GetUsedMemory()
	{
		PROCESS_MEMORY_COUNTERS_EX  Pmc;

		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&Pmc, sizeof(Pmc)))
			return Pmc.PrivateUsage / 1024 / 1024;

		return 0;
	}


	SIZE_T GetUsedHeap()
	{
		_CrtMemState memState = {};
		_CrtMemCheckpoint(&memState);
		return memState.lTotalCount / 1024 / 1024;
	}


	std::wstring GetDevList()
	{
		std::wstring out = L"";
		for (const std::wstring& str : TerraPGE::Core::GPUDevNames)
		{
			out += str;
			out += +L", ";
		}

		return out;
	}


	void GetCpuInfo()
	{
		SYSTEM_INFO SysInf;

		SimdInfo = CpuId.GetSupportedInstructions();
		GetSystemInfo(&SysInf);
		TerraPGE::Core::CpuCores = SysInf.dwNumberOfProcessors;
		TerraPGE::Core::CpuName = CpuId.GetProcessorName();
	}


	void UpdateSystemInfo()
	{
		GetCpuInfo();

		MEMORYSTATUSEX MemInf;
		DISPLAY_DEVICE DispDev;

		DispDev.cb = sizeof(DispDev);
		MemInf.dwLength = sizeof(MEMORYSTATUSEX);
		int DevIdx = 0;

		if (GlobalMemoryStatusEx(&MemInf))
			TerraPGE::Core::MaxMemoryMB = (MemInf.ullTotalPhys / 1024 / 1024);

		if (EnumDisplayDevices(NULL, DevIdx, &DispDev, 0))
		{
			TerraPGE::Core::GPUDevNames.push_back(DispDev.DeviceString);
			TerraPGE::Core::GPUDevCount++;

			if (DispDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
				TerraPGE::Core::PrimaryGPUDevName = DispDev.DeviceString;
		}

	}
}

