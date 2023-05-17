#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include "GameEngine.h"

static float FOV_ = 90.0f;
static float FNEAR = 0.1f;
static float FFAR = 1000.f;
static float RotSpeedX = 0.0f;
static float RotSpeedY = 30.0f;
static float RotSpeedZ = 0.0f;
static bool ShowStrs = false;
static Mesh TeaPot = Mesh(("TeaPot.obj"));
static Mesh Pyramid = Mesh("Pyramid.obj");
static Mesh Axis = Mesh("Axis.obj");
static Mesh Wrld = Mesh("World.obj");
static Material LightMat = Material(Vec3(255, 255, 255), Vec3(255, 255, 255), Vec3(255, 255, 255), Vec3(255, 255, 255), 0.0f);
static Material CubeMat = Material(Vec3(255, 0, 0), Vec3(255, 0, 0), Vec3(255, 0, 0), Vec3(255, 0, 0), 0.0f);
static Mesh LightSrcMesh = Mesh(CubeMesh, LightMat);
static std::vector<Mesh> Meshes = { CubeMesh, TeaPot, Axis, Wrld};
static int CurrMesh = 0;
static bool IsFullScreen = true;
static Camera Cam = Camera(Vec3(0, 0, 0), (float)((float)Engine::sy / (float)Engine::sx), FOV_, FNEAR, FFAR);

