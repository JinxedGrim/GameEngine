#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include "GameEngine.h"

float FOV_ = 90.f;
float FNEAR = 0.1f;
float FFAR = 1000.f;
float RotSpeedX = 0.0f;
float RotSpeedY = 0.0f;
float RotSpeedZ = 0.0f;
bool DoCull = true;
bool DoLighting = true;
bool WireFrame = false;
bool ShowTriLines = false;
bool ShowStrs = false;
Mesh TeaPot = Mesh(("TeaPot.obj"));
Mesh Pyramid = Mesh("Pyramid.obj");
Mesh Axis = Mesh("axis.obj");
std::vector<Mesh> Meshes = {Cube, TeaPot, Axis};
int CurrMesh = 0;

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
			FOV_ += 30;
			if (FOV_ >= 120)
			{
				FOV_ = 60;
			}
		}
		if (GetAsyncKeyState(VK_F3))
		{
			RotSpeedX += 0.1f;
			if (RotSpeedX > 2.0f)
			{
				RotSpeedX = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F4))
		{
			RotSpeedY += 0.1f;
			if (RotSpeedY > 2.0f)
			{
				RotSpeedY = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F5))
		{
			RotSpeedZ += 0.1f;
			if (RotSpeedZ > 2.0f)
			{
				RotSpeedZ = 0;
			}
		}
		if (GetAsyncKeyState(VK_F6))
		{
			DoCull = !DoCull;
		}
		if (GetAsyncKeyState(VK_F7))
		{
			DoLighting = !DoLighting;
		}
		if (GetAsyncKeyState(VK_F8))
		{
			WireFrame = !WireFrame;
		}
		if (GetAsyncKeyState(VK_F9))
		{
			ShowTriLines = !ShowTriLines;
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

DoTick_T Draw(GdiPP& Gdi, const float ElapsedTime)
{
	static float FTheta = 0;
	static Camera Cam = Camera(Vec3(0, 0, 0), (float)((float)Engine::sy / (float)Engine::sx), FOV_, FNEAR, FFAR);
	static PenPP WhitePen = PenPP(PS_SOLID, 2, RGB(255, 255, 255));
	static PenPP GreenPen = PenPP(PS_SOLID, 2, RGB(0, 255, 0));
	static BrushPP WhiteBrush = BrushPP(RGB(255, 255, 255));
	static BrushPP RedBrush = BrushPP(RGB(255, 0, 0));
	static PenPP BlackPen = PenPP(PS_SOLID, 2, RGB(1, 1, 1));
	static Matrix RotX;
	static Matrix RotZ;
	static Matrix RotY;
	static Vec3 LightSrc = { 0, 0, -1 };
	static float Intensity = 1.0f;
	static Vec3 CamUp = { 0, 1, 0 };
	static Vec3 Target = {0, 0, 1};
	static Vec3 LookDir = { 0, 0, 0 };


	FTheta += 1.0f * ElapsedTime;

	Matrix RotM = Matrix::CreateRotationMatrix(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ);
	Matrix TransMat = Matrix::CreateTranslationMatrix(Vec3(1, 1, 10.0f));
	Matrix WorldMatrix = Matrix::CreateIdentity();
	WorldMatrix = (WorldMatrix * RotM) * TransMat;

	if (GetAsyncKeyState(VK_UP))
	{
		Cam.ViewAngles += Vec3(0, (20.0f * ElapsedTime), 0);
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		Cam.ViewAngles -= Vec3(0, (20.0f * ElapsedTime), 0);
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		Cam.ViewAngles += Vec3((20.0f * ElapsedTime), 0, 0);
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		Cam.ViewAngles -= Vec3((20.0f * ElapsedTime), 0, 0);
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		Cam.Pos.y += 8.0f * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_LSHIFT))
	{
		Cam.Pos.y -= 8.0f * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_A))
	{
		Cam.Pos.x -= 8.0f * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_D))
	{
		Cam.Pos.x += 8.0f * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_W))
	{
		Cam.Pos.z += 8.0f * ElapsedTime;
	}

	if (GetAsyncKeyState(VK_S))
	{
		Cam.Pos.z -= 8.0f * ElapsedTime;
	}

	if (Engine::FpsEngineCounter && ShowStrs)
	{
		static std::string FovStr =      "(F2)  Fov: ";
		static std::string YawRotStr =   "(F3)  Yaw Rotation Speed: ";
		static std::string PitchRotStr = "(F4)  Pitch Rotation Speed: ";
		static std::string RollRotStr =  "(F5)  Roll Rotation Speed: ";
		static std::string CullingStr =  "(F6)  Culling: ";
		static std::string LightingStr = "(F7)  Lighting: ";
		static std::string FilledStr =   "(F8)  Filled: ";
		static std::string TriLinesStr = "(F9) Show Tri Lines: ";
		static std::string MeshStr = "(ESC) Mesh: ";
		static std::string VertStr = " Verts: "; 
		static std::string TriCountStr = ", Triangle Count: ";
		static std::string CamRotXstr = "Camera Pos: ( X: ";
		static std::string CamRotYstr = ", Y: ";
		static std::string CamRotZstr = ", Z: ";
		static std::string CamRotEndstr = ")";
		static std::string CamPosXstr = "Camera Rot: ( Yaw: ";
		static std::string CamPosYstr = ", Pitch: ";
		static std::string CamPosZstr = ", Roll: ";
		static std::string CamPosEndstr = ")";

		Gdi.DrawStringA(20, 40,  FovStr + std::to_string(FOV_), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 60,  YawRotStr + std::to_string(RotSpeedX), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 80, PitchRotStr + std::to_string(RotSpeedY), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 100, RollRotStr + std::to_string(RotSpeedZ), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 120, CullingStr + std::to_string(DoCull), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 140, LightingStr + std::to_string(DoLighting), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 160, FilledStr + std::to_string(WireFrame), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 180, TriLinesStr + std::to_string(ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 200, MeshStr + Meshes.at(CurrMesh).MeshName + VertStr + std::to_string(Meshes.at(CurrMesh).VertexCount) + TriCountStr + std::to_string(Meshes.at(CurrMesh).TriangleCount), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 220, CamPosXstr + std::to_string(Cam.Pos.x) + CamPosYstr + std::to_string(Cam.Pos.y) + CamPosZstr + std::to_string(Cam.Pos.z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 240, CamRotXstr + std::to_string(Cam.ViewAngles.x) + CamRotYstr + std::to_string(Cam.ViewAngles.y) + CamRotZstr + std::to_string(Cam.ViewAngles.z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
	}

	Matrix CamRot = Matrix::CreateRotationMatrix(ToRad(Cam.ViewAngles.y), ToRad(Cam.ViewAngles.x), ToRad(Cam.ViewAngles.z));
	LookDir = Target * CamRot;
	Vec3 T = Cam.Pos + LookDir;
	Cam.CalcCamViewMatrix(T, CamUp);

	std::vector<Triangle> TrisToRender = {};

	// Project and translate object 
	for (const auto &Tri : Meshes.at(CurrMesh).Triangles)
	{
		// 3D Space
		Triangle Proj;

		Proj = Tri;

		Proj.ApplyMatrix(WorldMatrix);

		// calc surface normal
		Vec3 TriNormal = (Proj.Points[1] - Proj.Points[0]).CrossNormalized((Proj.Points[2] - Proj.Points[0])); // this line and the if statement is used for culling
		
		if((TriNormal.Dot(Proj.Points[0] - Cam.Pos) < 0.0f) || !DoCull) // culling
		{
			Intensity = 1.0f;

			// Calc lighting intensity
			if (DoLighting)
				Intensity = TriNormal.Dot(LightSrc.Normalized());

			// 3d Space -> Viewed Space
			Proj.ApplyMatrix(Cam.ViewMatrix);

			// 3d Space -> Screen Space
			Proj = Cam.ProjectTriangle(&Proj); // Project from 3D Space To Screen Space

			// Offset to normalized space
			Proj.Translate(Vec3(1.0f, 1.0f, 0.0f));		    
			Proj.Scale(Vec3((float)(Engine::sx * 0.5f), (float)(Engine::sy * 0.5f), 1));
			Proj.Translate(Vec3(-1,  -1, 0));

			Proj.r *= Intensity;
			Proj.g *= Intensity;
			Proj.b *= Intensity;

			TrisToRender.push_back(Proj);
		}
	}

	// sort faces 
	std::sort(TrisToRender.begin(), TrisToRender.end(), [](Triangle& t1, Triangle& t2)
		{
			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;

			return z1 > z2;
		});

	// draw
	for (const auto& Proj : TrisToRender)
	{
		if (!WireFrame)
		{
			if (!ShowTriLines)
				Gdi.DrawFilledTriangle((Proj.Points[0].x), (Proj.Points[0].y), (Proj.Points[1].x), (Proj.Points[1].y), (Proj.Points[2].x), (Proj.Points[2].y), BrushPP(RGB(Proj.r, Proj.g, Proj.b)), PenPP(PS_SOLID, 1, RGB(Proj.r, Proj.g, Proj.b)));
			else
				Gdi.DrawFilledTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, BrushPP(RGB(Proj.r, Proj.g, Proj.b)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));
		}
		else
		{
			Gdi.DrawTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, PenPP(PS_SOLID, 1, RGB(Proj.r, Proj.g, Proj.b)));
		}
	}

	return NULL;
}

int main()
{
	WndCreatorA Console = GetConsoleWindow();

	Console.Hide();

	BrushPP ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

	WndCreator Wnd = WndCreator(CS_CLASSDC, GetModuleHandle(NULL), L"GameEngine", ClearBrush, 0, WS_POPUP | WS_VISIBLE, 0, 0, Engine::sx, Engine::sy);

	GdiPP Gdi = GdiPP(Wnd.Wnd, true);

	std::thread Sett(Settings);

	Engine::Run(Wnd, Gdi, ClearBrush, (DoTick_T)Draw);

	Sett.detach();

	Console.Show();

#ifdef _DEBUG
	system("pause");
#endif
}
