#pragma once
#define NOMINMAX
#include "Renderer.h"
#include "Scene.h"
#include "Physics.h"
#include "RayCaster.h"

// The entire premise of this project is to build a game engine without using any libraries, but the std library and the WinAPI. 
// GdiPP is more or less straight WinAPI, but you do have to link with a dll which is sort of cheating but it's about the only way to do graphics without any libs like DX or OpenGl
// Direct2D and DirectWrite are justified the same way

//     TO DO 
// 1. Fix the shadows
// 2. Fix Lighting
// 3. Add skyboxes 
// 3. Caclulate Vertex norms for all 3 vertices and store them
// 4. Better reosource management
// X. Audio system 
// X. Voice 
// X. SpotLights
// X. Ambient occlusion 
// X. Global illumination
// X. FXAA, TSAA, Etc
// X. Proper UI system
// X. Probably migrate to unicode

namespace TerraPGE
{
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


	void DrawFpsCounter(WndCreator& Wnd, const SIZE_T CurrMB)
	{
#ifdef UNICODE
		// Draw FPS and some debug info
		std::wstring Str = Core::FpsWStr + std::to_wstring(Fps) + L" Memory Usage: " + std::to_wstring(CurrMB) + L"/" + std::to_wstring(Core::MaxMemoryMB) + L" MB " + std::to_wstring(Core::GetUsedHeap()) + L"MB (Heap)" + (Core::DoMultiThreading ? L"(MultiThreaded)" : L"");
		Wnd.SetWndTitle(Str);
		Renderer::EngineGdi->DrawStringW(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
		std::string Str = FpsStr + std::to_string(Fps) + " Memory Usage: " + std::to_string(CurrMB) + "/" + std::to_string(Core::MaxMemoryMB) + " MB " + (DoMultiThreading ? "(MultiThreaded)" : "");;
		Wnd.SetWndTitle(Str);
		EngineGdi.DrawStringA(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
	}


	void UpdateLoadingScreen()
	{
		Renderer::ClearScreen();
		CurrScene->DrawLoadingScreen(Renderer::EngineGdi);
		Renderer::EngineGdi->DrawDoubleBufferPO();
	}


	//TerraGL (Proposed name for GdiPP)
	//TerraPGE (Proposed name for this engine)
	//Engine Pipeline (In a Loop): Check Window State -> Check Inputs -> Clear Screen -> Make Draw Calls (Rendering Pipeline) -> Draw Double Buffer
	void Run(WndCreator& Wnd, Scene* FirstScene)
	{
		// Counters
		MSG msg = { 0 };
		double ElapsedTime = 0.0f;
		double physicsAccumulator = 0.0;
		std::vector<Renderable*> ToRender;
		std::vector<LightObject*> Lights;
		CurrScene = FirstScene;

		Wnd.RegisterRawInput();
		Renderer::PrepareRenderingBackend(Wnd);
		Core::UpdateWindow(Wnd, &msg);
		UpdateLoadingScreen();
		CurrScene->BeginScene(Wnd);
		
		auto FrameStart = std::chrono::system_clock::now();
		auto LastPhysicsTime = std::chrono::system_clock::now();

		while (!Wnd.Input.IsKeyPressed(VK_RETURN))
		{
			FrameStart = std::chrono::system_clock::now();

			Core::UpdateWindow(Wnd, &msg);

			Renderer::ClearScreen();
			ToRender.clear();
			Lights.clear();

			// call draw code
			CurrScene->RunTick(Renderer::EngineGdi, Wnd, (float)ElapsedTime, &ToRender, &Lights);

			LightObject** LightsToRender = DEBUG_NEW LightObject * [Lights.size()];
			Renderable** ObjectsToRender = DEBUG_NEW Renderable * [ToRender.size()];
			Renderable* Floor = nullptr;
			RaycastHit FloorHit;

			for (size_t idx = 0; idx < Lights.size(); idx++)
			{
				LightsToRender[idx] = Lights.at(idx);
				LightsToRender[idx]->CalcVpMats();
			}

			std::vector<Renderable*> Roots;

			for (size_t idx = 0; idx < ToRender.size(); idx++)
			{
				ObjectsToRender[idx] = ToRender.at(idx);
				if (ObjectsToRender[idx]->Transform.Parent == nullptr)
				{
					Roots.push_back(ObjectsToRender[idx]);
				}
			}

			auto now = std::chrono::system_clock::now();
			double framePhysicsDelta = std::chrono::duration<double>(now - LastPhysicsTime).count();
			LastPhysicsTime = now;

			if (DoPhysics)
			{
				physicsAccumulator += framePhysicsDelta;
			}

			// Run fixed update as many times as needed
			while (physicsAccumulator >= PhysicsTick)
			{
				for (size_t idx = 0; idx < ToRender.size(); idx++)
				{
					if(ApplyGravity)
						for (size_t idx2 = 0; idx2 < ToRender.size(); idx2++)
						{
							Ray down(ObjectsToRender[idx]->Transform.GetWorldPosition(), Vec3(0, -1, 0));
							RaycastHit Out;
							if (RaycastMesh(down, ObjectsToRender[idx]->mesh->Triangles, &Out))
							{
								Floor = ObjectsToRender[idx];
								FloorHit = Out;
							}
						}

					if (ObjectsToRender[idx]->collider.PhysicsEnabled)
					{
						Physics::Integrate(&((ObjectsToRender[idx])->collider), PhysicsTick, Floor, &FloorHit);
					}
				}

				physicsAccumulator -= PhysicsTick;
			}

			for (Renderable* Obj : Roots)
			{
				Obj->Transform.WalkTransformChain();
			}

			CurrScene->MainCamera->Transform.WalkTransformChain();
			CurrScene->MainCamera->CalcCamViewMatrix();

			Renderer::RenderShadowMaps(ObjectsToRender, LightsToRender, ToRender.size(), Lights.size(), Core::ShadowMap);
			//Renderer::RenderDepthMap(ObjectsToRender, ToRender.size(), Core::DepthBuffer, CurrScene->MainCamera->ViewMatrix);

			Renderer::RenderMeshes(CurrScene->MainCamera, ObjectsToRender, LightsToRender, ToRender.size(), Lights.size());

			delete[] LightsToRender;
			delete[] ObjectsToRender;


			if (Core::FpsEngineCounter)
			{
				DrawFpsCounter(Wnd, CurrMB);
			}

			CurrScene->DrawSceneGUI(Renderer::EngineGdi);

			// Draw to screen
			Renderer::EngineGdi->DrawDoubleBufferPO();
			//EngineGdi.DrawDoubleBuffer();

			// Calc elapsed time
			ElapsedTime = std::chrono::duration<double>(std::chrono::system_clock::now() - FrameStart).count();

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
		}

		CurrScene->EndScene();

		Wnd.Destroy();

		Core::EngineCleanup();
	}
}