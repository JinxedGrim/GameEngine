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

#define FLOAT_LOWEST 0.0000001f

namespace TerraPGE::Core
{
	std::string FpsStr = "Fps: ";
	std::wstring FpsWStr = L"Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	float FOV = 90.0f;
	float FNEAR = 0.1f;
	float FFAR = 150.0f;

	bool FpsEngineCounter = true;
	bool DoMultiThreading = false;
	bool DoCull = true;
	bool DoLighting = true;
	bool WireFrame = false;
	bool ShowTriLines = false;
	bool DebugClip = false;
	bool DoShadows = true;
	bool ShowNormals = false;
	bool NormalMapping = false;

	int CpuCores = 0;
	int GPUDevCount = 0;
	std::vector<std::wstring> GPUDevNames = {};
	std::wstring PrimaryGPUDevName;
	SIZE_T MaxMemoryMB = 0;
	ThreadManager ThreadPool;
#ifdef SSE_SIMD_42_SUPPORT
	CPUID CpuId(1);
	SupportedInstructions SimdInfo = { 0 };
#endif

	bool UpdateMouseIn = true;
	bool LockCursor = true;
	bool CursorShow = false;

	int ShadowMapHeight = 1024;
	int ShadowMapWidth = 1024;

	float* DepthBuffer = DEBUG_NEW float[sx * sy];
	float* ShadowMap = DEBUG_NEW float[ShadowMapWidth * ShadowMapHeight];


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


	void UpdateSystemInfo()
	{
		SYSTEM_INFO SysInf;
		MEMORYSTATUSEX MemInf;
		DISPLAY_DEVICE DispDev;

		DispDev.cb = sizeof(DispDev);
		MemInf.dwLength = sizeof(MEMORYSTATUSEX);
		int DevIdx = 0;

		GetSystemInfo(&SysInf);
		TerraPGE::Core::CpuCores = SysInf.dwNumberOfProcessors;

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