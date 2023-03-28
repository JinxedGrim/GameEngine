#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include "GameEngine.h"

float FOV_ = 90.f;
float FNEAR = 0.1f;
float FFAR = 1000.f;
float ZTRANSLATE = 8;
float RotSpeedX = 0.0f;
float RotSpeedY = 0.35;
float RotSpeedZ = 0.0f;
bool DoCull = true;
bool DoLighting = true;
bool WireFrame = false;
bool ShowTriLines = false;
bool ShowStrs = false;
Mesh TeaPot = Mesh(("TeaPot.obj"));
Mesh Pyramid = Mesh("Pyramid.obj");
std::vector<Mesh> Meshes = {Cube, TeaPot};
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
			ZTRANSLATE += 1;
			if (ZTRANSLATE > 20)
			{
				ZTRANSLATE = 0;
			}
		}
		if (GetAsyncKeyState(VK_F4))
		{
			RotSpeedX += 0.1f;
			if (RotSpeedX > 2.0f)
			{
				RotSpeedX = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F5))
		{
			RotSpeedY += 0.1f;
			if (RotSpeedY > 2.0f)
			{
				RotSpeedY = 0.f;
			}
		}
		if (GetAsyncKeyState(VK_F6))
		{
			RotSpeedZ += 0.1f;
			if (RotSpeedZ > 2.0f)
			{
				RotSpeedZ = 0;
			}
		}
		if (GetAsyncKeyState(VK_F7))
		{
			DoCull = !DoCull;
		}
		if (GetAsyncKeyState(VK_F8))
		{
			DoLighting = !DoLighting;
		}
		if (GetAsyncKeyState(VK_F9))
		{
			WireFrame = !WireFrame;
		}
		if (GetAsyncKeyState(VK_F10))
		{
			ShowTriLines = !ShowTriLines;
		}
		if (GetAsyncKeyState(VK_F11))
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

DoDraw_t Draw(GdiPP& Gdi, const float ElapsedTime)
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

	FTheta += 1.0f * ElapsedTime;

	Matrix::CreateRotationX(&RotX, FTheta * RotSpeedX);
	Matrix::CreateRotationY(&RotY, FTheta * RotSpeedY);
	Matrix::CreateRotationZ(&RotZ, FTheta * RotSpeedZ);

	Matrix RotM = Matrix::CreateRotationMatrix(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ);

	if (Engine::FpsEngineCounter && ShowStrs)
	{
		static std::string FovStr =      "(F2)  Fov: ";
		static std::string ZTransStr =   "(F3)  ZTRANSLATE: ";
		static std::string YawRotStr =   "(F4)  Yaw Rotation Speed: ";
		static std::string PitchRotStr = "(F5)  Pitch Rotation Speed: ";
		static std::string RollRotStr =  "(F6)  Roll Rotation Speed: ";
		static std::string CullingStr =  "(F7)  Culling: ";
		static std::string LightingStr = "(F8)  Lighting: ";
		static std::string FilledStr =   "(F9)  Filled: ";
		static std::string TriLinesStr = "(F10) Show Tri Lines: ";
		static std::string MeshStr = "(ESC) Mesh: ";
		static std::string VertStr = " Verts: "; 
		static std::string TriCountStr = ", Triangle Count: ";

		Gdi.DrawStringA(20, 40,  FovStr + std::to_string(FOV_), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 60,  ZTransStr + std::to_string(ZTRANSLATE), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 80,  YawRotStr + std::to_string(RotSpeedX), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 100, PitchRotStr + std::to_string(RotSpeedY), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 120, RollRotStr + std::to_string(RotSpeedZ), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 140, CullingStr + std::to_string(DoCull), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 160, LightingStr + std::to_string(DoLighting), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 180, FilledStr + std::to_string(WireFrame), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 200, TriLinesStr + std::to_string(ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
		Gdi.DrawStringA(20, 220, MeshStr + Meshes.at(CurrMesh).MeshName + VertStr + std::to_string(Meshes.at(CurrMesh).VertexCount) + TriCountStr + std::to_string(Meshes.at(CurrMesh).TriangleCount), RGB(255, 0, 0), TRANSPARENT);
	}

	std::vector<Triangle> TrisToRender = {};

	for (const auto &Tri : Meshes.at(CurrMesh).Triangles)
	{
		// 3D Space
		Triangle Proj, Rxyz;

		Rxyz = Tri;

		// Apply XYZ Rotation Matrice
		Rxyz.Rotate(RotM);

		//Translate The 3D Triangle Into Camera View
		Rxyz.Translate(Vec3(0, -0.5f, ZTRANSLATE));

		// calc surface normal
		Vec3 TriNormal = (Rxyz.Points[1] - Rxyz.Points[0]).CrossNormalized((Rxyz.Points[2] - Rxyz.Points[0])); // this line and the if statement is used for culling
		
		if((TriNormal.Dot(Rxyz.Points[0] - Cam.Pos) <= 0.0f) || !DoCull) // culling
		{
			Intensity = 1.0f;

			// Calc lighting intensity
			if (DoLighting)
				Intensity = TriNormal.Dot(LightSrc.Normalized());

			// 3d Space -> Screen Space
			Cam.ProjectTriangle(&Rxyz, Proj); // Project from 3D Space To Screen Space

			// Offset to normalized space
			Proj.Translate(Vec3(1.0f, 1.0f, 0.0f));		    
			Proj.Scale(Vec3((float)(Engine::sx * 0.5f), (float)(Engine::sy * 0.5f), 1));
			//Proj.Translate(Vec3(-1, -2, 0));

			Proj.r *= Intensity;
			Proj.g *= Intensity;
			Proj.b *= Intensity;

			TrisToRender.push_back(Proj);

			//if (!WireFrame)
			//{
			//	if (!ShowTriLines)
			//		Gdi.DrawFilledTriangle((Proj.Points[0].x), (Proj.Points[0].y), (Proj.Points[1].x), (Proj.Points[1].y), (Proj.Points[2].x), (Proj.Points[2].y), BrushPP(RGB(Proj.r * Intensity, Proj.g * Intensity, Proj.b * Intensity)), PenPP(PS_SOLID, 1, RGB(Proj.r * Intensity, Proj.g * Intensity, Proj.b * Intensity)));
			//	else
			//		Gdi.DrawFilledTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, BrushPP(RGB(Proj.r * Intensity, Proj.g * Intensity, Proj.b * Intensity)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));
			//}
			//else
			//{
			//	Gdi.DrawTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, PenPP(PS_SOLID, 1, RGB(Proj.r * Intensity, Proj.g * Intensity, Proj.b * Intensity)));
			//}
		}
	}

	std::sort(TrisToRender.begin(), TrisToRender.end(), [](Triangle& t1, Triangle& t2)
		{
			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;

			return z1 > z2;
		});

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

	Engine::Run(Wnd, Gdi, ClearBrush, (DoDraw_t)Draw);

	Sett.detach();

	Console.Show();

#ifdef _DEBUG
	system("pause");
#endif
}
