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

#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"
#include "Graphics/TerraGL.h"
#include "ParrallelPP.h"
#include "Shading.h"

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
// 1. Fix the shadows during culling 
// 3. Caclulate Vertex norms for all 3 vertices and store them
// 4. figure out a better way of per pixel shading (CUDA?)
// 5. redisign engine into a more API-like manner
// X. Audio system 
// X. Voice 
// X. More light types
// X. Ambient occlusion 
// X. Global illumination
// X. FXAA, TSAA, Etc
// X. Proper UI system
// X. Probably migrate to unicode

constexpr auto VK_W = 0x57;
constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;
constexpr auto VK_D = 0x44;

class Renderable
{
	public:
	Renderable() = delete;

	Renderable(const Mesh mesh, const Camera& Cam, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, const std::function<void(ShaderArgs&)> Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		this->Cam = Cam;
		this->Pos = Pos;
		this->mesh = mesh;
		this->Scalar = Scalar;
		this->RotationRads = RotationRads;
		this->Shader = Shader;
		this->SHADER_TYPE = SHADER_TYPE;
	}

	Mesh mesh = Mesh();
	Vec3 Pos = Vec3();
	Vec3 Scalar = Vec3();
	Vec3 RotationRads = Vec3();
	Camera Cam;

	std::function<void(ShaderArgs&)> Shader = EngineShaders::Shader_Frag_Phong_Shadows;
	ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT;
};

typedef void(__fastcall* DoTick_T)(const GdiPP&, const WndCreator&, const float&, std::vector<Renderable>*, std::vector<LightObject*>*);

namespace TerraPGE
{
	std::string FpsStr = "Fps: ";
	std::wstring FpsWStr = L"Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	float FOV = 90.0f;
	float FNEAR = 0.1f;
	float FFAR = 150.0f;
	float ShadowMapBias = FLOAT_LOWEST;
	int ShadowMapHeight = 1024;
	int ShadowMapWidth = 1024;

	bool FpsEngineCounter = true;
	bool DoMultiThreading = false;
	bool DoCull = true;
	bool DoLighting = true;
	bool DoShadows = true;
	bool WireFrame = false;
	bool ShowTriLines = false;
	bool DebugClip = false;
	bool LockCursor = true;
	bool CursorShow = false;
	bool UpdateMouseIn = true;
	bool ShowNormals = false;
	bool DebugDepthBuffer = false;
	bool DebugShadowMap = false;
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

	Vec2 PrevMousePos;
	Vec2 DeltaMouse;
	POINT Tmp;
	float Sensitivity = 0.1f;
	int Fps = 0;

	float* DepthBuffer = new float[sx * sy];
	float* ShadowMap = new float[ShadowMapWidth * ShadowMapHeight];

