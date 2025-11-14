#pragma once
#include "Renderable.h"

namespace TerraPGE
{
	typedef void(__fastcall* BeginScene_T)(WndCreator&);
	typedef void(__fastcall* DoTick_T)(GdiPP*, WndCreator&, const float&, std::vector<Renderable*>*, std::vector<LightObject*>*);
	typedef void(__fastcall* EndScene_T)();


	class Scene
	{
	public:
		Camera* MainCamera;

		Scene()
		{

		}

		virtual void BeginScene(WndCreator&) = 0;
		virtual void RunTick(GdiPP*, WndCreator&, const float&, std::vector<Renderable*>*, std::vector<LightObject*>*) = 0;
		virtual void DrawSceneGUI(GdiPP* Gdi) = 0;
		virtual void DrawLoadingScreen(GdiPP* Gdi) = 0;
		virtual void EndScene() = 0;
	};
}