void Settings()
{
	while (true)
	{
		if (GetAsyncKeyState(VK_F1))
		{
			Engine::FpsEngineCounter = !Engine::FpsEngineCounter;
		}
		if (GetAsyncKeyState(VK_F2))
		{
			Cam.Fov += 30;
			if (FOV_ >= 120)
			{
				Cam.Fov = 60;
			}
		}
		if (GetAsyncKeyState(VK_F3))
		{
			RotSpeedX += 15.0f;
			if (RotSpeedX >= 45.0f)
			{
				RotSpeedX = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F4))
		{
			RotSpeedY += 15.0f;
			if (RotSpeedY >= 45.0f)
			{
				RotSpeedY = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F5))
		{
			RotSpeedZ += 15.0f;
			if (RotSpeedZ >= 45.0f)
			{
				RotSpeedZ = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F6))
		{
			Engine::DoCull = !Engine::DoCull;
		}
		if (GetAsyncKeyState(VK_F7))
		{
			Engine::DoLighting = !Engine::DoLighting;
		}
		if (GetAsyncKeyState(VK_F8))
		{
			Engine::WireFrame = !Engine::WireFrame;
		}
		if (GetAsyncKeyState(VK_F9))
		{
			Engine::ShowTriLines = !Engine::ShowTriLines;
		}
		if (GetAsyncKeyState(VK_F11))
		{
			Engine::DebugClip = !Engine::DebugClip;
		}
		if (GetAsyncKeyState(VK_F10))
		{
			ShowStrs = !ShowStrs;
		}
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			if (CurrMesh < Meshes.size() - 1)
			{
				CurrMesh++;
			}
			else
			{
				CurrMesh = 0;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
	}
}

static float FTheta = 0;
static PenPP WhitePen = PenPP(PS_SOLID, 2, RGB(255, 255, 255));
static PenPP GreenPen = PenPP(PS_SOLID, 2, RGB(0, 255, 0));
static BrushPP WhiteBrush = BrushPP(RGB(255, 255, 255));
static BrushPP RedBrush = BrushPP(RGB(255, 0, 0));
static PenPP BlackPen = PenPP(PS_SOLID, 2, RGB(1, 1, 1));
static Matrix RotX;
static Matrix RotZ;
static Matrix RotY;
static Vec3 LightSrc = { 0, 0, -1 };

DoTick_T Draw(GdiPP& Gdi, WndCreatorW& Wnd, const float ElapsedTime)
{
	Cam.ViewAngles += Vec3(Engine::DeltaMouse.y, 0, 0);
	
	Cam.ViewAngles += Vec3(0, Engine::DeltaMouse.x, 0);

	if (GetAsyncKeyState(VK_INSERT))
	{
		if (IsFullScreen)
		{
			Wnd.ResetStyle(WndModes::Windowed);
			Wnd.ResetStyleEx(WndExModes::WindowedEx);
			IsFullScreen = !IsFullScreen;
		}
		else
		{
			Wnd.ResetStyle(WndModes::FullScreen);
			Wnd.ResetStyleEx(WndExModes::FullScreenEx);
			IsFullScreen = !IsFullScreen;
		}
		Engine::UpdateScreenInfo(Gdi);
		Cam.AspectRatio = (float)((float)Engine::sy / (float)Engine::sx);
	}

	if (GetAsyncKeyState(VK_DELETE))
	{
		Engine::LockCursor = !Engine::LockCursor;
		Engine::CursorShow = !Engine::CursorShow;
		Engine::UpdateMouseIn = !Engine::UpdateMouseIn;
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 1, 0) * Cam.CamRotation)) * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_LSHIFT))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, -1, 0) * Cam.CamRotation)) * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_A))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_D))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(-1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_W))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, 1) * Cam.CamRotation)) * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_S))
	{
		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, -1) * Cam.CamRotation)) * ElapsedTime;
	}

	Cam.ViewAngles.x = std::clamp<float>(Cam.ViewAngles.x, -89.0f, 89.0f);

	Cam.CamRotation = Matrix::CreateRotationMatrix({ Cam.ViewAngles.x, Cam.ViewAngles.y, Cam.ViewAngles.z }); // Pitch Yaw Roll
	Cam.LookDir = Cam.InitialLook * Cam.CamRotation;
	Vec3 T = Cam.Pos + Cam.LookDir;
	Cam.CalcCamViewMatrix(T);

	FTheta += 1.0f * ElapsedTime;

	SimpleLightSrc sl = SimpleLightSrc(LightSrc, LightSrc, Vec3(255, 255, 255), 0.35f, 0.5f, 0.5f, LightSrcMesh);

	Engine::RenderMesh(Gdi, Cam, Meshes.at(CurrMesh), Vec3(1.0f, 1.0f, 1.0f), Vec3(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ), Vec3(1, 1, 10), sl.LightPos, sl.LightDir, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff);	//Engine::RenderRenderable(Gdi, Cam, lightsrcrend, LightSrc, DoCull, DoLighting, ShowTriLines, WireFrame);

	//Engine::RenderMesh(Gdi, Cam, sl.LightMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), sl.LightPos, sl.LightPos, sl.LightDir, sl.Color, 1.0f, 0.f, 0.f);	//Engine::RenderRenderable(Gdi, Cam, lightsrcrend, LightSrc, DoCull, DoLighting, ShowTriLines, WireFrame);

	if (Engine::FpsEngineCounter && ShowStrs)
	{
		static std::string FovStr = "(F2)  Fov: ";
		static std::string YawRotStr = "(F3)  Yaw Rotation Speed: ";
		static std::string PitchRotStr = "(F4)  Pitch Rotation Speed: ";
		static std::string RollRotStr = "(F5)  Roll Rotation Speed: ";
		static std::string CullingStr = "(F6)  Culling: ";
		static std::string LightingStr = "(F7)  Lighting: ";
		static std::string FilledStr = "(F8)  Filled: ";
		static std::string TriLinesStr = "(F9) Show Tri Lines: ";
		static std::string MeshStr = "(ESC) Mesh: ";
		static std::string VertStr = " Verts: ";
		static std::string TriCountStr = ", Triangle Count: ";
		static std::string MaterialNameStr = " MatName: ";
		static std::string CamPosXstr = "Camera Pos: ( X: ";
		static std::string CamPosYstr = ", Y: ";
		static std::string CamPosZstr = ", Z: ";
		static std::string CamPosEndstr = ")";
		static std::string CamRotXstr = "Camera Rot: ( Yaw: ";
		static std::string CamRotYstr = ", Pitch: ";
		static std::string CamRotZstr = ", Roll: ";
		static std::string CamRotEndstr = ")";

		Gdi.DrawStringA(20, 40, FovStr + std::to_string(FOV_), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 60, YawRotStr + std::to_string(RotSpeedX), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 80, PitchRotStr + std::to_string(RotSpeedY), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 100, RollRotStr + std::to_string(RotSpeedZ), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 120, CullingStr + std::to_string(Engine::DoCull), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 140, LightingStr + std::to_string(Engine::DoLighting), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 160, FilledStr + std::to_string(Engine::WireFrame), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 180, TriLinesStr + std::to_string(Engine::ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 200, MeshStr + Meshes.at(CurrMesh).MeshName + VertStr + std::to_string(Meshes.at(CurrMesh).VertexCount) + TriCountStr + std::to_string(Meshes.at(CurrMesh).TriangleCount) + MaterialNameStr + Meshes.at(CurrMesh).Mat.MaterialName, RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 220, CamPosXstr + std::to_string(Cam.Pos.x) + CamPosYstr + std::to_string(Cam.Pos.y) + CamPosZstr + std::to_string(Cam.Pos.z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 240, CamRotXstr + std::to_string(Cam.ViewAngles.x) + CamRotYstr + std::to_string(Cam.ViewAngles.y) + CamRotZstr + std::to_string(Cam.ViewAngles.z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
	}

	return NULL;
}

int main()
{
	WndCreatorA Console = GetConsoleWindow();

	Console.Hide();

	BrushPP ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

	WndCreator Wnd = WndCreator(CS_CLASSDC, L"GameEngine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, WndExModes::BorderLessEx, WndModes::BorderLess, 0, 0, Engine::sx, Engine::sy);

	GdiPP Gdi = GdiPP(Wnd.Wnd, true);

	std::thread Sett(Settings);

	Engine::Run(Wnd, Gdi, ClearBrush, (DoTick_T)Draw);

	Sett.detach();

	Console.Show();

#ifdef _DEBUG
	system("pause");
#endif
}
