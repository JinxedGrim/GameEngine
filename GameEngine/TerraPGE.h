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
// X. Fix texture interpolation in clipper 
// X. Redesign multithreading
// X. implement a prefab system
// X. Fix physics ray cast for ground detection (is mesh local space needs to be world)
// X. Bloom
// X. Skybox sun sampling (color/intensity/exposure/etc)
// X. Caclulate Vertex norms for all 3 vertices and store them
// X. Better reosource management
// X. Fix camera PointAt
// X. PBR lighting
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
// X. Add cpu metrics
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
	float Sensitivity = 0.1f;
	Vec2 PrevMousePos;
	double PhysicsTick = 1 / 30.0f;
	bool DoPhysics = false;
	bool ApplyGravity = true;



	//template<typename T>
	/*void RenderRenderable(GdiPP& Gdi, Camera& Cam, Renderable& R, LightObject LightSrc, T&& Shader, const int SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		Renderer::RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightPos, LightSrc.Color, LightSrc.AmbientCoeff, LightSrc.DiffuseCoeff, LightSrc.SpecularCoeff, Shader, SHADER_TYPE);
	}*/

	void UpdateLoadingScreen()
	{
		Renderer::RenderingCore::ClearScreen();
		CurrScene->DrawLoadingScreen(Renderer::EngineGdi);
		Renderer::EngineGdi->DrawDoubleBufferPO();
	}


	void MemCheckpoint()
	{

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
		MSG msg                   = { 0 };
		double ElapsedTime        = 0.0;
		double physicsAccumulator = 0.0;
		CurrScene = FirstScene;

		Renderable** RenderQueue                         = nullptr;
		const std::vector<Renderable*>* SceneRenderQueue = nullptr;
		LightObject** LightsToRender                     = nullptr;
		const std::vector<LightObject*>* SceneLights     = nullptr;

		std::vector<Renderable*> Roots;

		SceneManager* _sceneManager = SceneManager::Create(CurrScene);

		Wnd.RegisterRawInput();
		Renderer::RenderingCore::PrepareRenderingBackend(Wnd);
		Core::UpdateWindow(Wnd, &msg);
		UpdateLoadingScreen();
		CurrScene->BeginScene(Wnd);
		
		auto FrameStart      = std::chrono::steady_clock::now();
		auto LastPhysicsTime = std::chrono::steady_clock::now();
		uint64_t CpuFrameStart = Core::GetCpuUsageInfo();

		while (!Wnd.Input.IsKeyPressed(VK_RETURN))
		{
#ifdef _DEBUG
			_CrtMemCheckpoint(&stateBefore);
#endif
			FrameStart = std::chrono::steady_clock::now();
			CpuFrameStart = Core::GetCpuUsageInfo();

			Core::UpdateWindow(Wnd, &msg);

			Renderer::RenderingCore::ClearScreen();

			CurrScene->ClearRenderQueue();
			CurrScene->ClearLights();

			Roots.clear();

			// call draw code
			CurrScene->RunTick(Renderer::EngineGdi, Wnd, (float)ElapsedTime);

			SceneRenderQueue = CurrScene->GetObjects();
			SceneLights      = CurrScene->GetLights();

			LightsToRender = DEBUG_NEW LightObject * [CurrScene->GetLights()->size()];
			RenderQueue    = DEBUG_NEW Renderable * [CurrScene->GetObjects()->size()];

			Roots = CurrScene->GetRoots();

			Renderable* Floor = nullptr;
			RaycastHit FloorHit;

			for (size_t idx = 0; idx < SceneLights->size(); idx++)
			{
				LightsToRender[idx] = SceneLights->at(idx);
				LightsToRender[idx]->Transform.WalkTransformChain();
				LightsToRender[idx]->CalcVpMats();
			}

			for (size_t idx = 0; idx < SceneRenderQueue->size(); idx++)
			{
				RenderQueue[idx] = SceneRenderQueue->at(idx);
			}

			auto now = std::chrono::steady_clock::now();
			double framePhysicsDelta = std::chrono::duration<double>(now - LastPhysicsTime).count();
			LastPhysicsTime = now;

			if (DoPhysics)
			{
				physicsAccumulator += framePhysicsDelta;
			}

			// Run fixed update as many times as needed
			while (physicsAccumulator >= PhysicsTick)
			{
				for (size_t idx = 0; idx < SceneRenderQueue->size(); idx++)
				{
					if(ApplyGravity)
						for (size_t idx2 = 0; idx2 < SceneRenderQueue->size(); idx2++)
						{
							Ray down(RenderQueue[idx]->Transform.GetWorldPosition(), Vec3(0, -1, 0));
							RaycastHit Out;
							if (RaycastMesh(down, RenderQueue[idx]->mesh->Triangles, &Out))
							{
								Floor = RenderQueue[idx];
								FloorHit = Out;
							}
						}

					if (RenderQueue[idx]->collider.PhysicsEnabled)
					{
						Physics::Integrate(&((RenderQueue[idx])->collider), PhysicsTick, Floor, &FloorHit);
					}
				}

				physicsAccumulator -= PhysicsTick;
			}

			for (Renderable* Obj : Roots)
			{
				Obj->Transform.WalkTransformChain();
			}


			CurrScene->MainCamera->Transform.WalkTransformChain();

			Renderer::RenderScene(CurrScene->MainCamera, RenderQueue, LightsToRender, SceneRenderQueue->size(), SceneLights->size(), CurrScene->SkyboxToRender);

			delete[] LightsToRender;
			delete[] RenderQueue;

			if (Core::FpsEngineCounter)
			{
				uint64_t CpuFrameDelta = Core::GetCpuUsageInfo() - CpuFrameStart;
				Renderer::DrawFpsCounter(Wnd, Fps, CurrMB, ElapsedTime, Core::CalculateCpuUsage(CpuFrameDelta * 1e-9, ElapsedTime, Core::CpuCores));
			}

			CurrScene->DrawSceneGUI(Renderer::EngineGdi);

			// Draw to screen
			Renderer::EngineGdi->DrawDoubleBufferPO();
			//EngineGdi.DrawDoubleBuffer();

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
				CurrMB = Core::GetUsedMemory();
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
		Wnd.Destroy();
		Core::EngineCleanup();
	}
}