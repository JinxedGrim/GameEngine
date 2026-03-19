#pragma once
#define NOMINMAX
#include "Renderer.h"
#include "Scene.h"
#include "Physics.h"
#include "RayCaster.h"
#include "Skybox.h"

// The entire premise of this project is to build a game engine without using any libraries, but the std library and the WinAPI. 
// GdiPP is more or less straight WinAPI, but you do have to link with a dll which is sort of cheating but it's about the only way to do graphics without any libs like DX or OpenGl
// Direct2D and DirectWrite are justified the same way

//     TO DO 
// X. Edge funcs on rasterizer
// X. Fix texture interpolation in clipper 
// X. Redesign multithreading
// X. implement a prefab system
// X. Bloom
// X  Add some PBR stuff (metallness, roughness)
// X. Skybox sun sampling (color/intensity/exposure/etc)
// X. Caclulate Vertex norms for all 3 vertices and store them
// X. Better reosource management
// X. Fix camera PointAt
// X. Full PBR lighting
// X. Clear coat layers
// X. Sub surface scattering
// X. GGX 
// X. IBL || IBL approxZ
// X. experiment with terrain deformations
// X. Unify logging
// X. More UI
// X. Audio system 
// X. Voice 
// X. SpotLights
// X. Ambient occlusion 
// X. Global illumination
// X. FXAA, TSAA, Etc
// X. Proper UI system
// X. Probably migrate to unicode
// X. Add cvars
// X. LOD support
// X. Texture streaming
// X. Seed based generation
// X. Normal debug
// X. motion blur
// X. depth pre-pass
// X. temporal accumulation buffer
// X. Mip-Mapping
// X. Color Grading / LUTS
// X. Screen Space edge detection -- (For fxaa and can  be used to debug fxaa)
// X. Depth of field
// X. Exposure adaptation
// X. Depth based fog / scattering
// X. Particle system
// X. Debug rays
// X. 

namespace TerraPGE
{
	class SceneManager
	{
		static SceneManager* Instance;

		Scene* CurrScene = nullptr;
		Renderable** RenderQueue = nullptr;
		LightObject** LightsToRender = nullptr;

		const std::vector<Renderable*>* SceneRenderQueue = nullptr;
		const std::vector<LightObject*>* SceneLights = nullptr;
		std::vector<Renderable*> Roots;

		SceneManager(Scene* FirstScene)
		{
			CurrScene = FirstScene;
		}

		public:
		static SceneManager* GetInstance()
		{
			if (Instance == nullptr)
			{
				return nullptr;
			}

			return Instance;
		}

		static SceneManager* Create(Scene* FirstScene)
		{
			return new SceneManager(FirstScene);
		}

		void UpdateScene(WndCreator& Wnd, double ElapsedTime)
		{
			CurrScene->ClearRenderQueue();
			CurrScene->ClearLights();

			Roots.clear();

			// call draw code
			CurrScene->RunTick(Renderer::EngineGdi, Wnd, (float)ElapsedTime);

			SceneRenderQueue = CurrScene->GetObjects();
			SceneLights = CurrScene->GetLights();

			LightsToRender = DEBUG_NEW LightObject * [CurrScene->GetLights()->size()];
			RenderQueue = DEBUG_NEW Renderable * [CurrScene->GetObjects()->size()];

			Roots = CurrScene->GetRoots();
		}
	};

	// Render function for rendering entire meshes
	//Rendering Pipeline: Recieve call with mesh info including positions & lights sources -> Calc and apply matrices + normals -> backface culling	
	// -> Clipping + Frustum culling -> Call Draw Triangle from graphics API with shader supplied -> Shader called from triangle routine with pixel info + normals -> SetPixel
	static double FpsCounter = 0.0f;
	static int FrameCounter = 0;
	static SIZE_T CurrMB = 0;
	static Scene* CurrScene = nullptr;
	static int Fps = 0;
	static float Sensitivity = 0.1f;
	static Vec2 PrevMousePos;
	static const double PhysicsTick = 1 / 30.0f;
	static bool DoPhysics = false;
	static bool ApplyGravity = true;
	static float FrameTime = 0.0f;
	static float PhysTime = 0.0f;
	static bool DebugColliders = true;



	//template<typename T>
	/*void RenderRenderable(GdiPP& Gdi, Camera& Cam, Renderable& R, LightObject LightSrc, T&& Shader, const int SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		Renderer::RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightPos, LightSrc.Color, LightSrc.AmbientCoeff, LightSrc.DiffuseCoeff, LightSrc.SpecularCoeff, Shader, SHADER_TYPE);
	}*/

	void UpdateLoadingScreen(WndCreator& Wnd, Scene * scene)
	{
		MSG msg;
		Renderer::RenderingCore::UpdateWindow(Wnd, &msg);
		Renderer::RenderingCore::ClearScreen(true);
		scene->DrawLoadingScreen(Renderer::EngineGdi);
		Renderer::EngineGdi->DrawDoubleBufferPO();
	}


