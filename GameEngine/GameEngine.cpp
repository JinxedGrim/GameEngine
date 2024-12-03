#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>

#define _CRT_SECURE_NO_WARNINGS

#ifndef UseOGL
//#define SSE_SIMD_42_SUPPORT
//#include "TerraPGE.h"
#include "TerraPGE.h"
#else
// gonna add a opengl version
#endif

#ifdef _DEBUG
BOOL _ = AllocConsole();
FILE* new_stdout;
auto __ = freopen_s(&new_stdout, "CONOUT$", "w", stdout);
#endif

static float RotSpeedX = 0.0f;
static float RotSpeedY = 30.0f;
static float RotSpeedZ = 0.0f;
static bool ShowStrs = false;
static float FTheta = 0;
static int CurrMesh = 0;
static bool IsFullScreen = true;

CHAR cwd [MAX_PATH + 1] = "";
DWORD len = GetCurrentDirectoryA(MAX_PATH, cwd);

std::string CWD = cwd;

//DoTick_T Draw(GdiPP& Gdi, WndCreatorW& Wnd, const float& ElapsedTime, std::vector<Renderable>* ToRender, std::vector<LightObject*>* Lights)
//{
//	static float PrevFov = Cam.Fov;
//	
//	if (!LockCamera)
//	{
//		Cam.ViewAngles += Vec3((float)TerraPGE::DeltaMouse.y, 0, 0);
//		Cam.ViewAngles.x = std::clamp<float>(Cam.ViewAngles.x, -89.0f, 89.0f);
//		Cam.ViewAngles -= Vec3(0, (float)TerraPGE::DeltaMouse.x, 0);
//	}
//
//	if (PrevFov != Cam.Fov)
//	{
//		Cam.CalcProjectionMat();
//		PrevFov = Cam.Fov;
//	}
//
//	if (GetAsyncKeyState(VK_INSERT) & 0x8000)
//	{
//		if (IsFullScreen)
//		{
//			Wnd.ResetStyle((LONG_PTR)WndModes::Windowed);
//			Wnd.ResetStyleEx((LONG_PTR)WndExModes::WindowedEx);
//			IsFullScreen = !IsFullScreen;
//		}
//		else
//		{
//			Wnd.ResetStyle((LONG_PTR)WndModes::FullScreen);
//			Wnd.ResetStyleEx((LONG_PTR)WndExModes::FullScreenEx);
//			IsFullScreen = !IsFullScreen;
//		}
//		TerraPGE::UpdateScreenInfo(Gdi);
//		Cam.AspectRatio = (float)((float)TerraPGE::sy / (float)TerraPGE::sx);
//	}
//
//	if (GetAsyncKeyState(VK_DELETE) & 0x8000)
//	{
//		TerraPGE::LockCursor = !TerraPGE::LockCursor;
//		TerraPGE::CursorShow = !TerraPGE::CursorShow;
//		TerraPGE::UpdateMouseIn = !TerraPGE::UpdateMouseIn;
//	}
//
//	if (GetAsyncKeyState(VK_HOME))
//	{
//		LockCamera = !LockCamera;
//		Matrix Proj;
//		Proj.MakeOrthoMatrix(-40.0f, 40.0f, -40.0f, 40.0f, TerraPGE::FNEAR, TerraPGE::FFAR);
//		Cam.ProjectionMatrix = Proj;
//		Cam.ViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
//		Cam.Pos = -((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f;
//	}
//
//	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 1, 0) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, -1, 0) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_A) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(-1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_D) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_W) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, 1) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_S) & 0x8000)
//	{
//		Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, -1) * Cam.CamRotation)) * ElapsedTime;
//	}
//
//	if (!LockCamera)
//	{
//		Cam.CamRotation = Matrix::CreateRotationMatrix(Cam.ViewAngles); // Pitch Yaw Roll
//		Cam.LookDir = Cam.InitialLook * Cam.CamRotation;
//		Vec3 T = Cam.Pos + Cam.LookDir;
//		Cam.CalcCamViewMatrix(T);
//	}
//
//	FTheta += 1.0f * ElapsedTime;
//
//	//TerraPGE::RenderMesh(Gdi, Cam, RYNO, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f),  Vec3(10.0f, 2.5f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);
//	//TerraPGE::RenderMesh(Gdi, Cam, RCAMMO, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(-10.0f, 2.5f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);
//
//	// Camera shouldnt be in this class at all...
//	// TODO
//	CubeRender.Cam = &Cam;
//	PlaneRender.Cam = &Cam;
//	RCAMMO_Render.Cam = &Cam;
//
//	ToRender->push_back(CubeRender);
//	//ToRender->push_back(PlaneRender);
//	ToRender->push_back(PlaneRender);
//
//	Lights->push_back(&sl);
//
//	//TerraPGE::RenderMesh(Gdi, Cam, CubeMsh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Frag_Phong_Shadows);
//	//TerraPGE::RenderMesh(Gdi, Cam, Plane, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Frag_Phong_Shadows);
//
//	//Renderable CubeRend = Renderable(CubeMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f));
//	//Renderable PlaneRend = Renderable(Plane, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f));
//	//Renderable LightSrcRend = Renderable(sl.LightMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0));
//
////	TerraPGE::RenderMesh(Gdi, Cam, Meshes.at(CurrMesh), Vec3(1.0f, 1.0f, 1.0f), Vec3(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ), Vec3(1, 0, 10), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);
//
////	if (CurrMesh <= 2)
////		TerraPGE::RenderMesh(Gdi, Cam, Meshes.at(CurrMesh), Vec3(1.0f, 1.0f, 1.0f), Vec3(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ), Vec3(10, 0, 10), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Gradient_Centroid, SHADER_FRAGMENT);
//
//	//TerraPGE::RenderMesh(Gdi, Cam, sl.LightMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec3(0, 0, 0), Lights->data(), Lights->size(), EngineShaders::Shader_Material, ShaderTypes::SHADER_TRIANGLE);
//
//	if (TerraPGE::FpsEngineCounter && ShowStrs)
//	{
//		static std::string FovStr = "(F2)  Fov: ";
//		static std::string YawRotStr = "(F3)  Yaw Rotation Speed: ";
//		static std::string PitchRotStr = "(F4)  Pitch Rotation Speed: ";
//		static std::string RollRotStr = "(F5)  Roll Rotation Speed: ";
//		static std::string CullingStr = "(F6)  Culling: ";
//		static std::string LightingStr = "(F7)  Lighting: ";
//		static std::string FilledStr = "(F8)  Filled: ";
//		static std::string TriLinesStr = "(F9) Show Tri Lines: ";
//		static std::string MeshStr = "(ESC) Mesh: ";
//		static std::string VertStr = " Verts: ";
//		static std::string TriCountStr = ", Triangle Count: ";
//		static std::string NormalCountStr = ", Normal Count: ";
//		static std::string MaterialNameStr = " MatName: ";
//		static std::string CamPosXstr = "Camera Pos: ( X: ";
//		static std::string CamPosYstr = ", Y: ";
//		static std::string CamPosZstr = ", Z: ";
//		static std::string CamPosEndstr = ")";
//		static std::string CamRotXstr = "Camera Rot: ( Yaw: ";
//		static std::string CamRotYstr = ", Pitch: ";
//		static std::string CamRotZstr = ", Roll: ";
//		static std::string CamRotEndstr = ")";
//		static std::string ResourceInfoStr = "LoadedMaterials: ";
//		static std::string ResourceInfoEndStr = "";
//		std::stringstream ss;
//		//ss << " : 0x"; << std::hex << &Meshes.at(CurrMesh).Mat << std::dec;
//		std::string MatPtrStr = ss.str();
//
//		Gdi.DrawStringA(20, 40, FovStr + std::to_string(TerraPGE::FOV), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 60, YawRotStr + std::to_string(RotSpeedX), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 80, PitchRotStr + std::to_string(RotSpeedY), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 100, RollRotStr + std::to_string(RotSpeedZ), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 120, CullingStr + std::to_string(TerraPGE::DoCull), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 140, LightingStr + std::to_string(TerraPGE::DoLighting), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 160, FilledStr + std::to_string(TerraPGE::WireFrame), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 180, TriLinesStr + std::to_string(TerraPGE::ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
//		//Gdi.DrawStringA(20, 200, MeshStr + Meshes.at(CurrMesh).MeshName + VertStr + std::to_string(Meshes.at(CurrMesh).VertexCount) + TriCountStr + std::to_string(Meshes.at(CurrMesh).TriangleCount) + NormalCountStr + std::to_string(Meshes.at(CurrMesh).NormalCount) + MaterialNameStr + Meshes.at(CurrMesh).Mat.MaterialName + MatPtrStr, RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 220, ResourceInfoStr + std::to_string(Material::LoadedMaterials.size()), RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 240, CamPosXstr + std::to_string(Cam.Pos.x) + CamPosYstr + std::to_string(Cam.Pos.y) + CamPosZstr + std::to_string(Cam.Pos.z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 260, CamRotXstr + std::to_string(Cam.ViewAngles.x) + CamRotYstr + std::to_string(Cam.ViewAngles.y) + CamRotZstr + std::to_string(Cam.ViewAngles.z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
//	}
//
//	return NULL;
//}

