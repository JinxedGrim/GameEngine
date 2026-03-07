#pragma once
#include "ParrallelPP.h"
#include "Rasterizer.h"
#include "Renderable.h"
#include "EnvironmentRenderable.h"
#include "ParrallelPP.h"

namespace TerraPGE::Renderer
{
	enum class RenderingBackend
	{
		CPU = 0,
		MULTITHREADED = 1,
		GPU = 2
	};

	static RenderingBackend CurrBackend = RenderingBackend::CPU;
	static GdiPP* EngineGdi = nullptr;

	static BrushPP ClearBrush = -1;
	static const Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };

	bool DebugClip = false;
	bool DoCull = true;


	namespace RenderingCore
	{
		void PrepareRenderingBackend(WndCreator& Wnd)
		{
			Core::UpdateSystemInfo();
			std::stringstream msg;
			Core::Log("[RENDERER] Initializing Rendering Backend");
			msg << "[CPU] Name: " << TerraPGE::Core::CpuName << std::endl << "[CPU] Cores: " << Core::CpuCores << std::endl << "[CPU] " << Core::SimdInfo.GetSupportString();
			Core::Log(msg.str());

			if (Core::SimdInfo.SSE42)
			{
				Core::Log("[RENDERER] Detected >= SSE 4.2 Activating SIMD Acceleration");
				Core::SimdAcceleration = true;
			}
			else
				Core::SimdAcceleration = false;

			msg.str("");
			msg.clear();
			std::wstring GpuDev = TerraPGE::Core::GetDevList();
			std::string s(GpuDev.begin(), GpuDev.end());
			msg << "[RENDERER] Other Devices: " << s << std::endl;
			Core::Log(msg.str());



			switch (CurrBackend)
			{
				case RenderingBackend::CPU:
				{
					EngineGdi = new GdiPP(Wnd.Wnd, true);
					Core::sx = Wnd.GetClientArea().Width;
					Core::sy = Wnd.GetClientArea().Height;

					msg.str("");
					msg.clear();
					msg << "[CPU] Created GDI object with WxH: " << Core::sx << "x" << Core::sy;

					Core::Log(msg.str());

					delete[] Core::DepthBuffer;
					delete[] Core::FrameBuffer;

					// Create depth buffer
					Core::DepthBuffer = DEBUG_NEW float[(SIZE_T)(Core::sx * Core::sy)];
					Core::FrameBuffer = DEBUG_NEW float[(SIZE_T)(Core::sx * Core::sy * 3)];

					// Initial Client Region
					EngineGdi->UpdateClientRgn();
					Core::Log("[CPU] Rendering backend created successfully\n");
					break;
				}
				default:
					Core::LogError("[RENDERER]", "Unsupported Rendering backend!", 1);
			}
		}


		void DispatchRenderingCall()
		{

		}


		void SetClearColor(int BrushIdx)
		{
			ClearBrush = (HBRUSH)GetStockObject(BrushIdx);
		}


		void ClearScreen()
		{
			// clear the screen

			if ((HBRUSH)ClearBrush == (HBRUSH)-1)
			{
				SetClearColor(BLACK_BRUSH);
			}

			std::fill(Core::DepthBuffer, Core::DepthBuffer + Core::sx * Core::sy, 1.0f);
			std::fill(Core::ShadowMap, Core::ShadowMap + Core::ShadowMapWidth * Core::ShadowMapHeight, 1.0f);
			std::fill(Core::FrameBuffer, Core::FrameBuffer + (Core::sx * Core::sy * 3), 0.0f);
		}


		void SwapFrameBuffer(bool Hdr, bool GammaCorrection)
		{
			for (int i = 0; i < Core::sx * Core::sy; i++)
			{
				int index = i * 3;
				float* ChannelPtr = Core::FrameBuffer + index;

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

				int y = i / Core::sx;
				int x = i % Core::sx;

				// Write to out buffer
				EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
			}
		}
	}


	namespace RenderingUtils
	{
		__inline bool ShouldCulltriangle(const Vec3& WorldSpacePos, Vec3 FaceNormal, const Vec3& CamPos)
		{
			//if ((TriNormal.Dot(WorldSpaceTri.Points[0] - Cam->Pos) < 0.0f) || !TerraPGE::DoCull || !MeshToRender->BackfaceCulling) // backface culling
			float facing = FaceNormal.Dot(WorldSpacePos - CamPos);
			return facing >= 0.0f;
		}


		__inline bool ShouldCulltriangle(const Vec3& ViewSpacePos, const Vec3& FaceNormal, const Matrix& ViewMatrix)
		{
			Vec3 viewPos = ViewSpacePos * ViewMatrix;
			Vec3 normalVS = FaceNormal * ViewMatrix;
			float facing = normalVS.Dot(viewPos);
			return facing >= 0.0f;
		}


		void PrepareLights(LightObject** SceneLights, size_t LightCount)
		{
			// already do this? TODO
			for (int i = 0; i < LightCount; i++)
			{
				LightObject* Light = SceneLights[i];

				Light->CalcVpMats();
			}
		}


		__inline Triangle TransformToWorld(const Triangle& Tri, const Matrix& Model)
		{
			Triangle out = Tri;
			out.ApplyMatrix(Model);

			for (int i = 0; i < 3; ++i)
				out.WorldSpaceVerts.Points[i] = out.Points.Points[i];

			return out;
		}


		__inline Triangle ProjectToViewSpace(const Triangle* Tri, Camera* Cam)
		{
			// 3d Space -> Viewed Space
			Triangle Result = *Tri;
			Result.ApplyMatrix(Cam->GetViewMatrix());

			return Result;
		}


		std::vector<Triangle> VertexShader(Camera* Cam, Renderable* Object)
		{
			std::vector<Triangle> ClipSpaceTris = {};
			ClipSpaceTris.reserve(Object->mesh->Triangles.size());

			Vec3 NormPos = Vec3(0, 0, 0);
			Vec3 NormDir = Vec3(0, 0, 0);
			Vec3 TriNormal;

			for (const Triangle& Tri : Object->mesh->Triangles)
			{
				// 3D Space / World Space
				Triangle WorldSpaceTri = RenderingUtils::TransformToWorld(Tri, Object->Transform.GetWorldMatrix());

				//TODO Fix this whole damn thing calc normals at mesh gen or at vertex shade time fuck the rest
				TriNormal = Tri.FaceNormal;
				if (Object->mesh->Normals.size() == 0)
				{
					NormPos = ((Tri.Points.Points[0] + Tri.Points.Points[1] + Tri.Points.Points[2]) / 3.0f);
					NormDir = (Tri.Points.Points[1] - Tri.Points.Points[0]).GetVec3().CrossNormalized((Tri.Points.Points[2] - Tri.Points.Points[0])).Normalized();
					TriNormal = -(WorldSpaceTri.Points.Points[1] - WorldSpaceTri.Points.Points[0]).GetVec3().CrossNormalized((WorldSpaceTri.Points.Points[2] - WorldSpaceTri.Points.Points[0])).Normalized(); // this line and the if statement is used for culling

					for (int i = 0; i < 3; i++)
					{
						WorldSpaceTri.VertexNormals[i] *= Object->Transform.Normal;
					}

					WorldSpaceTri.NormalPositions[0] = NormPos;
					WorldSpaceTri.NormalPositions[1] = Tri.Points.Points[0];
					WorldSpaceTri.NormalPositions[2] = Tri.Points.Points[1];
					WorldSpaceTri.NormalPositions[3] = Tri.Points.Points[2];
					WorldSpaceTri.NormDirections[0] = NormDir;
				}
				else
				{
					TriNormal *= Object->Transform.Normal;
					for (int i = 0; i < 3; i++)
					{
						WorldSpaceTri.VertexNormals[i] *= Object->Transform.Normal;
					}

					NormDir = WorldSpaceTri.FaceNormal;
					NormPos = WorldSpaceTri.NormalPositions[0];
				}

				if (!RenderingUtils::ShouldCulltriangle(WorldSpaceTri.Points.Points[0], TriNormal, Cam->GetWorldPosition()) || !DoCull) // backface culling
				{
					WorldSpaceTri.FaceNormal = TriNormal;
					Triangle ViewSpaceTri = RenderingUtils::ProjectToViewSpace(&WorldSpaceTri, Cam);

					ViewSpaceTri.ApplyMatrix(Cam->GetProjectionMatrix());

					ClipSpaceTris.push_back(ViewSpaceTri);
				}
			}
			return ClipSpaceTris;
		}


		std::vector<Triangle> Clipping(const Triangle* Tri)
		{
			Triangle Clipped[2];

			std::vector<Triangle> ListTris;
			ListTris.push_back(*Tri);
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
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
							break;
						}
						case 1:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)Core::sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
							break;
						}
						case 2:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
							break;
						}
						case 3:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ (float)Core::sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
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

			return ListTris;
		}


		std::vector<Triangle> ClippingClipSpace(const Triangle* Tri)
		{
			std::vector<Triangle> Tris;
			Tris.push_back(*Tri);

			Vec4 planes[6] = {
				{  0,  0,  1,  0 }, // z >= 0
				{  1,  0,  0,  1 }, // x >= -w
				{ -1,  0,  0,  1 }, // x <=  w
				{  0,  1,  0,  1 }, // y >= -w
				{  0, -1,  0,  1 }, // y <=  w
				{  0,  0, -1,  1 }  // z <=  w
			};

			for (int p = 0; p < 6; ++p)
			{
				std::vector<Triangle> Next;

				for (const Triangle& T : Tris)
				{
					Triangle O1, O2;
					int count = T.ClipAgainstPlane(planes[p], O1, O2, DebugClip);

					if (count >= 1)
						Next.push_back(O1);
					if (count == 2)
						Next.push_back(O2);
				}

				Tris.swap(Next);

				if (Tris.empty())
					break;
			}

			return Tris;
		}


		void BuildRenderQueue(const Renderable** Renderables, int Sz)
		{
			// This is where i can do filtering and checking for transparency and stuff TODO
		}
	}


	// Some sort of way to force backend to implement this
	// Probably this Renderer Becomes a class thats derivable
	void RenderFormattedText(std::string)
	{

	}


	void DrawFpsCounter(WndCreator& Wnd, const float& Fps, const SIZE_T CurrMB, double FrameTime, double CpuUsage)
	{
#ifdef UNICODE
		// Draw FPS and some debug info
		std::wstringstream ss;
		ss << std::fixed << std::setprecision(2) << Core::FpsWStr << Fps << L" Cpu/Time: " << CpuUsage << L"/" << FrameTime << L" Memory Usage: " << CurrMB << L"/" 
			<< Core::MaxMemoryMB << L" MB " << (Core::DoMultiThreading ? L"(MultiThreaded)" : L"") << (Core::SimdAcceleration ? L" (SIMD)" : L"");

		//Wnd.SetWndTitle(Str);
		Renderer::EngineGdi->DrawStringW(20, 20, ss.str(), RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << Core::FpsStr << Fps << " Cpu/Time: " << CpuUsage << "/" << FrameTime << " Memory Usage: " << CurrMB << "/"
			<< Core::MaxMemoryMB << " MB " << (Core::DoMultiThreading ? "(MultiThreaded)" : "") << (Core::SimdAcceleration ? " (SIMD)" : L"");		//Wnd.SetWndTitle(Str);
		EngineGdi.DrawStringA(20, 20, ss.str(), RGB(255, 0, 0), TRANSPARENT);
#endif
	}


	void RenderMesh(Camera* Cam, Renderable* Object, LightObject** SceneLights, size_t LightCount)
	{
		std::vector<Triangle> ClipSpaceTris = RenderingUtils::VertexShader(Cam, Object);

		for (const Triangle& ClipSpaceTri : ClipSpaceTris)
		{
			std::vector<Triangle> Clipped = RenderingUtils::ClippingClipSpace(&ClipSpaceTri);

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
				ToDraw.Scale(Vec3((float)(Core::sx * 0.5f), (float)(Core::sy * 0.5f), 1.0f));
				ToDraw.Translate(Vec3((float)(Core::sx * 0.5f), (float)(Core::sy * 0.5f), 0.0f));

				//SceneLights, LightCount
				Args->EditShaderData(TPGE_SHDR_TRI, &ToDraw, 0);



				// Calc lighting (only if lighting is applied at a tri level)
				if ((Renderer::DoLighting) && Object->SHADER_TYPE == ShaderTypes::SHADER_TRIANGLE)
				{
					Object->Shader(Args);
				}


				if (Core::DoMultiThreading)
					Renderer::BaryCentricRasterizer(Core::DepthBuffer, Core::sx, Core::sy, EngineGdi, Object->Shader, Args);
				else
					Renderer::BaryCentricRasterizer(Core::DepthBuffer, Core::sx, Core::sy, EngineGdi, Object->Shader, Args);
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

						Renderer::BaryCentricRasterizerDepth(&Proj, Core::ShadowMap, (SIZE_T)Core::ShadowMapWidth, (SIZE_T)Core::ShadowMapHeight);
					}
					else if (Light->Type == LightTypes::PointLight)
					{
						for (int face = 0; face < 6; face++)
						{
							Matrix VpMatrix = SceneLights[0]->VpMatrices[face];
							Proj.ApplyMatrix(VpMatrix);

							Renderer::BaryCentricRasterizerDepth(&Proj, Core::ShadowMap, (SIZE_T)Core::ShadowMapWidth, (SIZE_T)Core::ShadowMapHeight);
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

		const int W = Core::sx;
		const int H = Core::sy;

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
				Core::SetPixelFrameBuffer(x, y, skyColor.R, skyColor.G, skyColor.B);
			}
		}
	}


	void RenderScene(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
	{
		Renderer::RenderSkybox(Cam, Skybox);

		Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Core::ShadowMap);

		Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

		//Renderer::RenderEnvironment();

		RenderingCore::SwapFrameBuffer(Renderer::UseHDR, Renderer::DoGammaCorrection);
	}




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
						R = Rf * 255.0f;
						G = Gf * 255.0f;
						B = Bf * 255.0f;

						// Write to out buffer
						EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
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
					R = Rf * 255.0f;
					G = Gf * 255.0f;
					B = Bf * 255.0f;

					int y = i / Core::sx;
					int x = i % Core::sx;

					// Write to out buffer
					EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
				}
			}


			void RenderScene(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, EnvironmentRenderable* Skybox = nullptr)
			{
				Renderer::RenderSkybox(Cam, Skybox);

				Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Core::ShadowMap);

				Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

				//Renderer::RenderEnvironment();

				Renderer::RenderingCore::SwapFrameBuffer(Renderer::UseHDR, Renderer::DoGammaCorrection);
				//Renderer::SIMD::SSE::SwapFrameBuffer(Core::FrameBuffer, Core::sx, Core::sy, Renderer::EngineGdi->GetPixelBuffer(), Renderer::UseHDR, Renderer::DoGammaCorrection);
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
					R = Rf * 255.0f;
					G = Gf * 255.0f;
					B = Bf * 255.0f;

					// Write to out buffer
					EngineGdi->QuickSetPixel(x, y, RGB(R, G, B));
				}
			}
		}

		void SwapFrameBuffer(float* Frame, uint64_t Width, uint64_t Height, bool Hdr, bool GammaCorrection)
		{
			uint64_t ChunkSz = std::max<uint64_t>(1, Height / Core::CpuCores);

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

					Core::SetPixelFrameBuffer(x, y, skyColor.R, skyColor.G, skyColor.B);
				}
			}
		}


		void RenderSkybox(Camera* Cam, EnvironmentRenderable* sky)
		{
			if (!sky) return;

			const Matrix& Proj = Cam->GetProjectionMatrix();
			const Matrix3x3 CamRot = Cam->GetRotationMatrix();

			const uint64_t Width = Core::sx;
			const uint64_t Height = Core::sy;

			const uint64_t ChunkSz = std::max<uint64_t>(1, Height / Core::CpuCores);
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

			Renderer::RenderShadowMaps(SceneObjects, SceneLights, ObjectCount, LightCount, Core::ShadowMap);

			Renderer::RenderMeshes(Cam, SceneObjects, SceneLights, ObjectCount, LightCount);

			//Renderer::RenderEnvironment();

			Multithreaded::SwapFrameBuffer(Core::FrameBuffer, Core::sx, Core::sy, UseHDR, Renderer::DoGammaCorrection);
		}
	}

	namespace GPU
	{

	}
}