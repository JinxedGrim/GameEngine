#pragma once
#include "Renderable.h"

namespace TerraPGE
{
	typedef void(__fastcall* BeginScene_T)(WndCreator&);
	typedef void(__fastcall* DoTick_T)(GdiPP*, WndCreator&, const float&, std::vector<Renderable*>*, std::vector<LightObject*>*);
	typedef void(__fastcall* EndScene_T)();


	class Scene
	{
		std::vector<TerraPGE::Renderable*> RenderQueue;
		std::vector<LightObject*> Lights;
		std::vector<TerraPGE::Renderable*> Roots;

	public:
		Camera* MainCamera;

		Scene()
		{

		}

		void AddToRenderQueue(Renderable* Object)
		{
			this->RenderQueue.push_back(Object);
		}


		void ClearRenderQueue()
		{
			this->RenderQueue.clear();
		}


		void AddLight(LightObject* Object)
		{
			this->Lights.push_back(Object);
		}


		void ClearLights()
		{
			this->Lights.clear();
		}


		// certainly a proper way is to copy TODO
		const std::vector<Renderable*>* GetObjects()
		{
			return &this->RenderQueue;
		}

		// certainly a proper way is to copy TODO
		const std::vector<LightObject*>* GetLights()
		{
			return &this->Lights;
		}

		const std::vector <Renderable*> GetRoots() 
		{
			std::vector<Renderable*> Roots;

			for (Renderable* Obj : RenderQueue)
			{
				if (Obj->Transform.Parent == nullptr)
				{
					Roots.push_back(Obj);
				}
			}

			return Roots;
		}


		virtual void BeginScene(WndCreator&) = 0;
		virtual void RunTick(GdiPP*, WndCreator&, const float&) = 0;
		virtual void DrawSceneGUI(GdiPP* Gdi) = 0;
		virtual void DrawLoadingScreen(GdiPP* Gdi) = 0;
		virtual void EndScene() = 0;
	};
}