#pragma once
#include "ParrallelPP.h"
#include "Rasterizer.h"
#include "EnvironmentRenderable.h"
#include "WndCreator.hpp"
#include "GdiPP.hpp"

//TODO
// X. Add icons to text formatter
// X. Add  fonts to format parser



namespace TerraPGE::Renderer
{
	void OpenConsole()
	{
		AllocConsole();

		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
		freopen_s(&fp, "CONIN$", "r", stdin);

		Core::HasOpenConsole = true;
	}

	enum class RenderingBackend
	{
		CPU = 0,
		MULTITHREADED = 1,
		GPU = 2
	};

	struct TEXT_PARAMS
	{
		int X = 0;
		int Y = 0;
		int BkMode = 0;
		COLORREF Clr = RGB(0, 0, 0);
		std::string Text = "";


		TEXT_PARAMS(const int x, const int y, const std::string text, COLORREF clr, const int BkMode)
		{
			this->X = x;
			this->Y = y;
			this->Text = text;
			this->Clr = clr;
			this->BkMode = BkMode;
		}
	};

	static RenderingBackend CurrBackend = RenderingBackend::CPU;
	static GdiPP* EngineGdi = nullptr;

	static BrushPP ClearBrush = -1;
	static const Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };

	static int sx = GetSystemMetrics(SM_CXSCREEN);
	static int sy = GetSystemMetrics(SM_CYSCREEN);

	bool LockCursor = true;
	bool CursorShow = false;

	static float* DepthBuffer = DEBUG_NEW float[sx * sy];
	static float* FrameBuffer = DEBUG_NEW float[sx * sy * 3];

	// move out and into ligting objects
	static int ShadowMapHeight = 1024;
	static int ShadowMapWidth = 1024;
	static float* ShadowMap = DEBUG_NEW float[ShadowMapWidth * ShadowMapHeight];

	static std::string FpsStr = "Fps: ";
	static std::wstring FpsWStr = L"Fps: ";

	static constexpr const char* TPGE_TEXT_COLOR_TOKEN = "\\^c";
	static constexpr const char* TPGE_TEXT_RESET_TOKEN = "\\^r";
	static constexpr const char* TPGE_TEXT_SHIFT_X_TOKEN = "\\^x";
	static constexpr const char* TPGE_TEXT_SHIFT_Y_TOKEN = "\\^y";
	static constexpr const char* TPGE_TEXT_EFFECT_TOKEN = "\\^e";
	static constexpr const char* TPGE_TEXT_NEW_LINE_TOKEN = "\n";
	static constexpr const char* TPGE_TEXT_BK_TOKEN = "\\^b";


	static int CpuCores = 0;
	static SIZE_T MaxMemoryMB = 0;
	static CPUID CpuId(1);
	static SupportedInstructions SimdInfo = { 0 };

	namespace RenderingUtils
	{
		namespace Profiler
		{
			static int GPUDevCount = 0;
			static std::vector<std::wstring> GPUDevNames = {};
			static std::wstring PrimaryGPUDevName;
			std::string CpuName;

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


			void GetCpuInfo()
			{
				SYSTEM_INFO SysInf;

				SimdInfo = CpuId.GetSupportedInstructions();
				GetSystemInfo(&SysInf);
				CpuCores = SysInf.dwNumberOfProcessors;
				CpuName = CpuId.GetProcessorName();
			}


			std::wstring GetDevList()
			{
				std::wstring out = L"";
				for (const std::wstring& str : GPUDevNames)
				{
					out += str;
					out += +L", ";
				}

				return out;
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
					MaxMemoryMB = (MemInf.ullTotalPhys / 1024 / 1024);

				if (EnumDisplayDevices(NULL, DevIdx, &DispDev, 0))
				{
					GPUDevNames.push_back(DispDev.DeviceString);
					GPUDevCount++;

					if (DispDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
						PrimaryGPUDevName = DispDev.DeviceString;
				}

			}
		}


		void PrepareLights(LightObject** SceneLights, size_t LightCount)
		{
			// already do this? TODO
			for (int i = 0; i < LightCount; i++)
			{
				LightObject* Light = SceneLights[i];
				Light->Transform.WalkTransformChain();
				Light->CalcVpMats();
			}
		}


		void BuildRenderQueue(const Renderable** Renderables, int Sz)
		{
			// This is where i can do filtering and checking for transparency and stuff TODO
		}
	}



	namespace RenderingCore
	{
		void PrepareRenderingBackend(WndCreator& Wnd)
		{
			Renderer::RenderingUtils::Profiler::UpdateSystemInfo();
			std::stringstream msg;
			Core::LogInfo("[RENDERER]", "Initializing Rendering Backend");
			msg << "[CPU] Name: " << Renderer::RenderingUtils::Profiler::CpuName << std::endl << "[CPU] Cores: " << Renderer::CpuCores << std::endl << "[CPU] " << Renderer::SimdInfo.GetSupportString();
			Core::LogInfo("[RENDERER]", msg.str());

			if (Renderer::SimdInfo.SSE42)
			{
				Core::LogInfo("[RENDERER]", "Detected >= SSE 4.2 Activating SIMD Acceleration");
				Core::SimdAcceleration = true;
			}
			else
				Core::SimdAcceleration = false;

			msg.str("");
			msg.clear();
			std::wstring GpuDev = Renderer::RenderingUtils::Profiler::GetDevList();
			std::string s(GpuDev.begin(), GpuDev.end());
			msg << "[RENDERER] Other Devices: " << s << std::endl;
			Core::LogInfo("[RENDERER]", msg.str());



			switch (CurrBackend)
			{
				case RenderingBackend::CPU:
				{
					EngineGdi = new GdiPP(Wnd.Wnd, true);
					sx = Wnd.GetClientArea().Width;
					sy = Wnd.GetClientArea().Height;

					msg.str("");
					msg.clear();
					msg << "[CPU] Created GDI object with WxH: " << Renderer::sx << "x" << Renderer::sy;

					Core::LogInfo("[RENDERER]", msg.str());

					delete[] Renderer::DepthBuffer;
					delete[] Renderer::FrameBuffer;

					// Create depth buffer
					Renderer::DepthBuffer = DEBUG_NEW float[(SIZE_T)(Renderer::sx * Renderer::sy)];
					Renderer::FrameBuffer = DEBUG_NEW float[(SIZE_T)(Renderer::sx * Renderer::sy * 3)];

					// Initial Client Region
					EngineGdi->UpdateClientRgn();
					Core::LogInfo("[RENDERER]", "[CPU] Rendering backend created successfully\n");
					break;
				}
				default:
					Core::LogError("[RENDERER]", "Unsupported Rendering backend!", 1);
			}
		}


		void DeleteRenderingBackend()
		{
			Core::LogInfo("[RENDERER]", "Destroying renderer");

			delete[] DepthBuffer;
			delete[] FrameBuffer;
			delete[] ShadowMap;
		}


		void DispatchRenderingCall()
		{

		}


		// Update screen info
		void UpdateScreenInfo()
		{
			// Update GDI info
			EngineGdi->UpdateClientRgn();

			// Update screen dimenstions
			sx = EngineGdi->ClientRect.right - EngineGdi->ClientRect.left;
			sy = EngineGdi->ClientRect.bottom - EngineGdi->ClientRect.top;

			// Update DepthBuffer
			delete[] DepthBuffer;
			DepthBuffer = DEBUG_NEW float[sx * sy];
		}


		void SetClearColor(int BrushIdx)
		{
			ClearBrush = (HBRUSH)GetStockObject(BrushIdx);
		}


		void __inline UpdateCursor(WndCreator& Wnd)
		{
			if (Wnd.HasFocus())
			{
				static POINT Tmp;

				if (LockCursor)
					SetCursorPos(sx / 2, sy / 2);

				if (CursorShow)
					while (ShowCursor(TRUE) < 0) {}
				else
					while (ShowCursor(FALSE) >= 0) {}
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

			UpdateCursor(Wnd);
		}


		// Some sort of way to force backend to implement this
		// Probably this Renderer Becomes a class thats derivable
		void RenderFormattedText(const int& startX, const int& startY, const std::string& str, const COLORREF& InitialColor, const int& BkMode = TRANSPARENT)
		{
			size_t  lastPoint = 0;
			std::vector<TEXT_PARAMS> SubStrs;

			int CurrX = startX;
			int CurrY = startY;
			COLORREF CurrColor = InitialColor;
			int CurrBkMode = BkMode;
			bool RainBowMode = false;
			static int RainbowStep = 0;

			for (size_t i = 0; i < str.length(); i++)
			{
				if (str.at(i) == '\n')
				{
					SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, str.substr(lastPoint, i - lastPoint), CurrColor, CurrBkMode));
					CurrY += 20; // TODO Dont hardcode this (ratio?) also (font, size etc)
					CurrX = startX;

					lastPoint = i + 1;
					continue;
				}
				else if (i + 2 < str.length() && str[i] == '\\' && str[i + 1] == '^') 
				{
					int consumed = 0;

					switch (str.at(i+2))
					{
						case 'c':
						{
							// switch color
							int r, g, b;
							int consumed = 0;

							if (sscanf_s(&str[i], "\\^c{%d}{%d}{%d}%n", &r, &g, &b, &consumed) == 3)
							{
								std::string sub = str.substr(lastPoint, i - lastPoint);
								SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
								CurrX += EngineGdi->MeasureTextWidth(sub.c_str());
								CurrColor = RGB(r, g, b);

								i += consumed - 1;
								lastPoint = i + 1;
								RainBowMode = false;
								continue;
							}
							break;
						}
						case 'b':
						{
							int bkMode;
							int consumed = 0;

							if (sscanf_s(&str[i], "\\^b{%d}%n", &bkMode, &consumed) == 1)
							{
								std::string sub = str.substr(lastPoint, i - lastPoint);
								SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
								CurrX += EngineGdi->MeasureTextWidth(sub.c_str());

								i += consumed - 1;
								lastPoint = i + 1;
								CurrBkMode = bkMode;
								continue;
							}
							break;
						}
						case 'x':
						{
							int consumed = 0;
							int offsetX = 0;

							if (sscanf_s(&str[i], "\\^x{%d}%n", &offsetX, &consumed) == 1)
							{
								std::string sub = str.substr(lastPoint, i - lastPoint);
								SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
								CurrX += EngineGdi->MeasureTextWidth(sub.c_str());

								i += consumed - 1;
								lastPoint = i + 1;
								CurrX += offsetX;
								continue;
							}
							break;
						}
						case 'y':
						{
							int consumed = 0;
							int offsetY = 0;

							if (sscanf_s(&str[i], "\\^y{%d}%n", &offsetY, &consumed) == 1)
							{
								std::string sub = str.substr(lastPoint, i - lastPoint);
								SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
								CurrX += EngineGdi->MeasureTextWidth(sub.c_str());

								i += consumed - 1;
								lastPoint = i + 1;
								CurrY += offsetY;							
								continue;
							}
							break;
						}
						case 'r':
						{
							int consumed = 3;
							std::string sub = str.substr(lastPoint, i - lastPoint);
							SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
							CurrX += EngineGdi->MeasureTextWidth(sub.c_str());
							CurrColor = InitialColor;
							RainBowMode = false;
							CurrBkMode = BkMode;

							i += consumed - 1;
							lastPoint = i + 1;
							continue;
							break;
						}
						case 'e':
						{
							char effect[45] = {'\0'};
							if (sscanf_s(&str[i], "\\^e{%[^}]}%n", effect, (unsigned)_countof(effect), &consumed) == 1)
							{
								if (strcmp(effect, "Rainbow") == 0)
								{
									std::string sub = str.substr(lastPoint, i - lastPoint);
									SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
									CurrX += EngineGdi->MeasureTextWidth(sub.c_str());

									RainBowMode = true;
									CurrColor = RGB(255, 0, 0);
								}

								i += consumed - 1;
								lastPoint = i + 1;
								continue;
							}
							break;
						}
					}
				}
				else if (RainBowMode)
				{
					std::string sub = str.substr(i, 1);

					SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
					CurrX += EngineGdi->MeasureTextWidth(sub.c_str());

					Color c = Color::RainbowColor(RainbowStep++);
					CurrColor = RGB((int)c.R, (int)c.G, (int)c.B);

					lastPoint = i + 1;
					continue;
				}
			}

			if (lastPoint < str.length())
			{
				std::string sub = str.substr(lastPoint);
				SubStrs.push_back(TEXT_PARAMS(CurrX, CurrY, sub, CurrColor, CurrBkMode));
			}

			for (const TEXT_PARAMS& params : SubStrs)
			{
				EngineGdi->DrawStringA(params.X, params.Y, params.Text, params.Clr, params.BkMode);
			}
		}


		void DrawFpsCounter(WndCreator& Wnd, const float& Fps, const SIZE_T CurrMB, double FrameTime, double CpuUsage)
		{
#ifdef UNICODE
			// Draw FPS and some debug info
			std::wstringstream ss;
			ss << std::fixed << std::setprecision(2) << Renderer::FpsWStr << Fps << L" Cpu/Time: " << CpuUsage << L"/" << FrameTime << L" Memory Usage: " << CurrMB << L"/"
				<< Renderer::MaxMemoryMB << L" MB " << (Core::DoMultiThreading ? L"(MultiThreaded)" : L"") << (Core::SimdAcceleration ? L" (SIMD)" : L"");

			//Wnd.SetWndTitle(Str);
			Renderer::EngineGdi->DrawStringW(20, 20, ss.str(), RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << Renderer::FpsStr << Fps << " Cpu/Time: " << CpuUsage << "/" << FrameTime << " Memory Usage: " << CurrMB << "/"
				<< Core::MaxMemoryMB << " MB " << (Core::DoMultiThreading ? "(MultiThreaded)" : "") << (Core::SimdAcceleration ? " (SIMD)" : L"");		//Wnd.SetWndTitle(Str);
			EngineGdi.DrawStringA(20, 20, ss.str(), RGB(255, 0, 0), TRANSPARENT);
#endif
		}


		void ClearScreen()
		{
			// clear the screen

			if ((HBRUSH)ClearBrush == (HBRUSH)-1)
			{
				SetClearColor(BLACK_BRUSH);
			}

			std::fill(Renderer::DepthBuffer, Renderer::DepthBuffer + Renderer::sx * Renderer::sy, 1.0f);
			std::fill(Renderer::ShadowMap, Renderer::ShadowMap + Renderer::ShadowMapWidth * Renderer::ShadowMapHeight, 1.0f);
			std::fill(Renderer::FrameBuffer, Renderer::FrameBuffer + (Renderer::sx * Renderer::sy * 3), 0.0f);
		}


		void SwapFrameBuffer(bool Hdr, bool GammaCorrection)
		{
			for (int i = 0; i < Renderer::sx * Renderer::sy; i++)
			{
				int index = i * 3;
				float* ChannelPtr = Renderer::FrameBuffer + index;

				float Rf, Gf, Bf = 0.0f;
				int R, G, B = 0;

				Rf = ChannelPtr[0];
				Gf = ChannelPtr[1];
				Bf = ChannelPtr[2];
				
				// this is done to remove branches
				float hdrMask = Renderer::UseHDR ? 1.0f : 0.0f;
				Rf = Rf + hdrMask * ((Rf / (1.0f + Rf)) - Rf);
				Gf = Gf + hdrMask * ((Gf / (1.0f + Gf)) - Gf);
				Bf = Bf + hdrMask * ((Bf / (1.0f + Bf)) - Bf);

				Rf = std::clamp<float>(Rf, 0, 1.0f);
				Gf = std::clamp<float>(Gf, 0, 1.0f);
				Bf = std::clamp<float>(Bf, 0, 1.0f);

				// this is done to remove branches
				float gammaMask = Renderer::DoGammaCorrection ? 1.0f : 0.0f;
				Rf = Rf + gammaMask * (Color::LinearToSRGB_Channel(Rf) - Rf);
				Gf = Gf + gammaMask * (Color::LinearToSRGB_Channel(Gf) - Gf);
				Bf = Bf + gammaMask * (Color::LinearToSRGB_Channel(Bf) - Bf);

				// Final transformation to RGB Space
				R = Rf * 255.0f;
				G = Gf * 255.0f;
				B = Bf * 255.0f;

				int y = i / Renderer::sx;
				int x = i % Renderer::sx;

				// Write to out buffer
				EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
			}
		}
	}


	void RenderMesh(Camera* Cam, Renderable* Object, LightObject** SceneLights, size_t LightCount)
	{
		std::vector<Triangle> ClipSpaceTris = VertexShader(Cam, Object);

		for (const Triangle& ClipSpaceTri : ClipSpaceTris)
		{
			std::vector<Triangle> Clipped = Renderer::ClippingClipSpace(&ClipSpaceTri);

			// initialize shader
			ShaderArgs* Args = DEBUG_NEW ShaderArgs();
			Args->AddShaderDataByValue(TPGE_SHDR_TYPE, Object->SHADER_TYPE, 0);
			Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_CAMERA_POS, Cam->Transform.GetWorldPosition(), 0);
			Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_CAMERA_LDIR, Cam->GetLookDirection(), 0);
			Args->AddShaderDataPtr(TPGE_SHDR_CAMERA_VIEW_MATRIX, Cam->_GetViewMatrixPtr(), 0);
			Args->AddShaderDataPtr(TPGE_SHDR_CAMERA_PROJ_MATRIX, Cam->_GetProjectionMatrixPtr(), 0);
			Args->AddShaderDataPtr(TPGE_SHDR_OBJ_MATRIX, Object->Transform._GetWorldMatrixPtr(), 0);
			Args->AddShaderDataPtr(TPGE_SHDR_LIGHT_OBJECTS, SceneLights, 0);
			Args->AddShaderDataByValue<size_t>(TPGE_SHDR_LIGHT_COUNT, LightCount, 0);
			Args->AddShaderDataByValue<bool>(TPGE_SHDR_DEBUG_SHADOWS, DebugShadows);
			Args->AddShaderDataByValue<bool>(TPGE_SHDR_IS_IN_SHADOW, false);
			Args->AddShaderDataPtr(TPGE_SHDR_TRI, nullptr, 0);

			// Draw each tri
			for (Triangle& ToDraw : Clipped)
			{
				// Clip Space -> NDC Space
				ToDraw.ApplyPerspectiveDivide();

				// Offset to viewport space (Ndc -> Screen Space)
				ToDraw.Scale(Vec3((float)(Renderer::sx * 0.5f), (float)(Renderer::sy * 0.5f), 1.0f));
				ToDraw.Translate(Vec3((float)(Renderer::sx * 0.5f), (float)(Renderer::sy * 0.5f), 0.0f));

				//SceneLights, LightCount
				Args->EditShaderData(TPGE_SHDR_TRI, &ToDraw, 0);



				// Calc lighting (only if lighting is applied at a tri level)
				if ((Renderer::DoLighting) && Object->SHADER_TYPE == ShaderTypes::SHADER_TRIANGLE)
				{
					Object->Shader(Args);
				}


				if (Core::DoMultiThreading)
					Renderer::BaryCentricRasterizer(Renderer::FrameBuffer, Renderer::DepthBuffer, Renderer::sx, Renderer::sy, Renderer::ShadowMap, Renderer::ShadowMapWidth, Renderer::ShadowMapHeight, Object->Shader, Args);
				else
					Renderer::BaryCentricRasterizer(Renderer::FrameBuffer, Renderer::DepthBuffer, Renderer::sx, Renderer::sy, Renderer::ShadowMap, Renderer::ShadowMapWidth, Renderer::ShadowMapHeight, Object->Shader, Args);
			}

			Args->Delete();
		}
	}


	// Render function for rendering entire meshes
	// Rendering Pipeline: Recieve call with mesh info including positions & lights sources -> Calc and apply world matrix + normals -> backface culling	
	// -> Clipping + Frustum culling -> Call Draw Triangle from graphics API with shader supplied -> Shader called from triangle routine with pixel info + normals -> SetPixel
	void RenderMeshes(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount /*T&& Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT*/)
	{
		// TODO MultiThread??
		// TODO Allow vertex shaders (:
		// Project and translate object 
		RenderingUtils::PrepareLights(SceneLights, LightCount);

		for (int i = 0; i < ObjectCount; i++)
		{
			TerraPGE::Renderer::RenderMesh(Cam, SceneObjects[i], SceneLights, LightCount);
		}
	}


	void RenderShadowMaps(Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, float* Buffer)
	{
		bool HasLight = LightCount >= 1;

		for (int objIdx = 0; objIdx < ObjectCount; objIdx++)
		{
			Renderable* Object = SceneObjects[objIdx];

			for (const Triangle& Tri : Object->mesh->Triangles)
			{
				// 3D Space / World Space
				Triangle Proj = Tri;
				Proj.ApplyMatrix(Object->Transform.World);

				for (int i = 0; i < 3; i++)
				{
					Proj.WorldSpaceVerts.Points[i] = Proj.Points.Points[i];
				}

				if (Renderer::DoShadows && HasLight)
				{
					// TODO Add more maps for supporting more lights
					LightObject* Light = SceneLights[0];
					// 3D Space -> Viewed Space -> Clipped Space

					if (Light->Type == LightTypes::DirectionalLight)
					{
						const Matrix VpMatrix = Light->VpMatrices[0];
						Proj.ApplyMatrix(VpMatrix);

						Renderer::BaryCentricRasterizerDepth(&Proj, Renderer::ShadowMap, (SIZE_T)Renderer::ShadowMapWidth, (SIZE_T)Renderer::ShadowMapHeight);
					}
					else if (Light->Type == LightTypes::PointLight)
					{
						for (int face = 0; face < 6; face++)
						{
							Matrix VpMatrix = SceneLights[0]->VpMatrices[face];
							Proj.ApplyMatrix(VpMatrix);

							Renderer::BaryCentricRasterizerDepth(&Proj, Renderer::ShadowMap, (SIZE_T)Renderer::ShadowMapWidth, (SIZE_T)Renderer::ShadowMapHeight);
						}
					}
				}
			}
		}
	}

	
	void RenderEnvironment(EnvironmentRenderable** Objects, size_t Count)
	{
		//Vertex Shading

		//for (int i = 0; i < Count; i++)
		//{
		//	Objects[i]->;
		//}
	}


	void RenderSkybox(Camera* Cam, EnvironmentRenderable* sky)
	{
		if (sky == nullptr)
			return;

		const int W = Renderer::sx;
		const int H = Renderer::sy;

		const Matrix& Proj = Cam->GetProjectionMatrix();
		const Matrix3x3 CamRot = Cam->GetRotationMatrix(); // rotation only
		const float Fov = Cam->GetFov();

		for (int y = 0; y < H; ++y)
		{
			for (int x = 0; x < W; ++x)
			{
				// skyboox render
				Color skyColor = sky->Render(x, y, W, H, Fov, CamRot);

				// 5. Write color
				Renderer::RenderingCore::SetPixelFrameBuffer(x, y, Renderer::FrameBuffer, Renderer::sx, skyColor.R, skyColor.G, skyColor.B);
			}
		}
	}


	void RenderScene(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
	{
		Renderer::RenderSkybox(Cam, Skybox);

		Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Renderer::ShadowMap);

		Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

		//Renderer::RenderEnvironment();

		RenderingCore::SwapFrameBuffer(Renderer::UseHDR, Renderer::DoGammaCorrection);
	}


	namespace SIMD
	{
		namespace SSE
		{
			static __inline void ApplyHDR(__m128& R, __m128& G, __m128& B, const __m128& HdrMask, const __m128& one)
			{
				R = _mm_add_ps(R, _mm_mul_ps(HdrMask, _mm_sub_ps(_mm_div_ps(R, _mm_add_ps(one, R)), R)));
				G = _mm_add_ps(G, _mm_mul_ps(HdrMask, _mm_sub_ps(_mm_div_ps(G, _mm_add_ps(one, G)), G)));
				B = _mm_add_ps(B, _mm_mul_ps(HdrMask, _mm_sub_ps(_mm_div_ps(B, _mm_add_ps(one, B)), B)));
			}

			
			void SwapFrameBufferByChunk(float* Frame, const unsigned __int32 width, const unsigned __int32  y0, const unsigned __int32 y1)
			{
				float hdrMask = Renderer::UseHDR ? 1.0f : 0.0f;

				const __m128 zero = _mm_set1_ps(0.0f);
				const __m128 one = _mm_set1_ps(1.0f);
				const __m128 max1 = _mm_set1_ps(1.0f);
				const __m128 scale = _mm_set1_ps(255.0f);

				const __m128 HdrMask = _mm_set1_ps(hdrMask);
				const __m128 gammaMask = _mm_set1_ps(Renderer::DoGammaCorrection ? 1.0f : 0.0f);

				for (unsigned __int32 y = y0; y < y1; ++y)
				{
					unsigned __int32 rowBase = y * width * 3;
					unsigned __int32 x = 0;

					for (; x+3 < width; x+=4)
					{
						float* ChannelPtr = Frame + rowBase + x * 3;

						__m128 R, G, B;

						SIMDUtils::SSE::LoadRGB4(ChannelPtr, R, G, B);

						ApplyHDR(R, G, B, HdrMask, one);

						SIMDUtils::SSE::ClampRGBFloat4(R, G, B, zero, max1);

						// Gamma
						SIMDUtils::SSE::LinearToSRGB4(R);
						SIMDUtils::SSE::LinearToSRGB4(G);
						SIMDUtils::SSE::LinearToSRGB4(B);

						// Scale
						R = _mm_mul_ps(R, scale);
						G = _mm_mul_ps(G, scale);
						B = _mm_mul_ps(B, scale);

						__m128i r, g, b;
						SIMDUtils::SSE::RGB4ToByte(R, G, B, r, g, b);

						// Interleave BGR into 12-byte output
						uint8_t* out = Renderer::EngineGdi->GetPixelBuffer() + rowBase + x * 3;
						SIMDUtils::SSE::RGBStoreBGR(r, g, b, out);
					}
					for (; x < width; ++x)
					{
						float* ChannelPtr = Frame + rowBase + x * 3;

						float Rf = 0.0f, Gf = 0.0f, Bf = 0.0f;
						int R = 0, G = 0, B = 0;

						Rf = ChannelPtr[0];
						Gf = ChannelPtr[1];
						Bf = ChannelPtr[2];

						Rf = Rf + hdrMask * ((Rf / (1.0f + Rf)) - Rf);
						Gf = Gf + hdrMask * ((Gf / (1.0f + Gf)) - Gf);
						Bf = Bf + hdrMask * ((Bf / (1.0f + Bf)) - Bf);

						Rf = std::clamp<float>(Rf, 0, 1.0f);
						Gf = std::clamp<float>(Gf, 0, 1.0f);
						Bf = std::clamp<float>(Bf, 0, 1.0f);

						if (Renderer::DoGammaCorrection)
						{
							// Gamma Correction
							Rf = Color::LinearToSRGB_Channel(Rf);
							Gf = Color::LinearToSRGB_Channel(Gf);
							Bf = Color::LinearToSRGB_Channel(Bf);
						}

						// Final transformation to RGB Space
						R = (int)(Rf * 255.0f);
						G = (int)(Gf * 255.0f);
						B = (int)(Bf * 255.0f);

						// Write to out buffer
						EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
					}
				}
			}


			void RenderSkyboxByChunk(EnvironmentRenderable* sky, const Camera* Cam, const uint64_t y0, const uint64_t y1, const uint64_t width, const uint64_t height, const float& Fov, const Matrix& Proj, const Matrix3x3& CamRot)
			{
				for (uint64_t y = y0; y < y1; ++y)
				{
					for (uint64_t x = 0; x + 3 < width; x+=3)
					{
						Color skyColor = sky->Render(x, y, width, height, Fov, CamRot);

						Renderer::RenderingCore::SetPixelFrameBuffer(x, y, Renderer::FrameBuffer, Renderer::sx, skyColor.R, skyColor.G, skyColor.B);
					}
				}
			}


			void SwapFrameBuffer(float* Src, const int& Width, const int& Height, unsigned __int8* OutBuffer, const bool Hdr, const bool GammaCorrection)
			{
				const int pixelCount = Width * Height;

				const __m128 zero = _mm_set1_ps(0.0f);
				const __m128 one = _mm_set1_ps(1.0f);
				const __m128 max1 = _mm_set1_ps(1.0f);
				const __m128 scale = _mm_set1_ps(255.0f);

				const __m128 hdrMask = _mm_set1_ps(Renderer::UseHDR ? 1.0f : 0.0f);
				const __m128 gammaMask = _mm_set1_ps(Renderer::DoGammaCorrection ? 1.0f : 0.0f);

				int i = 0;
				for (; i <= pixelCount - 4; i += 4)
				{
					float* src = Src + i * 3;

					__m128 R, G, B;

					// Load 12 floats (4 pixels)
					SIMDUtils::SSE::LoadRGB4(src, R, G, B);

					// HDR
					ApplyHDR(R, G, B, hdrMask, one);

					SIMDUtils::SSE::ClampRGBFloat4(R, G, B, zero, max1);

					// Gamma
					SIMDUtils::SSE::LinearToSRGB4(R);
					SIMDUtils::SSE::LinearToSRGB4(G);
					SIMDUtils::SSE::LinearToSRGB4(B);

					// Scale
					_mm_mul_ps(R, scale);
					_mm_mul_ps(G, scale);
					_mm_mul_ps(B, scale);

					__m128i r, g, b;
					SIMDUtils::SSE::RGB4ToByte(R, G, B, r, g, b);

					// Interleave BGR into 12-byte output
					uint8_t* out = OutBuffer + i * 3;
					SIMDUtils::SSE::RGBStoreBGR(r, g, b, out);
				}

				for (; i < pixelCount; ++i)
				{
					int index = i * 3;
					float* ChannelPtr = Src + index;

					float Rf, Gf, Bf = 0.0f;
					int R, G, B = 0;

					Rf = ChannelPtr[0];
					Gf = ChannelPtr[1];
					Bf = ChannelPtr[2];

					// this is done to remove branches
					float hdrMask = Renderer::UseHDR ? 1.0f : 0.0f;
					Rf = Rf + hdrMask * ((Rf / (1.0f + Rf)) - Rf);
					Gf = Gf + hdrMask * ((Gf / (1.0f + Gf)) - Gf);
					Bf = Bf + hdrMask * ((Bf / (1.0f + Bf)) - Bf);

					Rf = std::clamp<float>(Rf, 0, 1.0f);
					Gf = std::clamp<float>(Gf, 0, 1.0f);
					Bf = std::clamp<float>(Bf, 0, 1.0f);

					// this is done to remove branches
					float gammaMask = Renderer::DoGammaCorrection ? 1.0f : 0.0f;
					Rf = Rf + gammaMask * (Color::LinearToSRGB_Channel(Rf) - Rf);
					Gf = Gf + gammaMask * (Color::LinearToSRGB_Channel(Gf) - Gf);
					Bf = Bf + gammaMask * (Color::LinearToSRGB_Channel(Bf) - Bf);

					// Final transformation to RGB Space
					R = (int)(Rf * 255.0f);
					G = (int)(Gf * 255.0f);
					B = (int)(Bf * 255.0f);

					int y = i / Renderer::sx;
					int x = i % Renderer::sx;

					// Write to out buffer
					EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
				}
			}


			void RenderScene(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
			{
				Renderer::RenderSkybox(Cam, Skybox);

				Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Renderer::ShadowMap);

				Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

				//Renderer::RenderEnvironment();

				//Renderer::RenderingCore::SwapFrameBuffer(Renderer::UseHDR, Renderer::DoGammaCorrection);
				Renderer::SIMD::SSE::SwapFrameBuffer(Renderer::FrameBuffer, Renderer::sx, Renderer::sy, Renderer::EngineGdi->GetPixelBuffer(), Renderer::UseHDR, Renderer::DoGammaCorrection);
				Renderer::EngineGdi->SetNeedsPixelsRedrawn();
			}
		}
	}


	namespace Multithreaded
	{
		void SwapFrameBufferByChunk(float* Frame, const unsigned __int32 width, const unsigned __int32  y0, const unsigned __int32 y1)
		{
			float hdrMask = Renderer::UseHDR ? 1.0f : 0.0f;

			for (unsigned __int32 y = y0; y < y1; ++y)
			{
				unsigned __int32 rowBase = y * width * 3;

				for (unsigned __int32 x = 0; x < width; ++x)
				{
					float* ChannelPtr = Frame + rowBase + x * 3;

					float Rf = 0.0f, Gf = 0.0f, Bf = 0.0f;
					int R = 0, G = 0, B = 0;

					Rf = ChannelPtr[0];
					Gf = ChannelPtr[1];
					Bf = ChannelPtr[2];

					Rf = Rf + hdrMask * ((Rf / (1.0f + Rf)) - Rf);
					Gf = Gf + hdrMask * ((Gf / (1.0f + Gf)) - Gf);
					Bf = Bf + hdrMask * ((Bf / (1.0f + Bf)) - Bf);

					Rf = std::clamp<float>(Rf, 0, 1.0f);
					Gf = std::clamp<float>(Gf, 0, 1.0f);
					Bf = std::clamp<float>(Bf, 0, 1.0f);

					if (Renderer::DoGammaCorrection)
					{
						// Gamma Correction
						Rf = Color::LinearToSRGB_Channel(Rf);
						Gf = Color::LinearToSRGB_Channel(Gf);
						Bf = Color::LinearToSRGB_Channel(Bf);
					}

					// Final transformation to RGB Space
					R = (int)(Rf * 255.0f);
					G = (int)(Gf * 255.0f);
					B = (int)(Bf * 255.0f);

					// Write to out buffer
					EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
				}
			}
		}

		void SwapFrameBuffer(float* Frame, uint64_t Width, uint64_t Height, bool Hdr, bool GammaCorrection)
		{
			uint64_t ChunkSz = std::max<uint64_t>(1, Height / Renderer::CpuCores);

			for (uint64_t y = 0; y < Height; y += ChunkSz)
			{
				uint64_t y0 = y;
				uint64_t y1 = std::min(y + ChunkSz, Height);

				if(!Core::SimdAcceleration)
					Core::ThreadPool.EnqueueTask([Frame, y0, y1, Width]()
						{
							SwapFrameBufferByChunk(Frame, Width, y0, y1);
						});
				else
					Core::ThreadPool.EnqueueTask([Frame, y0, y1, Width]()
						{
							Renderer::SIMD::SSE::SwapFrameBufferByChunk(Frame, Width, y0, y1);
						});
			}

			Core::ThreadPool.WaitUntilAllTasksFinished();
		}


		void RenderSkyboxByChunk(EnvironmentRenderable* sky, const Camera* Cam, const uint64_t y0, const uint64_t y1, const uint64_t width, const uint64_t height, const float& Fov, const Matrix& Proj, const Matrix3x3& CamRot)
		{
			for (uint64_t y = y0; y < y1; ++y)
			{
				for (uint64_t x = 0; x < width; ++x)
				{
					Color skyColor = sky->Render(x, y, width, height, Fov, CamRot);

					Renderer::RenderingCore::SetPixelFrameBuffer(x, y, Renderer::FrameBuffer, Renderer::sx, skyColor.R, skyColor.G, skyColor.B);
				}
			}
		}


		void RenderSkybox(Camera* Cam, EnvironmentRenderable* sky)
		{
			if (!sky) return;

			const Matrix& Proj = Cam->GetProjectionMatrix();
			const Matrix3x3 CamRot = Cam->GetRotationMatrix();

			const uint64_t Width = Renderer::sx;
			const uint64_t Height = Renderer::sy;

			const uint64_t ChunkSz = std::max<uint64_t>(1, Height / Renderer::CpuCores);
			const float Fov = Cam->GetFov();

			for (uint64_t y = 0; y < Height; y += ChunkSz)
			{
				uint64_t y0 = y;
				uint64_t y1 = std::min(y + ChunkSz, Height);

				Core::ThreadPool.EnqueueTask([sky, Cam, y0, y1, Width, Height, &Fov, &Proj, &CamRot]()
					{
						RenderSkyboxByChunk(sky, Cam, y0, y1, Width, Height, Fov, Proj, CamRot);
					});
			}

			Core::ThreadPool.WaitUntilAllTasksFinished();
		}


		void RenderScene(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
		{
			Multithreaded::RenderSkybox(Cam, Skybox);

			Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Renderer::ShadowMap);

			Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

			//Renderer::RenderEnvironment();

			Multithreaded::SwapFrameBuffer(Renderer::FrameBuffer, Renderer::sx, Renderer::sy, UseHDR, Renderer::DoGammaCorrection);
		}
	}


	namespace GPU
	{

	}
}


//class RenderingBackend
//{

//};

//class CPUBackend: RenderingBackend
//{

//};


//class Renderer
//{
//	Renderer(RenderingBackend* backend)
//	{

//	}

//	public:

//	Renderer* Create(RenderingBackend* Backend)
//	{

//	}

//	void RenderScene(Camera* Cam, TerraPGE::Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
//	{

//	}
//};





// TODO salvage this code for a debug routine?
//if (false)
//{
	//if (Object->mesh->MeshName != "Ray")
	//{
		//EngineGdi->DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));
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
	//}
	//else
	//{
		//EngineGdi->DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
	//}
//}