	namespace DebugUtils
	{
		void DrawDebugCollider(const Collider* c)
		{
			switch (c->type)
			{
				case ColliderType::AABB:
				{
					Vec3 min = c->GetPosition() + c->_AABBParams.offset - c->_AABBParams.halfExtents * c->GetParentScale();
					Vec3 max = c->GetPosition() + c->_AABBParams.offset + c->_AABBParams.halfExtents * c->GetParentScale();
					Vec3 col = { 0.0f, 0.0f,  0.0f };

					if (!c->PhysicsEnabled)
						col = { 1, 1, 1 };
					else if (c->body.Velocity == Vec3(0.0f, 0.0f, 0.0f))
						col = { 0,  255, 255 };
					else if(c->IsColliding)
						col = { 255,  0, 0 };
					else
						col = { 0,  255, 0 };

					Vec3 v0 = { min.x, min.y, min.z };
					Vec3 v1 = { max.x, min.y, min.z };
					Vec3 v2 = { max.x, max.y, min.z };
					Vec3 v3 = { min.x, max.y, min.z };

					Vec3 v4 = { min.x, min.y, max.z };
					Vec3 v5 = { max.x, min.y, max.z };
					Vec3 v6 = { max.x, max.y, max.z };
					Vec3 v7 = { min.x, max.y, max.z };

					// bottom
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v0, v1, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v1, v2, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v2, v3, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v3, v0, col);

					// top
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v4, v5, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v5, v6, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v6, v7, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v7, v4, col);

					// vertical
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v0, v4, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v1, v5, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v2, v6, col);
					Renderer::RenderingCore::Render3DLine(CurrScene->MainCamera, v3, v7, col);

