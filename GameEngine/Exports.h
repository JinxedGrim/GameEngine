#pragma once
#ifdef TPGE_EXPORTS
#define TPGE_API extern "C" __declspec(dllexport)
#else
#define TPGE_API 
#endif
#include "ExampleScene.h"

TPGE_API void CreateDefaultWindow()
{
	BrushPP ClearBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	WndCreatorW Wnd = WndCreatorW(CS_OWNDC, L"GameEngine", L"Game Engine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, (DWORD)WndExModes::BorderLessEx, (DWORD)WndModes::BorderLess | (DWORD)WndModes::ClipChildren, 0, 0, TerraPGE::Renderer::sx, TerraPGE::Renderer::sy);
}


//TPGE_API void ChangeCameraPosition(Scene* scene, float x, float y, float z)
//{
//	if (scene && scene->MainCamera)
//	{
//		scene->MainCamera->SetPosition(x, y, z);
//	}
//}


// need a way to provide a scene to the engine from the outside, maybe a function pointer or a scene factory
// maybe serialized scene?
TPGE_API void RunEngine(HWND WndHnd)
{
	WndCreatorW Wnd = WndCreatorW(WndHnd);

#ifdef _DEBUG
	TerraPGE::Renderer::RenderingUtils::OpenConsole();
#endif

	ExampleScene* ExScene = new ExampleScene();

	TerraPGE::Run(Wnd, ExScene);
}