class ExampleScene : public Scene
{
	Mesh* Ryno = nullptr;
	Mesh* RCAMMO = nullptr;
	Mesh* AK47 = nullptr;
	Mesh* Plane = nullptr;
	Mesh* CubeMsh = nullptr;
	Vec3 LightSrcPos = Vec3();
	DirectionalLight sl;
	Renderable* CubeRender;
	Renderable* PlaneRender;
	Renderable* RCAMMO_Render;
	bool LockCamera = false;

	public:
	Camera Cam;
	Texture* Txt = nullptr;

	ExampleScene()
	{

	}

	void BeginScene() override
	{
		this->Txt = DEBUG_NEW Texture("Test.bmp");
		//this->Ryno = DEBUG_NEW Mesh("RYNO.obj");
		//this->RCAMMO = DEBUG_NEW Mesh("RC_AMMO.obj");
		//this->AK47 = DEBUG_NEW Mesh("AK47.obj");
		this->Plane = DEBUG_NEW Mesh("FlatTerrain.obj");
		this->CubeMsh = DEBUG_NEW Cube(1, 1, 1, DEBUG_NEW Material(), Txt);

		//static Mesh Mountains = Mesh("Mountains.obj");
		//static Mesh TeaPot = Mesh(("TeaPot.obj"));
		//static Mesh Axis = Mesh("Axis.obj");
		//static Mesh Spyro = Mesh("Spyro.obj");
		//static Mesh DragStat = Mesh("DragonStatue.obj");
		//static std::vector<Mesh> Meshes = {};//{Cube(1, 1, 1, Material(), &Txt), RYNO, Sphere(1, 20, 20, TeaPot.Mat), TeaPot, Axis, AK47, Spyro, Mountains, DragStat };
		this->LightSrcPos = { 0, 30, -34 };
		sl = DirectionalLight(LightSrcPos, { 0, 10, 0 }, Vec3(253, 251, 211), 0.35f, 0.15f, 0.5f);
		Cam = Camera(Vec3(0, 1, 0), (float)((float)TerraPGE::sy / (float)TerraPGE::sx), TerraPGE::FOV, TerraPGE::FNEAR, TerraPGE::FFAR);
		CubeRender = DEBUG_NEW Renderable(CubeMsh, &Cam, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f), EngineShaders::Shader_Texture_Only);
		PlaneRender = DEBUG_NEW Renderable(Plane, &Cam, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), EngineShaders::Shader_Frag_Phong_Shadows);
		//RCAMMO_Render = DEBUG_NEW Renderable(RCAMMO, &Cam, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), EngineShaders::Shader_Frag_Phong_Shadows);

		LockCamera = false;
	}

	void RunTick(GdiPP& Gdi, WndCreatorW& Wnd, const float& ElapsedTime, std::vector<Renderable*>* ToRender, std::vector<LightObject*>* Lights) override
	{
		static float PrevFov = Cam.Fov;

		if (!LockCamera)
		{
			Cam.ViewAngles += Vec3((float)TerraPGE::DeltaMouse.y, 0, 0);
			Cam.ViewAngles.x = std::clamp<float>(Cam.ViewAngles.x, -89.0f, 89.0f);
			Cam.ViewAngles -= Vec3(0, (float)TerraPGE::DeltaMouse.x, 0);
		}

		if (PrevFov != Cam.Fov)
		{
			Cam.CalcProjectionMat();
			PrevFov = Cam.Fov;
		}

		if (GetAsyncKeyState(VK_INSERT) & 0x8000)
		{
			if (IsFullScreen)
			{
				Wnd.ResetStyle((LONG_PTR)WndModes::Windowed);
				Wnd.ResetStyleEx((LONG_PTR)WndExModes::WindowedEx);
				IsFullScreen = !IsFullScreen;
			}
			else
			{
				Wnd.ResetStyle((LONG_PTR)WndModes::FullScreen);
				Wnd.ResetStyleEx((LONG_PTR)WndExModes::FullScreenEx);
				IsFullScreen = !IsFullScreen;
			}
			TerraPGE::UpdateScreenInfo(Gdi);
			Cam.AspectRatio = (float)((float)TerraPGE::sy / (float)TerraPGE::sx);
		}

		if (GetAsyncKeyState(VK_DELETE) & 0x8000)
		{
			TerraPGE::LockCursor = !TerraPGE::LockCursor;
			TerraPGE::CursorShow = !TerraPGE::CursorShow;
			TerraPGE::UpdateMouseIn = !TerraPGE::UpdateMouseIn;
		}

		if (GetAsyncKeyState(VK_HOME))
		{
			LockCamera = !LockCamera;
			Matrix Proj;
			Proj.MakeOrthoMatrix(-40.0f, 40.0f, -40.0f, 40.0f, TerraPGE::FNEAR, TerraPGE::FFAR);
			Cam.ProjectionMatrix = Proj;
			Cam.ViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
			Cam.Pos = -((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f;
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 1, 0) * Cam.CamRotation)) * ElapsedTime;
		}

		if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(0, -1, 0) * Cam.CamRotation)) * ElapsedTime;
		}

		if (GetAsyncKeyState(VK_A) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(-1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
		}

		if (GetAsyncKeyState(VK_D) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(1, 0, 0) * Cam.CamRotation)) * ElapsedTime;
		}

		if (GetAsyncKeyState(VK_W) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, 1) * Cam.CamRotation)) * ElapsedTime;
		}

		if (GetAsyncKeyState(VK_S) & 0x8000)
		{
			Cam.Pos += (Cam.GetNewVelocity(Vec3(0, 0, -1) * Cam.CamRotation)) * ElapsedTime;
		}

		if (!LockCamera)
		{
			Cam.CamRotation = Matrix::CreateRotationMatrix(Cam.ViewAngles); // Pitch Yaw Roll
			Cam.LookDir = Cam.InitialLook * Cam.CamRotation;
			Vec3 T = Cam.Pos + Cam.LookDir;
			Cam.CalcCamViewMatrix(T);
		}

		FTheta += 1.0f * ElapsedTime;

		//TerraPGE::RenderMesh(Gdi, Cam, RYNO, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f),  Vec3(10.0f, 2.5f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);
		//TerraPGE::RenderMesh(Gdi, Cam, RCAMMO, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(-10.0f, 2.5f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);

		// Camera shouldnt be in this class at all...
		// TODO
		CubeRender->Cam = &Cam;
		PlaneRender->Cam = &Cam;

		ToRender->push_back(CubeRender);
		ToRender->push_back(PlaneRender);

		Lights->push_back(&sl);

		//TerraPGE::RenderMesh(Gdi, Cam, CubeMsh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Frag_Phong_Shadows);
		//TerraPGE::RenderMesh(Gdi, Cam, Plane, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Frag_Phong_Shadows);

		//Renderable CubeRend = Renderable(CubeMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f));
		//Renderable PlaneRend = Renderable(Plane, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f));
		//Renderable LightSrcRend = Renderable(sl.LightMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0));

	//	TerraPGE::RenderMesh(Gdi, Cam, Meshes.at(CurrMesh), Vec3(1.0f, 1.0f, 1.0f), Vec3(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ), Vec3(1, 0, 10), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Texture_Only);

	//	if (CurrMesh <= 2)
	//		TerraPGE::RenderMesh(Gdi, Cam, Meshes.at(CurrMesh), Vec3(1.0f, 1.0f, 1.0f), Vec3(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ), Vec3(10, 0, 10), sl.LightPos, sl.Color, sl.AmbientCoeff, sl.DiffuseCoeff, sl.SpecularCoeff, EngineShaders::Shader_Gradient_Centroid, SHADER_FRAGMENT);

		//TerraPGE::RenderMesh(Gdi, Cam, sl.LightMesh, Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Vec3(0, 0, 0), Lights->data(), Lights->size(), EngineShaders::Shader_Material, ShaderTypes::SHADER_TRIANGLE);

		if (TerraPGE::FpsEngineCounter && ShowStrs)
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
			static std::string NormalCountStr = ", Normal Count: ";
			static std::string MaterialNameStr = " MatName: ";
			static std::string CamPosXstr = "Camera Pos: ( X: ";
			static std::string CamPosYstr = ", Y: ";
			static std::string CamPosZstr = ", Z: ";
			static std::string CamPosEndstr = ")";
			static std::string CamRotXstr = "Camera Rot: ( Yaw: ";
			static std::string CamRotYstr = ", Pitch: ";
			static std::string CamRotZstr = ", Roll: ";
			static std::string CamRotEndstr = ")";
			static std::string ResourceInfoStr = "LoadedMaterials: ";
			static std::string ResourceInfoEndStr = "";
			std::stringstream ss;
			//ss << " : 0x"; << std::hex << &Meshes.at(CurrMesh).Mat << std::dec;
			std::string MatPtrStr = ss.str();

			Gdi.DrawStringA(20, 40, FovStr + std::to_string(TerraPGE::FOV), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 60, YawRotStr + std::to_string(RotSpeedX), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 80, PitchRotStr + std::to_string(RotSpeedY), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 100, RollRotStr + std::to_string(RotSpeedZ), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 120, CullingStr + std::to_string(TerraPGE::DoCull), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 140, LightingStr + std::to_string(TerraPGE::DoLighting), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 160, FilledStr + std::to_string(TerraPGE::WireFrame), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 180, TriLinesStr + std::to_string(TerraPGE::ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
			//Gdi.DrawStringA(20, 200, MeshStr + Meshes.at(CurrMesh).MeshName + VertStr + std::to_string(Meshes.at(CurrMesh).VertexCount) + TriCountStr + std::to_string(Meshes.at(CurrMesh).TriangleCount) + NormalCountStr + std::to_string(Meshes.at(CurrMesh).NormalCount) + MaterialNameStr + Meshes.at(CurrMesh).Mat.MaterialName + MatPtrStr, RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 220, ResourceInfoStr + std::to_string(Material::LoadedMaterials.size()), RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 240, CamPosXstr + std::to_string(Cam.Pos.x) + CamPosYstr + std::to_string(Cam.Pos.y) + CamPosZstr + std::to_string(Cam.Pos.z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
			Gdi.DrawStringA(20, 260, CamRotXstr + std::to_string(Cam.ViewAngles.x) + CamRotYstr + std::to_string(Cam.ViewAngles.y) + CamRotZstr + std::to_string(Cam.ViewAngles.z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
		}
	}

	void DrawGUI()
	{

	}

	void EndScene() override
	{
		delete this->Ryno;
		delete this->CubeMsh;
		delete this->CubeRender;
		delete this->Plane;
		delete this->PlaneRender;
		delete this->RCAMMO;
		this->Txt->Delete();
	}

	void Settings()
	{
		while (true)
		{
			if (GetAsyncKeyState(VK_F1))
			{
				TerraPGE::FpsEngineCounter = !TerraPGE::FpsEngineCounter;
			}
			if (GetAsyncKeyState(VK_F2))
			{
				Cam.Fov += 30;
				TerraPGE::FOV += 30;
				if (TerraPGE::FOV > 120)
				{
					Cam.Fov = 60;
					TerraPGE::FOV = 60;
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
				TerraPGE::DoMultiThreading = !TerraPGE::DoMultiThreading;
			}
			if (GetAsyncKeyState(VK_F6))
			{
				static bool ca = false;

				if (!ca)
					Txt->Delete();
				ca = true;
			}
			if (GetAsyncKeyState(VK_F7))
			{
				TerraPGE::DoLighting = !TerraPGE::DoLighting;
			}
			if (GetAsyncKeyState(VK_F8))
			{
				TerraPGE::WireFrame = !TerraPGE::WireFrame;
			}
			if (GetAsyncKeyState(VK_F9))
			{
				TerraPGE::ShowTriLines = !TerraPGE::ShowTriLines;
			}
			if (GetAsyncKeyState(VK_F11))
			{
				TerraPGE::DebugClip = !TerraPGE::DebugClip;
			}
			if (GetAsyncKeyState(VK_F10))
			{
				ShowStrs = !ShowStrs;
			}
			if (GetAsyncKeyState(VK_ESCAPE))
			{
				//if (CurrMesh < Meshes.size() - 1)
				//{
				//	CurrMesh++;
				//}
				//else
				//{
				//	CurrMesh = 0;
				//}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
};


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	BrushPP ClearBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndCreatorW Wnd = WndCreatorW(CS_OWNDC, L"GameEngine", L"Game Engine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, (DWORD)WndExModes::BorderLessEx, (DWORD)WndModes::BorderLess | (DWORD)WndModes::ClipChildren, 0, 0, TerraPGE::sx, TerraPGE::sy);

#ifdef _DEBUG
	WndCreatorA Console = GetConsoleWindow();
#endif

	//Console.Hide();

	ExampleScene* ExScene = new ExampleScene();

	TerraPGE::Run(Wnd, ClearBrush, ExScene);

	//Console.Show();

#ifdef _DEBUG
	//system("pause");
	Console.Destroy();
#endif
}