					break;
				}
				case ColliderType::None:
				{
					break;
				}
				default:
					throw;
			};
		}

		void  DrawAllColliders(Renderable** Objects, size_t Sz)
		{
			for (size_t idx = 0; idx < Sz; idx++)
			{
				Collider* c = &(Objects[idx]->collider);

				DrawDebugCollider(c);
			}
		}
	}


	//TerraGL (Proposed name for GdiPP)
	//TerraPGE (Proposed name for this engine)
	//Engine Pipeline (In a Loop): Check Window State -> Check Inputs -> Clear Screen -> Make Draw Calls (Rendering Pipeline) -> Draw Double Buffer
	void Run(WndCreator& Wnd, Scene* FirstScene)
	{
#ifdef _DEBUG
		_CrtMemState stateBefore;
		_CrtMemState stateAfter;
		_CrtMemState stateDiff;
#endif


		// Counters
		MSG msg                      = { 0 };
		LightObject** LightsToRender = nullptr;
		Renderable** RenderQueue = nullptr;
		double ElapsedTime           = 0.0;
		double physicsAccumulator    = 0.0;
		CurrScene                    = FirstScene;
		const std::vector<Renderable*>* SceneRenderQueue = nullptr;
		const std::vector<LightObject*>* SceneLights     = nullptr;
		std::vector<Renderable*> Roots;

		Wnd.RegisterRawInput();
		Renderer::RenderingCore::PrepareRenderingBackend(Wnd);
		Renderer::RenderingCore::UpdateWindow(Wnd, &msg);
		SceneManager* _sceneManager = SceneManager::Create(CurrScene);

		UpdateLoadingScreen(Wnd, CurrScene);
		CurrScene->BeginScene(Wnd);
		
		auto FrameStart      = std::chrono::steady_clock::now();
		uint64_t CpuFrameStart = Renderer::RenderingUtils::Profiler::GetCpuUsageInfo();
		auto LastPhysicsTime = std::chrono::steady_clock::now();
		double framePhysicsDelta = 0.0;
		auto now = std::chrono::steady_clock::now();
		CodeTimer Timer;

		while (!Wnd.Input.IsKeyPressed(VK_RETURN))
		{
#ifdef _DEBUG
			_CrtMemCheckpoint(&stateBefore);
#endif
			FrameStart = std::chrono::steady_clock::now();
			CpuFrameStart = Renderer::RenderingUtils::Profiler::GetCpuUsageInfo();

			Renderer::RenderingCore::UpdateWindow(Wnd, &msg);
			Renderer::RenderingCore::ClearScreen();

			CurrScene->ClearRenderQueue();
			CurrScene->ClearLights();
			Roots.clear();

			// call draw code
			CurrScene->RunTick(Renderer::EngineGdi, Wnd, (float)ElapsedTime);

			Roots = CurrScene->GetRoots();
			SceneRenderQueue = CurrScene->GetObjects();
			SceneLights      = CurrScene->GetLights();

			LightsToRender = DEBUG_NEW LightObject * [SceneLights->size()];
			RenderQueue    = DEBUG_NEW Renderable * [SceneRenderQueue->size()];

			for (size_t idx = 0; idx < SceneLights->size(); idx++)
			{
				LightsToRender[idx] = SceneLights->at(idx);
			}

			for (size_t idx = 0; idx < SceneRenderQueue->size(); idx++)
			{
				RenderQueue[idx] = SceneRenderQueue->at(idx);
			}


			Timer.Start();
			now = std::chrono::steady_clock::now();
			framePhysicsDelta = std::chrono::duration<double>(now - LastPhysicsTime).count();
			LastPhysicsTime = now;

			if (DoPhysics)
			{
				physicsAccumulator += framePhysicsDelta;
			}

			// Run fixed update as many times as needed
			while (physicsAccumulator >= PhysicsTick)
			{
				//TODO replace with a max constant
				for (size_t idx = 0; idx < SceneRenderQueue->size(); idx++)
				{
					Renderable* Floor = nullptr;
					RaycastHit FloorHit;
					Collider* FloorCollider = nullptr;

					if (!RenderQueue[idx]->collider.PhysicsEnabled)
						continue;

					if (ApplyGravity)
					{
						float minDistance = 9999.0f;

						for (int idx2 = 0; idx2 < SceneRenderQueue->size(); idx2++)
						{
							if (idx == idx2)
								continue;

							Renderable* FloorCandidate = RenderQueue[idx2];
							Matrix CandidateWorld = FloorCandidate->Transform.GetWorldMatrix();

							Vec3 ObjectWorldPos = RenderQueue[idx]->Transform.GetWorldPosition();

							//TODO Define somewhere / Derive from gravity derection
							Vec3 WorldDown = Vec3(0.0f, -1.0f, 0.0f);

							Ray down(ObjectWorldPos, WorldDown);
							RaycastHit Out;

							//TODO Decide if i want this mesh based or collider based (collider  since physics? but mesh has better accuracy for falling)
							//if (RaycastMesh(down, FloorCandidate->mesh->Triangles, &Out, &CandidateWorld))
							if(FloorCandidate->collider.TestCollision(&down, &Out))
							{
								if (Out.distance < minDistance)
								{
									Floor = FloorCandidate;
									FloorCollider = &(FloorCandidate->collider);
									FloorHit = Out;

									minDistance = Out.distance;
								}
							}
						}
					}

					if (RenderQueue[idx]->collider.PhysicsEnabled)
					{
						Physics::Integrate(&((RenderQueue[idx])->collider), PhysicsTick, FloorCollider, &FloorHit);
					}
				}

				physicsAccumulator -= PhysicsTick;
			}

			for (Renderable* Obj : Roots)
			{
				Obj->Transform.UpdateTransformChain();
			}

			PhysTime = Timer.Stop();

			Timer.Start();
			if (!Core::DoMultiThreading && !Core::SimdAcceleration)
				Renderer::RenderScene(CurrScene->MainCamera, RenderQueue, LightsToRender, SceneRenderQueue->size(), SceneLights->size(), CurrScene->SkyboxToRender);
			else if (!Core::DoMultiThreading && Core::SimdAcceleration)
				Renderer::SIMD::RenderScene(CurrScene->MainCamera, RenderQueue, LightsToRender, SceneRenderQueue->size(), SceneLights->size(), CurrScene->SkyboxToRender);
			else
				Renderer::Multithreaded::RenderScene(CurrScene->MainCamera, RenderQueue, LightsToRender, SceneRenderQueue->size(), SceneLights->size(), CurrScene->SkyboxToRender);
			FrameTime = Timer.Stop();

			CurrScene->DrawSceneGUI(Renderer::EngineGdi,  (float)ElapsedTime);

			if (Core::FpsEngineCounter)
			{
				double CpuFrameDelta = Renderer::RenderingUtils::Profiler::GetCpuUsageInfo() - CpuFrameStart;
				Renderer::RenderingCore::DrawFpsCounter(Wnd, Fps, CurrMB, ElapsedTime, Renderer::RenderingUtils::Profiler::CalculateCpuUsage(CpuFrameDelta * 1e-9, ElapsedTime, Renderer::CpuCores));
			}

			if (DebugColliders)
			{
				DebugUtils::DrawAllColliders(RenderQueue, SceneRenderQueue->size());
			}

			delete[] LightsToRender;
			delete[] RenderQueue;


			// Draw to screen
			Renderer::EngineGdi->DrawDoubleBufferPO();

			// Calc elapsed time
			ElapsedTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - FrameStart).count();

			if (Core::FpsEngineCounter)
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
				CurrMB = Renderer::RenderingUtils::Profiler::GetUsedMemory();
			}

#ifdef _DEBUG
			_CrtMemCheckpoint(&stateAfter);

			if (_CrtMemDifference(&stateDiff, &stateBefore, &stateAfter))
			{
				// This prints only allocations that happened inside RenderFrame
				_CrtMemDumpStatistics(&stateDiff);
				_CrtMemDumpAllObjectsSince(&stateBefore);
			}
#endif
		}

		CurrScene->EndScene();
		Renderer::RenderingCore::DeleteRenderingBackend();
	}
}