	SIZE_T GetUsedMemory()
	{
		PROCESS_MEMORY_COUNTERS_EX  Pmc;

		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&Pmc, sizeof(Pmc)))
			return Pmc.PrivateUsage / 1024 / 1024;

		return 0;
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
		TerraPGE::CpuCores = SysInf.dwNumberOfProcessors;

		if (GlobalMemoryStatusEx(&MemInf))
			TerraPGE::MaxMemoryMB = (MemInf.ullTotalPhys / 1024 / 1024);

		if (EnumDisplayDevices(NULL, DevIdx, &DispDev, 0))
		{
			TerraPGE::GPUDevNames.push_back(DispDev.DeviceString);
			TerraPGE::GPUDevCount++;

			if (DispDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
				TerraPGE::PrimaryGPUDevName = DispDev.DeviceString;
		}

	}

	// Update screen info
	void UpdateScreenInfo(GdiPP& Gdi)
	{
		// Update GDI info
		Gdi.UpdateClientRgn();

		// Update screen dimenstions
		sx = Gdi.ClientRect.right - Gdi.ClientRect.left;
		sy = Gdi.ClientRect.bottom - Gdi.ClientRect.top;

		// Update DepthBuffer
		delete[] DepthBuffer;
		DepthBuffer = new float[sx * sy];
	}

	// Delete buffers
	void EngineCleanup()
	{
		// Cleanup buffers
		delete[] DepthBuffer;
		delete[] ShadowMap;
	}

	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	template<typename T>
	void __fastcall RenderTriangleThreaded(float* DepthBuffer, GdiPP& Gdi, T&& Shader, ShaderArgs Args)
	{
		// Setup variables
		Triangle* Tri = Args.Tri;
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;
		Matrix Vp = (Args.ViewMat * Args.ProjectionMat);

		int x1 = PixelRound(Tri->Points[0].x);
		int y1 = PixelRound(Tri->Points[0].y);
		int x2 = PixelRound(Tri->Points[1].x);
		int y2 = PixelRound(Tri->Points[1].y);
		int x3 = PixelRound(Tri->Points[2].x);
		int y3 = PixelRound(Tri->Points[2].y);
		float u1 = 0.0f;
		float u2 = 0.0f;
		float u3 = 0.0f;
		float w1 = 0.0f;
		float w2 = 0.0f;
		float w3 = 0.0f;
		float v1 = 0.0f;
		float v2 = 0.0f;
		float v3 = 0.0f;

		if (Tri->Material->HasUsableTexture())
		{
			u1 = Tri->TexCoords[0].u;
			u2 = Tri->TexCoords[1].u;
			u3 = Tri->TexCoords[2].u;
			v1 = Tri->TexCoords[0].v;
			v2 = Tri->TexCoords[1].v;
			v3 = Tri->TexCoords[2].v;
			w1 = Tri->TexCoords[0].w;
			w2 = Tri->TexCoords[1].w;
			w3 = Tri->TexCoords[2].w;
		}

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);
		if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
				{
					for (int j = ax; j < bx; j++)
					{
						tex_u = (1.0f - t) * tex_su + t * tex_eu;
						tex_v = (1.0f - t) * tex_sv + t * tex_ev;
						tex_w = (1.0f - t) * tex_sw + t * tex_ew;
						t += tstep;

						int idx = ContIdx(j, i, TerraPGE::sx);

						// Calculate the barycentric coordinates which we use for interpolation
						Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
						// Use Barycentric coords to interpolate our world position
						Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

						// World Pos -> Clipped Space
						Vec4 NdcPos = InterpolatedPos * Vp;
						// Clipped -> Ndc
						NdcPos.CorrectPerspective();

						// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
						// this line also normalizes that value to [0, 1]
						float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						// Depth test
						if (Depth < DepthBuffer[idx])
							DepthBuffer[idx] = Depth;
						else
							continue; // Don't draw pixel

						float ShadowValue = 0.0f;
						float ShadowDepth = 0.0f;
						int MapIdx = 0;

						if (DoShadows)
						{
							// Make this better TODO
							Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();

							ShadowNdcPos.CorrectPerspective();

							ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
							ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

							MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

							ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
							ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							if (ShadowDepth > ShadowValue)
							{
								Args.IsInShadow = true;
							}
							else
							{
								Args.IsInShadow = false;
							}
						}

						// Debug depth buffer (Grayscale the pixel * depth val)
						if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
						{
							// I plan to add a way to visualize the shadow map here
							float Val = Depth;
							if (TerraPGE::DebugShadowMap)
							{
								if (ShadowDepth > ShadowValue)
								{
									Val = 0.3f;
								}
								else
								{
									Val = TerraPGE::ShadowMap[MapIdx];
								}
							}

							Args.FragColor.R = 255.0f * Val;
							Args.FragColor.G = 255.0f * Val;
							Args.FragColor.B = 255.0f * Val;
						}
						else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !TerraPGE::DoLighting)
						{
							// This entire else if is mainly for debugging clipping
							if (Tri->OverrideTextureColor)
							{
								Args.FragColor.R = Tri->Col.x;
								Args.FragColor.G = Tri->Col.y;
								Args.FragColor.B = Tri->Col.z;
							}
							else if (Tri->Material->HasUsableTexture())
							{
								Vec3 TexturCol = Args.Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
								Args.FragColor.R = TexturCol.x;
								Args.FragColor.G = TexturCol.y;
								Args.FragColor.B = TexturCol.z;
							}
							else
							{
								Args.FragColor.R = Tri->Material->AmbientColor.x;
								Args.FragColor.G = Tri->Material->AmbientColor.y;
								Args.FragColor.B = Tri->Material->AmbientColor.z;
							}
						}
						else
						{
							Vec3 InterpolatedNormal = ((Tri->FaceNormal * BaryCoords.x) + (Tri->FaceNormal * BaryCoords.y) + (Tri->FaceNormal * BaryCoords.z)).Normalized();

							// Set up some shader args and call fragment shader=
							Args.FragPos = InterpolatedPos;
							Args.FragNormal = InterpolatedNormal;
							Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
							Args.BaryCoords = BaryCoords;
							Args.PixelCoords = Vec2((float)j, (float)i);
							Shader(Args);
						}

						// Set pixel in pixel buffer
						COLORREF PixelClr = RGB(PixelRoundFloor(Args.FragColor.R), PixelRoundFloor(Args.FragColor.G), PixelRoundFloor(Args.FragColor.B));
						Gdi.QuickSetPixel(j, i, PixelClr);
					}
				};
				TerraPGE::ThreadPool.EnqueueTask(Func);
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
				{
					for (int j = ax; j < bx; j++)
					{
						tex_u = (1.0f - t) * tex_su + t * tex_eu;
						tex_v = (1.0f - t) * tex_sv + t * tex_ev;
						tex_w = (1.0f - t) * tex_sw + t * tex_ew;
						t += tstep;

						int idx = ContIdx(j, i, TerraPGE::sx);

						// Calculate the barycentric coordinates which we use for interpolation
						Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
						// Use Barycentric coords to interpolate our world position
						Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

						// World Pos -> Clipped Space
						Vec4 NdcPos = InterpolatedPos * Vp;
						// Clipped -> Ndc
						NdcPos.CorrectPerspective();

						// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
						// this line also normalizes that value to [0, 1]
						float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						// Depth test
						if (Depth < DepthBuffer[idx])
							DepthBuffer[idx] = Depth;
						else
							continue; // Don't draw pixel

						float ShadowValue = 0.0f;
						float ShadowDepth = 0.0f;
						int MapIdx = 0;

						if (DoShadows)
						{
							Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
							ShadowNdcPos.CorrectPerspective();

							ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
							ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

							MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

							ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
							ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							if (ShadowDepth > ShadowValue)
							{
								Args.IsInShadow = true;
							}
							else
							{
								Args.IsInShadow = false;
							}
						}

						// Debug depth buffer (Grayscale the pixel * depth val)
						if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
						{
							// I plan to add a way to visualize the shadow map here
							float Val = Depth;
							if (TerraPGE::DebugShadowMap)
							{
								if (ShadowDepth > ShadowValue)
								{
									Val = 0.3f;
								}
								else
								{
									Val = TerraPGE::ShadowMap[MapIdx];
								}
							}

							Args.FragColor.R = 255.0f * Val;
							Args.FragColor.G = 255.0f * Val;
							Args.FragColor.B = 255.0f * Val;
						}
						else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !TerraPGE::DoLighting)
						{
							// This entire else if is mainly for debugging clipping
							if (Tri->OverrideTextureColor)
							{
								Args.FragColor.R = Tri->Col.x;
								Args.FragColor.G = Tri->Col.y;
								Args.FragColor.B = Tri->Col.z;
							}
							else if (Tri->Material->HasUsableTexture())
							{
								Vec3 TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
								Args.FragColor.R = TexturCol.x;
								Args.FragColor.G = TexturCol.y;
								Args.FragColor.B = TexturCol.z;
							}
							else
							{
								Args.FragColor.R = Tri->Material->AmbientColor.x;
								Args.FragColor.G = Tri->Material->AmbientColor.y;
								Args.FragColor.B = Tri->Material->AmbientColor.z;
							}
						}
						else
						{
							Vec3 InterpolatedNormal = ((Tri->FaceNormal * BaryCoords.x) + (Tri->FaceNormal * BaryCoords.y) + (Tri->FaceNormal * BaryCoords.z)).Normalized();

							// Set up some shader args and call fragment shader=
							Args.FragPos = InterpolatedPos;
							Args.FragNormal = InterpolatedNormal;
							Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
							Args.BaryCoords = BaryCoords;
							Args.PixelCoords = Vec2((float)j, (float)i);
							Shader(Args);
						}

						// Set pixel in pixel buffer
						COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
						Gdi.QuickSetPixel(j, i, PixelClr);
					}
				};

				TerraPGE::ThreadPool.EnqueueTask(Func);
			}
		}

		TerraPGE::ThreadPool.WaitUntilAllTasksFinished();
	}

	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	template<typename T>
	void __fastcall RenderTriangle(float* DepthBuffer, GdiPP& Gdi, T&& Shader, ShaderArgs Args)
	{
		// Setup variables
		Triangle& Tri = *Args.Tri;
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;
		Matrix Vp = (Args.ViewMat * Args.ProjectionMat);

		int x1 = PixelRound(Tri.Points[0].x);
		int y1 = PixelRound(Tri.Points[0].y);
		int x2 = PixelRound(Tri.Points[1].x);
		int y2 = PixelRound(Tri.Points[1].y);
		int x3 = PixelRound(Tri.Points[2].x);
		int y3 = PixelRound(Tri.Points[2].y);
		float u1 = 0.0f;
		float u2 = 0.0f;
		float u3 = 0.0f;
		float v1 = 0.0f;
		float v2 = 0.0f;
		float v3 = 0.0f;
		float w1 = 0.0f;
		float w2 = 0.0f;
		float w3 = 0.0f;

		if (Tri.Material->HasUsableTexture())
		{
			u1 = Tri.TexCoords[0].u;
			u2 = Tri.TexCoords[1].u;
			u3 = Tri.TexCoords[2].u;
			v1 = Tri.TexCoords[0].v;
			v2 = Tri.TexCoords[1].v;
			v3 = Tri.TexCoords[2].v;
			w1 = Tri.TexCoords[0].w;
			w2 = Tri.TexCoords[1].w;
			w3 = Tri.TexCoords[2].w;
		}

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);
		if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					tex_w = (1.0f - t) * tex_sw + t * tex_ew;
					t += tstep;

					int idx = ContIdx(j, i, TerraPGE::sx);

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

					// World Pos -> Clipped Space
					Vec4 NdcPos = InterpolatedPos * Vp;
					// Clipped -> Ndc
					NdcPos.CorrectPerspective();

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					// Depth test
					if (Depth < DepthBuffer[idx])
						DepthBuffer[idx] = Depth;
					else
						continue; // Don't draw pixel

					float ShadowValue = 0.0f;
					float ShadowDepth = 0.0f;
					int MapIdx = 0;

					if (DoShadows)
					{
						Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
						ShadowNdcPos.CorrectPerspective();

						ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
						ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

						ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
						ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						if (ShadowDepth > ShadowValue)
						{
							Args.IsInShadow = true;
						}
						else
						{
							Args.IsInShadow = false;
						}
					}

					// Debug depth buffer (Grayscale the pixel * depth val)
					if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
					{
						// I plan to add a way to visualize the shadow map here
						float Val = Depth;
						if (TerraPGE::DebugShadowMap)
						{
							if (ShadowDepth > ShadowValue)
							{
								Val = 0.3f;
							}
							else
							{
								Val = TerraPGE::ShadowMap[MapIdx];
							}
						}

						Args.FragColor.R = 255.0f * Val;
						Args.FragColor.G = 255.0f * Val;
						Args.FragColor.B = 255.0f * Val;
					}
					else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
					{
						// This entire else if is mainly for debugging clipping
						if (Tri.OverrideTextureColor)
						{
							Args.FragColor.R = Tri.Col.x;
							Args.FragColor.G = Tri.Col.y;
							Args.FragColor.B = Tri.Col.z;
						}
						else if (Tri.Material->HasUsableTexture())
						{
							Vec3 TexturCol = Args.Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
							Args.FragColor.R = TexturCol.x;
							Args.FragColor.G = TexturCol.y;
							Args.FragColor.B = TexturCol.z;
						}
						else
						{
							Args.FragColor.R = Tri.Material->AmbientColor.x;
							Args.FragColor.G = Tri.Material->AmbientColor.y;
							Args.FragColor.B = Tri.Material->AmbientColor.z;
						}
					}
					else
					{
						Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

						// Set up some shader args and call fragment shader=
						Args.FragPos = InterpolatedPos;
						Args.FragNormal = InterpolatedNormal;
						Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
						Args.BaryCoords = BaryCoords;
						Args.PixelCoords = Vec2((float)j, (float)i);
						Shader(Args);
					}

					// Set pixel in pixel buffer
					COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
					Gdi.QuickSetPixel(j, i, PixelClr);
				}
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					tex_w = (1.0f - t) * tex_sw + t * tex_ew;
					t += tstep;

					int idx = ContIdx(j, i, TerraPGE::sx);

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

					// World Pos -> Clipped Space
					Vec4 NdcPos = InterpolatedPos * Vp;
					// Clipped -> Ndc
					NdcPos.CorrectPerspective();

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					// Depth test
					if (Depth < DepthBuffer[idx])
						DepthBuffer[idx] = Depth;
					else
						continue;

					float ShadowValue = 0.0f;
					float ShadowDepth = 0.0f;
					int MapIdx = 0;

					if (DoShadows)
					{
						Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
						ShadowNdcPos.CorrectPerspective();

						ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
						ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

						ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
						ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						if (ShadowDepth > ShadowValue)
						{
							Args.IsInShadow = true;
						}
						else
						{
							Args.IsInShadow = false;
						}
					}

					// Debug depth buffer (Grayscale the pixel * depth val)
					if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
					{
						// I plan to add a way to visualize the shadow map here
						float Val = Depth;
						if (TerraPGE::DebugShadowMap)
						{
							if (ShadowDepth > ShadowValue)
							{
								Val = 0.3f;
							}
							else
							{
								Val = TerraPGE::ShadowMap[MapIdx];
							}
						}

						Args.FragColor.R = 255.0f * Val;
						Args.FragColor.G = 255.0f * Val;
						Args.FragColor.B = 255.0f * Val;
					}
					else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
					{
						// This entire else if is mainly for debugging clipping
						if (Tri.OverrideTextureColor)
						{
							Args.FragColor.R = Tri.Col.x;
							Args.FragColor.G = Tri.Col.y;
							Args.FragColor.B = Tri.Col.z;
						}
						else if (Tri.Material->HasUsableTexture())
						{
							Vec3 TexturCol = Args.Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
							Args.FragColor.R = TexturCol.x;
							Args.FragColor.G = TexturCol.y;
							Args.FragColor.B = TexturCol.z;
						}
						else
						{
							Args.FragColor.R = Tri.Material->AmbientColor.x;
							Args.FragColor.G = Tri.Material->AmbientColor.y;
							Args.FragColor.B = Tri.Material->AmbientColor.z;
						}
					}
					else
					{
						Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

						// Set up some shader args and call fragment shader
						Args.FragPos = InterpolatedPos;
						Args.FragNormal = InterpolatedNormal;
						Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
						Args.BaryCoords = BaryCoords;
						Args.PixelCoords = Vec2((float)j, (float)i);
						Shader(Args);
					}

					// Set pixel in pixel buffer
					COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
					Gdi.QuickSetPixel(j, i, PixelClr);
				}
			}
		}
	}

	// Render process but writes to buffer only
	void __fastcall RenderTriangleDepthOnly(Triangle* Tri, float* Buffer, SIZE_T BufferWidth, SIZE_T BufferHeight, const Matrix& ViewProjMat)
	{
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;

		int x1 = PixelRound(Tri->Points[0].x);
		int y1 = PixelRound(Tri->Points[0].y);
		int x2 = PixelRound(Tri->Points[1].x);
		int y2 = PixelRound(Tri->Points[1].y);
		int x3 = PixelRound(Tri->Points[2].x);
		int y3 = PixelRound(Tri->Points[2].y);

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;

		float dax_step = 0, dbx_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				if (ax > bx)
				{
					std::swap(ax, bx);
				}

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					t += tstep;

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);
					// Calculate our clip space position (The perspective divide is done by the * operator making these NDC coords)
					Vec4 NdcPos = InterpolatedPos * (ViewProjMat);
					NdcPos.CorrectPerspective();

					SIZE_T Idx = ContIdx(j, i, BufferWidth);

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					if (Depth < Buffer[Idx])
						Buffer[Idx] = Depth;
					else
						continue;
				}

			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				if (ax > bx)
				{
					std::swap(ax, bx);
				}

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					t += tstep;

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);
					// Calculate our clip space position (The perspective divide is done by the * operator making these NDC coords)
					Vec4 NdcPos = InterpolatedPos * (ViewProjMat);
					NdcPos.CorrectPerspective();

					SIZE_T Idx = ContIdx(j, i, BufferWidth);

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					if (Depth < Buffer[Idx])
						Buffer[Idx] = Depth;
					else
						continue;
				}
			}
		}
	}

	// Render function for rendering entire meshes
	//Rendering Pipeline: Recieve call with mesh info including positions & lights sources -> Calc and apply matrices + normals -> backface culling	
	// -> Clipping + Frustum culling -> Call Draw Triangle from graphics API with shader supplied -> Shader called from triangle routine with pixel info + normals -> SetPixel
	template<typename T>
	void RenderMesh(GdiPP& Gdi, Camera& Cam, const Mesh& MeshToRender, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, LightObject** SceneLights, size_t LightCount, T&& Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
		const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		// Normal Matrix (it's weird)
		Matrix NormalMat = ObjectMatrix.QuickInversed();
		NormalMat.Transpose3x3();

		std::vector<Triangle> TrisToRender = {};
		TrisToRender.reserve(MeshToRender.Triangles.size());

		Triangle Clipped[2];

		Vec3 NormPos = Vec3(0, 0, 0);
		Vec3 NormDir = Vec3(0, 0, 0);


		for (int i = 0; i < LightCount; i++)
		{
			LightObject* Light = SceneLights[i];

			Light->CalcVpMat();
		}

		//TODO MultiThread??
		// TODO Allow vertex shaders (:
		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space / World Space
			Triangle WorldSpaceTri = Tri;
			WorldSpaceTri.ApplyMatrix(ObjectMatrix);

			for (int i = 0; i < 3; i++)
			{
				WorldSpaceTri.WorldSpaceVerts[i] = WorldSpaceTri.Points[i];
			}

			//TODO Fix this whole damn thing calc normals at mesh gen or at vertex shade time fuck the rest
			Vec3 TriNormal = Tri.FaceNormal;
			if (MeshToRender.Normals.size() == 0)
			{
				NormPos = ((Tri.Points[0] + Tri.Points[1] + Tri.Points[2]) / 3.0f);
				NormDir = (Tri.Points[1] - Tri.Points[0]).GetVec3().CrossNormalized((Tri.Points[2] - Tri.Points[0])).Normalized();

				TriNormal = (WorldSpaceTri.Points[1] - WorldSpaceTri.Points[0]).GetVec3().CrossNormalized((WorldSpaceTri.Points[2] - WorldSpaceTri.Points[0])).Normalized(); // this line and the if statement is used for culling

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= NormalMat;
				}

				WorldSpaceTri.NormalPositions[0] = NormPos;
				WorldSpaceTri.NormalPositions[1] = Tri.Points[0];
				WorldSpaceTri.NormalPositions[2] = Tri.Points[1];
				WorldSpaceTri.NormalPositions[3] = Tri.Points[2];
				WorldSpaceTri.NormDirections[0] = NormDir;
			}
			else
			{
				TriNormal *= NormalMat;
				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= NormalMat;
				}

				NormDir = WorldSpaceTri.FaceNormal;
				NormPos = WorldSpaceTri.NormalPositions[0];
			}

			WorldSpaceTri.FaceNormal = TriNormal;

			if ((TriNormal.Dot(WorldSpaceTri.Points[0] - Cam.Pos) < 0.0f) || !TerraPGE::DoCull || !MeshToRender.BackfaceCulling) // backface culling
			{
				// 3d Space -> Viewed Space
				WorldSpaceTri.ApplyMatrix(Cam.ViewMatrix);

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.ViewSpaceVerts[i] = WorldSpaceTri.Points[i];
				}

				Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };
				int Count = WorldSpaceTri.ClipAgainstPlane(Cam.NearPlane, PlaneNormal, Clipped[0], Clipped[1], DebugClip);

				if (Count == 0)
					continue;

				for (int i = 0; i < Count; i++)
				{
					Triangle Projected = Clipped[i];

					// Viewed Space -> clip space
					Projected = Cam.ProjectTriangle(&Projected);

					// Clip Space -> NDC Space
					Projected.ApplyPerspectiveDivide();


					// Offset to viewport space (Ndc -> Screen Space)
					Projected.Scale(Vec3((float)(TerraPGE::sx * 0.5f), (float)(TerraPGE::sy * 0.5f), 1.0f));
					Projected.Translate(Vec3((float)(TerraPGE::sx * 0.5f), (float)(TerraPGE::sy * 0.5f), 0.0f));

					// Add Triangle to render list
					TrisToRender.push_back(Projected);
				}
			}
		}

		// sort faces 
