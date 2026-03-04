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

#include "GdiPP.hpp"
#include "WndCreator.hpp"
#include "Math.h"
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"
#include "Lighting.h"
#include "Shading.h"
#include "Input.h"

#include "ParrallelPP.h"

#ifdef SSE_SIMD_42_SUPPORT
#include "Intrinsics/VectorExHelper.h"
#endif

#ifdef GPU_SUPPORT
#include "../../../Source/repos/GlLoader/GlLoader/GlLoader.h"
#endif

namespace TerraPGE::Core
{
	std::string FpsStr = "Fps: ";
	std::wstring FpsWStr = L"Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	float FOV = 90.0f;
	float FNEAR = 0.1f;
	float FFAR = 100.0f;

	// move all to TPGE
	bool FpsEngineCounter = true;
	bool DoMultiThreading = true;
	bool SimdAcceleration = true;
	bool GpuAcceleration = false;

	int CpuCores = 0;
	int GPUDevCount = 0;
	std::vector<std::wstring> GPUDevNames = {};
	std::wstring PrimaryGPUDevName;
	SIZE_T MaxMemoryMB = 0;

	ThreadManager ThreadPool;
	CPUID CpuId(1);
	SupportedInstructions SimdInfo = { 0 };

	// move to TPGE keep all and add a switch for updating keyboard input (allow user to disable all input)
	bool UpdateMouseIn = true;
	bool LockCursor = true;
	bool CursorShow = false;

	// move to lighting obj
	int ShadowMapHeight = 1024;
	int ShadowMapWidth = 1024;

	// move to renderer
	float* DepthBuffer = DEBUG_NEW float[sx * sy];
	float* FrameBuffer = DEBUG_NEW float[sx * sy * 3];

	// move out and into ligting objects
	float* ShadowMap = DEBUG_NEW float[ShadowMapWidth * ShadowMapHeight];

	void Log(std::string Message)
	{
		std::cout << Message << std::endl;
	}


	void LogInfo(std::string CallerTag, std::string Message)
	{
		Core::Log("[I] (" + CallerTag + ") " + Message);
	}
	

	void LogWarning(std::string CallerTag, std::string Message)
	{
		Core::Log("[W] (" + CallerTag + ") " + Message);
	}


	void LogError(std::string CallerTag, std::string Message, int Level)
	{
		Core::Log("[E] (" + CallerTag + ") " + Message);
	}


	uint64_t GetCpuUsageInfo()
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


	double CalculateCpuUsage(uint64_t cpuTimeDeltaNs, uint64_t wallTimeDeltaNs, int coreCount)
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


	void OpenConsole()
	{
		AllocConsole();

		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		freopen_s(&fp, "CONIN$", "r", stdin);
	}


	// Update screen info
	void UpdateScreenInfo(GdiPP* Gdi)
	{
		// Update GDI info
		Gdi->UpdateClientRgn();

		// Update screen dimenstions
		sx = Gdi->ClientRect.right - Gdi->ClientRect.left;
		sy = Gdi->ClientRect.bottom - Gdi->ClientRect.top;

		// Update DepthBuffer
		delete[] DepthBuffer;
		DepthBuffer = DEBUG_NEW float[sx * sy];
	}


	// Delete buffers
	void EngineCleanup()
	{
		Log("EngineCleanup called");
		// Cleanup buffers
		delete[] DepthBuffer;
		delete[] ShadowMap;
	}


	void UpdateInput(WndCreator& Wnd)
	{
		if (Wnd.HasFocus())
		{
			static POINT Tmp;

			if (LockCursor)
				SetCursorPos(sx / 2, sy / 2);

			if (CursorShow == false)
			{
				while (ShowCursor(FALSE) >= 0) {}
			}
			else
			{
				while (ShowCursor(TRUE) <= 0) {}
			}
		}

	}


	__inline void SetPixelFrameBuffer(int x, int y, float R, float G, float B)
	{
		int index = ContIdx(x, y, Core::sx) * 3;
		Core::FrameBuffer[index++] = R;
		Core::FrameBuffer[index++] = G;
		Core::FrameBuffer[index] = B;
	}


	void UpdateWindow(WndCreator& Wnd, MSG* msg)
	{
		Wnd.Input.BeginFrame();

		while (PeekMessageW(msg, NULL, 0, 0, PM_REMOVE))
		{
			// Check Msg
			if (msg->message == WM_QUIT || msg->message == WM_CLOSE || msg->message == WM_DESTROY)
			{
				break;
			}
			else
			{
				TranslateMessage(msg);
				DispatchMessageW(msg);
			}
		}

		Core::UpdateInput(Wnd);
	}
}

