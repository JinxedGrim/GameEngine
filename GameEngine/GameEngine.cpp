#define _CRT_SECURE_NO_WARNINGS
#include "ExampleScene.h"


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(1648658); // example: break at specific leak ID
#endif

	BrushPP ClearBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	WndCreatorW Wnd = WndCreatorW(CS_OWNDC, L"GameEngine", L"Game Engine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, (DWORD)WndExModes::BorderLessEx, (DWORD)WndModes::BorderLess | (DWORD)WndModes::ClipChildren, 0, 0, TerraPGE::Renderer::sx, TerraPGE::Renderer::sy);

#ifdef _DEBUG
	TerraPGE::Renderer::RenderingUtils::OpenConsole();
#endif

	ExampleScene* ExScene = new ExampleScene();

	TerraPGE::Run(Wnd, ExScene);

	TerraPGE::Core::LogInfo("[APP]", "Exiting...");

	Wnd.Destroy();

#ifdef _DEBUG
	system("timeout \t 8");
#endif
}