//		std::sort(TrisToRender.begin(), TrisToRender.end(), [](const Triangle& t1, const Triangle& t2)
//		{
//			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
//			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;
//			return z1 > z2;
//		});

		for (const auto& Proj : TrisToRender)
		{
			std::vector<Triangle> ListTris;

			ListTris.push_back(Proj);
			int NewTris = 1;

			for (int p = 0; p < 4; p++)
			{
				int NewTrisToAdd = 0;
				while (NewTris > 0)
				{
					Triangle Test = ListTris.front();
					ListTris.erase(ListTris.begin());
					NewTris--;

					switch (p)
					{
						case 0:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 1:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 2:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 3:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ (float)sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
					}

					for (int w = 0; w < NewTrisToAdd; w++)
					{
						ListTris.push_back(Clipped[w]);
					}
				}
				NewTris = (int)ListTris.size();
			}

			// draw
			for (auto& ToDraw : ListTris)
			{
				const Material* MatToUse = nullptr;
				const Texture* TexToUse = nullptr;

				if (!MeshToRender.UseSingleMat)
					MatToUse = ToDraw.Material;
				else
					MatToUse = MeshToRender.Materials.at(0);

				ShaderArgs Args(&ToDraw, MatToUse, Cam.Pos, Cam.LookDir, ObjectMatrix, Cam.ViewMatrix, Cam.ProjectionMatrix, SceneLights, LightCount, SHADER_TYPE);

				// Calc lighting (only if lighting is applied at a tri level)
				if ((DoLighting) && SHADER_TYPE == ShaderTypes::SHADER_TRIANGLE)
				{
					Shader(Args);
				}

				if (!WireFrame)
				{
					if (!ShowTriLines)
					{
						if (TerraPGE::DoMultiThreading)
							RenderTriangleThreaded(DepthBuffer, Gdi, Shader, Args);
						else
							RenderTriangle(DepthBuffer, Gdi, Shader, Args);
					}
					else
					{
						if (MeshToRender.MeshName != "Ray")
						{
							Gdi.DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));

							//if (ShowNormals)
							//{
							//	Ray NormalRay = Ray(ToDraw.NormalPositions[0], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay.mesh.UseSingleMat = true;

							//	NormalRay = Ray(ToDraw.NormalPositions[1], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay = Ray(ToDraw.NormalPositions[2], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay = Ray(ToDraw.NormalPositions[3], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);
							//}
						}
						else
						{
							Gdi.DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
						}
					}
				}
				else
				{
					Gdi.DrawTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
				}
			}
		}
	}

	void RenderMeshDepth(GdiPP& Gdi, const Mesh& MeshToRender, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, const Vec3& LightPos, float* Buffer)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
		const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		// Light Matrices (For shadow mapping)
		// TODO calc these better (And probably move to class?)
		Matrix LightProjectionMat = Matrix::CalcOrthoMatrix(-40.0f, 40.0f, -40.0f, 40.0f, TerraPGE::FNEAR, TerraPGE::FFAR);
		Matrix LightViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightPos).Normalized()) * 50.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
		Matrix LightVP = LightViewMatrix * LightProjectionMat;

		//TODO MultiThread??
		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space / World Space
			Triangle Proj = Tri;
			Proj.ApplyMatrix(ObjectMatrix);

			for (int i = 0; i < 3; i++)
			{
				Proj.WorldSpaceVerts[i] = Proj.Points[i];
			}

			if (TerraPGE::DoShadows)
			{
				// 3D Space -> Viewed Space -> Clipped Space
				Proj.ApplyMatrix(LightVP);
				// Clipped Space -> NDC Space
				Proj.ApplyPerspectiveDivide();

				// Offset to viewport space (Ndc -> Screen Space)
				Proj.Scale(Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
				Proj.Translate(Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

				TerraPGE::RenderTriangleDepthOnly(&Proj, TerraPGE::ShadowMap, TerraPGE::ShadowMapWidth, TerraPGE::ShadowMapHeight, LightVP);
			}
		}
	}

	template<typename T>
	void RenderRenderable(GdiPP& Gdi, Camera& Cam, Renderable& R, LightObject LightSrc, T&& Shader, const int SHADER_TYPE = SHADER_FRAGMENT)
	{
		RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightPos, LightSrc.Color, LightSrc.AmbientCoeff, LightSrc.DiffuseCoeff, LightSrc.SpecularCoeff, Shader, SHADER_TYPE);
	}

	//TerraGL (Proposed name for GdiPP)
	//TerraPGE (Proposed name for this engine)
	//Engine Pipeline (In a Loop): Check Window State -> Check Inputs -> Clear Screen -> Make Draw Calls (Rendering Pipeline) -> Draw Double Buffer
	void Run(WndCreator& Wnd, BrushPP& ClearBrush, DoTick_T DrawCallBack)
	{
		// Init Variables and grab screen dimensions
		GdiPP EngineGdi = GdiPP(Wnd.Wnd, true);
		TerraPGE::sx = Wnd.GetClientArea().Width;
		TerraPGE::sy = Wnd.GetClientArea().Height;

		TerraPGE::UpdateSystemInfo();

		delete[] TerraPGE::DepthBuffer;

		// Create depth buffer
		TerraPGE::DepthBuffer = new float[(SIZE_T)(sx * sy)];

		// Initial Client Region
		EngineGdi.UpdateClientRgn();

		// Counters
		MSG msg = { 0 };
		double ElapsedTime = 0.0f;
		double FpsCounter = 0.0f;
		int FrameCounter = 0;
		auto Start = std::chrono::system_clock::now();
		SIZE_T CurrMB = 0;

		// Init Cursor position
		SetCursorPos(sx / 2, sy / 2);
		GetCursorPos(&Tmp);
		PrevMousePos = { (float)Tmp.x, (float)Tmp.y };

		std::vector<Renderable> ToRender;
		std::vector<LightObject*> Lights;

		while (!GetAsyncKeyState(VK_RETURN))
		{
			Start = std::chrono::system_clock::now();

			PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);

			// Translate and Dispatch message to WindowProc
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			// Check Msg
			if (msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY)
			{
				break;
			}


			// Check window focus and calc mouse deltas
			if (Wnd.HasFocus())
			{
				if (UpdateMouseIn)
				{
					GetCursorPos(&Tmp);

					DeltaMouse.x = Tmp.x - PrevMousePos.x;
					DeltaMouse.y = -(Tmp.y - PrevMousePos.y);
					DeltaMouse.x *= Sensitivity;
					DeltaMouse.y *= Sensitivity;
				}

				if (LockCursor)
					SetCursorPos(sx / 2, sy / 2);

				GetCursorPos(&Tmp);
				PrevMousePos = { (float)Tmp.x, (float)Tmp.y };

				if (CursorShow == false)
				{
					while (ShowCursor(FALSE) >= 0) {}
				}
				else
				{
					while (ShowCursor(TRUE) <= 0) {}
				}
			}

			// clear the screen
			EngineGdi.Clear(GDIPP_PIXELCLEAR, ClearBrush);

			// Clear Buffers
			std::fill(DepthBuffer, DepthBuffer + sx * sy, 1.0f);
			std::fill(ShadowMap, ShadowMap + ShadowMapWidth * ShadowMapHeight, 1.0f);

			ToRender.clear();
			Lights.clear();

			// call draw code
			DrawCallBack(EngineGdi, Wnd, (float)ElapsedTime, &ToRender, &Lights);

			LightObject** LightsToRender = new LightObject * [Lights.size()];

			for (int idx = 0; idx < Lights.size(); idx++)
			{
				LightsToRender[idx] = Lights.at(idx);
			}

			// Depth Pass
			for (const auto& Rend : ToRender)
			{
				RenderMeshDepth(EngineGdi, Rend.mesh, Rend.Scalar, Rend.RotationRads, Rend.Pos, LightsToRender[0]->LightPos, ShadowMap);
			}

			for (auto& Rend : ToRender)
			{
				TerraPGE::RenderMesh(EngineGdi, Rend.Cam, Rend.mesh, Rend.Scalar, Rend.RotationRads, Rend.Pos, LightsToRender, Lights.size(), Rend.Shader, Rend.SHADER_TYPE);
			}

			if (FpsEngineCounter)
			{
#ifdef UNICODE
				// Draw FPS and some debug info
				std::wstring Str = FpsWStr + std::to_wstring(Fps) + L" Memory Usage: " + std::to_wstring(CurrMB) + L"/" + std::to_wstring(TerraPGE::MaxMemoryMB) + L" MB " + (DoMultiThreading ? L"(MultiThreaded)" : L"");
				Wnd.SetWndTitle(Str);
				EngineGdi.DrawStringW(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
				std::string Str = FpsStr + std::to_string(Fps) + " Memory Usage: " + std::to_string(CurrMB) + "/" + std::to_string(TerraPGE::MaxMemoryMB) + " MB " + (DoMultiThreading ? "(MultiThreaded)" : "");;
				Wnd.SetWndTitle(Str);
				EngineGdi.DrawStringA(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
			}

			// Draw to screen
			EngineGdi.DrawDoubleBuffer();

			// Calc elapsed time
			ElapsedTime = std::chrono::duration<double>(std::chrono::system_clock::now() - Start).count();

			if (FpsEngineCounter)
			{
				if (FpsCounter >= 1)
				{
					Fps = FrameCounter;
					FrameCounter = 0;
					FpsCounter = 0;
				}
				else
				{
					FrameCounter++;
					FpsCounter += ElapsedTime;
				}

				// Get current memory usage
				CurrMB = GetUsedMemory();
			}
		}

		Wnd.Destroy();

		EngineCleanup();
	}
}
