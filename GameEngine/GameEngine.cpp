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

static bool ShowStrs = true;
static float FTheta = 0;
static int CurrMesh = 0;
static bool IsFullScreen = true;

CHAR cwd [MAX_PATH + 1] = "";
DWORD len = GetCurrentDirectoryA(MAX_PATH, cwd);

std::string CWD = cwd;


//DoTick_T Draw(GdiPP& Gdi, WndCreatorW& Wnd, const float& ElapsedTime, std::vector<Renderable>* ToRender, std::vector<LightObject*>* Lights)
//{
//	static float PrevFov = Cam->Fov;
//	
//	if (!LockCamera)
//	{
//		Cam->ViewAngles += Vec3((float)TerraPGE::DeltaMouse.y, 0, 0);
//		Cam->ViewAngles.x = std::clamp<float>(Cam->ViewAngles.x, -89.0f, 89.0f);
//		Cam->ViewAngles -= Vec3(0, (float)TerraPGE::DeltaMouse.x, 0);
//	}
//
//	if (PrevFov != Cam->Fov)
//	{
//		Cam->CalcProjectionMat();
//		PrevFov = Cam->Fov;
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
//		Cam->AspectRatio = (float)((float)TerraPGE::sy / (float)TerraPGE::sx);
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
//		Cam->ProjectionMatrix = Proj;
//		Cam->ViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
//		Cam->Pos = -((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f;
//	}
//
//	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(0, 1, 0) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(0, -1, 0) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_A) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(-1, 0, 0) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_D) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(1, 0, 0) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_W) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(0, 0, 1) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (GetAsyncKeyState(VK_S) & 0x8000)
//	{
//		Cam->Pos += (Cam->GetNewVelocity(Vec3(0, 0, -1) * Cam->CamRotation)) * ElapsedTime;
//	}
//
//	if (!LockCamera)
//	{
//		Cam->CamRotation = Matrix::CreateRotationMatrix(Cam->ViewAngles); // Pitch Yaw Roll
//		Cam->LookDir = Cam->InitialLook * Cam->CamRotation;
//		Vec3 T = Cam->Pos + Cam->LookDir;
//		Cam->CalcCamViewMatrix(T);
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
//		Gdi.DrawStringA(20, 240, CamPosXstr + std::to_string(Cam->Pos.x) + CamPosYstr + std::to_string(Cam->Pos.y) + CamPosZstr + std::to_string(Cam->Pos.z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
//		Gdi.DrawStringA(20, 260, CamRotXstr + std::to_string(Cam->ViewAngles.x) + CamRotYstr + std::to_string(Cam->ViewAngles.y) + CamRotZstr + std::to_string(Cam->ViewAngles.z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
//	}
//
//	return NULL;
//}


class ExampleScene : public TerraPGE::Scene
{
	Mesh* Ryno = nullptr;
	Mesh* RCAMMO = nullptr;
	Mesh* AK47 = nullptr;
	Mesh* Spyro = nullptr;
	Mesh* Plane = nullptr;
	Mesh* CubeMsh = nullptr;
	Mesh* CubeMesh2 = nullptr;
	Material* CubeMat = nullptr;
	Vec3 LightSrcPos = Vec3();
	PointLight sl; // TODO make this ptr
	DirectionalLight Dl; // TODO make this ptr

	Material* WorldBlockMat = nullptr;

	TerraPGE::Renderable* SlRender = nullptr;
	TerraPGE::Renderable* CubeRender = nullptr;
	TerraPGE::Renderable* PlaneRender = nullptr;
	TerraPGE::Renderable* RCAMMO_Render = nullptr;
	TerraPGE::Renderable* HoveredRend = nullptr;
	TerraPGE::Renderable* Ak47Render = nullptr;
	std::vector<TerraPGE::Renderable*> WorldCubes = {};

	int LoadingMode = 0;

	bool LockCamera = false;
	bool EndSettings = false;
	std::thread SettingsTh;

	public:
	Texture* Txt = nullptr;

	ExampleScene()
	{
		
	}


	TerraPGE::Renderable* SpawnCubeAt(Vec3 Position)
	{
		return DEBUG_NEW TerraPGE::Renderable(CubeMesh2, this->MainCamera, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Position, TerraPGE::EngineShaders::Shader_Frag_Phong_Shadows);
	}


	void BeginScene(WndCreator& Wnd) override
	{
		Vec3 InitialVelocity = Vec3(4.0f, 1.5f, 0.0f);
		this->LightSrcPos = { 0, 6, 0 };
		Vec3 dir = { 0.918234f, -0.390731f, 0.064190f };

		this->LoadingMode++;
		TerraPGE::UpdateLoadingScreen();
		this->Txt = DEBUG_NEW Texture("Test.bmp");
		this->AK47 = DEBUG_NEW Mesh("AK47.obj");
		this->Plane = DEBUG_NEW Mesh("FlatTerrain.obj");
		this->WorldBlockMat = Material::CreateMaterial(SoftUnlitMatAmbient, SoftUnlitMatDiffuse, SoftUnlitMatSpecular, 16.0f, "Unlit");
		this->CubeMat = Material::GetNullMaterial();
		CubeMat->Textures.push_back(Txt);
		this->CubeMsh = DEBUG_NEW Cube(1, 1, 1, CubeMat);
		this->CubeMesh2 = DEBUG_NEW Cube(1, 1, 1, WorldBlockMat);

		this->LoadingMode++;
		TerraPGE::UpdateLoadingScreen();


		sl = PointLight(LightSrcPos, { 0, 0, 0 }, Vec3(253, 255, 255), 1.0f, 0.02f, 0.002f, 0.5f, 0.15f, 0.5f);
		sl.CastsShadows = true;
		sl.Render = true;
		Dl = DirectionalLight({0, 30, -60}, dir, Vec3(253, 251, 211), 0.15f, 0.3f, 0.2f);
		Dl.CastsShadows = true;

		this->MainCamera = DEBUG_NEW Camera(Vec3(0, 3, 0), (float)((float)TerraPGE::Core::sy / (float)TerraPGE::Core::sx), TerraPGE::Core::FOV, TerraPGE::Core::FNEAR, TerraPGE::Core::FFAR);

		LockCamera = false;
		this->MainCamera->SetLocalPosition(Vec3(0, 3, 0));
		this->MainCamera->SetLocalViewAngles(Vec3(-89, 0, 0));

		this->LoadingMode++;
		TerraPGE::UpdateLoadingScreen();
		CubeRender = DEBUG_NEW TerraPGE::Renderable(CubeMsh, this->MainCamera, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 5.f, -4.0f), TerraPGE::EngineShaders::Shader_Texture_Only);
		CubeRender->collider.PhysicsEnabled = false;
		Ak47Render = DEBUG_NEW TerraPGE::Renderable(CubeMesh2, this->MainCamera, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 6.f, 4.0f), TerraPGE::EngineShaders::Shader_Frag_Phong_Shadows);
		PlaneRender = DEBUG_NEW TerraPGE::Renderable(Plane, this->MainCamera, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), TerraPGE::EngineShaders::Shader_Frag_Phong_Shadows);
		CubeRender->mesh->MeshName = "SmileCube";

		AABBColliderParams params = Collider::CalculateAABB(Ak47Render->mesh->Triangles);
		Ak47Render->AddAABBCollider(params, 1.0f, 0.2f, InitialVelocity);
		Ak47Render->collider.body.KineticFriction = WOOD_KINETIC_FRICTION;

		params = Collider::CalculateAABB(PlaneRender->mesh->Triangles);
		PlaneRender->AddAABBCollider(params, 1000.0f, 0.1f, Vec3(0.0f, 0.0f, 0.0f));
		PlaneRender->collider.body.KineticFriction = ICE_KINETIC_FRICTION;
		PlaneRender->collider.PhysicsEnabled = false;

		//this->SlRender = DEBUG_NEW TerraPGE::Renderable(&(sl.LightMesh), (this->MainCamera), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), sl.Transform.GetLocalPosition(), TerraPGE::EngineShaders::Shader_Frag_Phong, ShaderTypes::SHADER_FRAGMENT);
		SettingsTh = std::thread(ExampleScene::Settings, this);

		this->LoadingMode++;
		TerraPGE::UpdateLoadingScreen();

		//for (int x = 0; x < 20; x++) 
		//{
		//	for (int z = 0; z < 20; z++) 
		//	{
		//		for (int y = 0; y <= 0; y++) 
		//		{
		//			TerraPGE::Renderable* Block = SpawnCubeAt(Vec3(x,y,z));
		//			WorldCubes.push_back(Block);
		//		}
		//	}
		//}
	}


	void DrawLoadingScreen(GdiPP* Gdi)
	{
		if (LoadingMode == 0)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2, "Loading Scene", RGB(255, 255, 255), TRANSPARENT);
		}
		else if (LoadingMode == 1)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2, "Loading Meshes", RGB(255, 255, 255), TRANSPARENT);
		}
		else if (LoadingMode == 2)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2, "Creating Lights", RGB(255, 255, 255), TRANSPARENT);
		}
		else if (LoadingMode == 3)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2, "Creating Objects And Calculating Bounding boxes", RGB(255, 255, 255), TRANSPARENT);
		}
		else if (LoadingMode == 4)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2, "Spawning World", RGB(255, 0, 0), TRANSPARENT);
		}
	}


	void HandleMovement(WndCreator& Wnd, const float& ElapsedTime)
	{
		if (Wnd.Input.IsKeyDown(VK_SPACE))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(0, 1, 0) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown(VK_LSHIFT))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(0, -1, 0) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown('A'))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(-1, 0, 0) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown('D'))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(1, 0, 0) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown('W'))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(0, 0, 1) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown('S'))
		{
			this->MainCamera->Transform.SetLocalPosition(MainCamera->Transform.GetLocalPosition() + (MainCamera->GetNewVelocity(Vec3(0, 0, -1) * MainCamera->Transform.GetWorldMatrix().GetBasis())) * ElapsedTime);
		}

		if (Wnd.Input.IsKeyDown('C'))
		{
			CubeRender->Transform.WalkTransformChain();
			//Cam->Transform.PointAt(CubeRender->Transform.GetWorldPosition(), Cam->CamUp);
		}

		if (!LockCamera)
		{
			Vec3 Euler = this->MainCamera->GetLocalViewAngles();

			Euler -= Vec3((float)Wnd.Input.Current.DeltaY * TerraPGE::Sensitivity, 0, 0);
			Euler -= Vec3(0, (float)Wnd.Input.Current.DeltaX * TerraPGE::Sensitivity, 0);

			this->MainCamera->SetLocalViewAngles(Euler.AngleNormalized());
		}

	}


	TerraPGE::Renderable* GetHoveredObj(const std::vector<TerraPGE::Renderable*>* ToRender)
	{
		TerraPGE::Renderable* hovered = nullptr;
		for (TerraPGE::Renderable* obj : *ToRender)
		{
			this->MainCamera->Transform.WalkTransformChain();
			Matrix inv = obj->Transform.World.QuickInversed();
			Vec3 localOrigin = this->MainCamera->Transform.GetWorldPosition() * inv;
			Vec3 localDir = ((this->MainCamera->GetLookDirection())).Normalized();


			if (!obj) continue;

			Ray camRay = Ray(localOrigin, localDir);
			RaycastHit Out;
			if (RaycastMesh(camRay, obj->mesh->Triangles, &Out))
			{
				if (Out.hit)
					hovered = obj;
			}
		}

		return hovered;
	}


	void RunTick(GdiPP* Gdi, WndCreator& Wnd, const float& ElapsedTime, std::vector<TerraPGE::Renderable*>* ToRender, std::vector<LightObject*>* Lights) override
	{
		if (Wnd.Input.IsKeyPressed(VK_INSERT))
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
			TerraPGE::Core::UpdateScreenInfo(Gdi);
			MainCamera->SetAspectRatio((float)((float)TerraPGE::Core::sy / (float)TerraPGE::Core::sx));
		}

		if (Wnd.Input.IsKeyPressed(VK_ESCAPE))
		{
			TerraPGE::Core::LockCursor = !TerraPGE::Core::LockCursor;
			LockCamera = true;
			TerraPGE::Core::CursorShow = !TerraPGE::Core::CursorShow;
			TerraPGE::Core::UpdateMouseIn = !TerraPGE::Core::UpdateMouseIn;
			TerraPGE::DoPhysics = !TerraPGE::DoPhysics;
		}

		if (Wnd.Input.IsKeyPressed(VK_HOME))
		{
			//LockCamera = !LockCamera;
			//Matrix Proj;
			//Proj.MakeOrthoMatrix(-40.0f, 40.0f, -40.0f, 40.0f, TerraPGE::Core::FNEAR, TerraPGE::Core::FFAR);
			//Cam->ProjectionMatrix = Proj;
			//Cam->ViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
			//Cam->Transform.SetLocalPosition(-((Vec3(0.0f, 0.0f, 0.0f) - LightSrcPos).Normalized()) * 100.0f);
		}

		if (Wnd.Input.IsKeyDown('P'))
		{
			TerraPGE::DoPhysics = !TerraPGE::DoPhysics;
		}

		if (Wnd.Input.IsLeftMouseDown())
		{
			if (HoveredRend)
			{
				HoveredRend->collider.body.Velocity = this->MainCamera->GetLookDirection().Normalized() * 8.0f;
			}
		}

		HandleMovement(Wnd, ElapsedTime);

		ToRender->push_back(CubeRender);
		ToRender->push_back(PlaneRender);
		ToRender->push_back(Ak47Render);

		for (TerraPGE::Renderable* Block : WorldCubes)
		{
			//ToRender->push_back(Block);
		}

		Lights->push_back(&Dl);

		HoveredRend = GetHoveredObj(ToRender);
	}


	void DrawCrosshair(GdiPP* Gdi)
	{
		Vec2 CenterPoint = Vec2(TerraPGE::Core::sx / 2, TerraPGE::Core::sy / 2);
		float Gap = 6.0f;
		float Len = 10.0f;    
		float Thickness = 2.0f;


		Gdi->DrawLine(CenterPoint.x, CenterPoint.y - Gap - Len, CenterPoint.x, CenterPoint.y - Gap);       // Top
		Gdi->DrawLine(CenterPoint.x, CenterPoint.y + Gap, CenterPoint.x, CenterPoint.y + Gap + Len);       // Bottom
		Gdi->DrawLine(CenterPoint.x - Gap - Len, CenterPoint.y, CenterPoint.x - Gap, CenterPoint.y);       // Left
		Gdi->DrawLine(CenterPoint.x + Gap, CenterPoint.y, CenterPoint.x + Gap + Len, CenterPoint.y);       // Right

		const static int rightSideGap = 250;

		if (HoveredRend != nullptr)
		{
			Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 40, HoveredRend->mesh->MeshName, RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 60, "Pos (" + std::to_string(HoveredRend->Transform.GetLocalPosition().x) + ", " + std::to_string(HoveredRend->Transform.GetLocalPosition().y) + ", " + std::to_string(HoveredRend->Transform.GetLocalPosition().z) + ")", RGB(255, 0, 0), TRANSPARENT);

			if (HoveredRend->mesh->Materials.size() != 0)
			{
				if (HoveredRend->mesh->Materials.size() == 1)
				{
					Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 80, HoveredRend->mesh->Materials[0]->MaterialName, RGB(255, 0, 0), TRANSPARENT);
				}
				else
				{
					Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 80, "Material Count: " + std::to_string(HoveredRend->mesh->Materials.size()), RGB(255, 0, 0), TRANSPARENT);
				}
				
				if (HoveredRend->mesh->Materials[0]->Textures.size() != 0)
				{
					if (HoveredRend->mesh->Materials[0]->Textures.size() == 1)
					{
						Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 100, HoveredRend->mesh->Materials[0]->Textures[0]->Name, RGB(255, 0, 0), TRANSPARENT);
					}
					else
					{
						Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 100, "Textures: " + HoveredRend->mesh->Materials[0]->Textures.size(), RGB(255, 0, 0), TRANSPARENT);
					}
				}
				else
				{
					Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 100, "No Texture", RGB(255, 0, 0), TRANSPARENT);
				}
			}
			else
			{
				Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 80, "No Material", RGB(255, 0, 0), TRANSPARENT);
			}

			Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 120, "ShaderType: " + std::to_string((int)HoveredRend->SHADER_TYPE), RGB(255, 0, 0), TRANSPARENT);

			Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 140, "Physics Enabled: " + std::to_string(HoveredRend->collider.PhysicsEnabled), RGB(255, 0, 0), TRANSPARENT);

			if (HoveredRend->collider.PhysicsEnabled)
			{
				Gdi->DrawStringA(TerraPGE::Core::sx - rightSideGap, 160, "Velocity: (" + std::to_string(HoveredRend->collider.body.Velocity.x) + ", " + std::to_string(HoveredRend->collider.body.Velocity.y) + ", " + std::to_string(HoveredRend->collider.body.Velocity.z) + ")", RGB(255, 0, 0), TRANSPARENT);
			}
		}
	}


	void DrawSceneGUI(GdiPP* Gdi)
	{
		if (TerraPGE::Core::FpsEngineCounter && ShowStrs)
		{
			static std::string FovStr = "(F2)  Fov: ";
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
			static std::string CamPosWolrd = "Camera World Pos: ( X: ";
			static std::string CamRotXstr = "Camera Rot: ( Pitch: ";
			static std::string CamRotYstr = ", Yaw: ";
			static std::string CamRotZstr = ", Roll: ";
			static std::string CamRotEndstr = ")";
			static std::string CamWorldRotXstr = "Camera  WorldRot: ( Pitch: ";
			static std::string DepthStr = "Depth: ";
			static std::string ClipStr = "Clip: (z, w): ";
			static std::string NdcStr = "Ndc: ";

			Gdi->DrawStringA(20, 40, FovStr + std::to_string(TerraPGE::Core::FOV), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 60, CullingStr + std::to_string(TerraPGE::Core::DoCull), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 80, LightingStr + std::to_string(TerraPGE::Core::DoLighting), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 100, FilledStr + std::to_string(TerraPGE::Core::WireFrame), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 120, TriLinesStr + std::to_string(TerraPGE::Core::ShowTriLines), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 160, CamPosXstr + std::to_string(MainCamera->Transform.GetLocalPosition().x) + CamPosYstr + std::to_string(MainCamera->Transform.GetLocalPosition().y) + CamPosZstr + std::to_string(MainCamera->Transform.GetLocalPosition().z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 180, CamPosWolrd + std::to_string(MainCamera->Transform.GetWorldPosition().x) + CamPosYstr + std::to_string(MainCamera->Transform.GetWorldPosition().y) + CamPosZstr + std::to_string(MainCamera->Transform.GetWorldPosition().z) + CamPosEndstr, RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 200, CamRotXstr + std::to_string(MainCamera->Transform.GetLocalEulerAngles().x) + CamRotYstr + std::to_string(MainCamera->Transform.GetLocalEulerAngles().y) + CamRotZstr + std::to_string(MainCamera->Transform.GetLocalEulerAngles().z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 220, CamWorldRotXstr + std::to_string(MainCamera->Transform.GetWorldRotation().x) + CamRotYstr + std::to_string(MainCamera->Transform.GetWorldRotation().y) + CamRotZstr + std::to_string(MainCamera->Transform.GetWorldRotation().z) + CamRotEndstr, RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 240, DepthStr + std::to_string(TerraPGE::Renderer::TestDepth), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 260, ClipStr + std::to_string(TerraPGE::Renderer::TestClipZ) + ", " + std::to_string(TerraPGE::Renderer::TestClipW), RGB(255, 0, 0), TRANSPARENT);
			Gdi->DrawStringA(20, 280, NdcStr + std::to_string(TerraPGE::Renderer::TestNdcZ), RGB(255, 0, 0), TRANSPARENT);
		}

		DrawCrosshair(Gdi);
	}


	void EndScene() override
	{
		delete this->Ryno;
		delete this->CubeMsh;
		delete this->CubeRender;
		delete this->Plane;
		delete this->PlaneRender;
		delete this->RCAMMO;
		delete this->MainCamera;
		this->Txt->Delete();
	}


	static void Settings(ExampleScene* Scene)
	{
		while (!Scene->EndSettings)
		{
			if (GetAsyncKeyState(VK_F1))
			{
				TerraPGE::Core::FpsEngineCounter = !TerraPGE::Core::FpsEngineCounter;
			}
			if (GetAsyncKeyState(VK_F2))
			{
				TerraPGE::Core::FOV += 30;
				Scene->MainCamera->SetFov(TerraPGE::Core::FOV);
				if (TerraPGE::Core::FOV > 120)
				{
					TerraPGE::Core::FOV = 60;
					Scene->MainCamera->SetFov(TerraPGE::Core::FOV);
				}
			}
			if (GetAsyncKeyState(VK_F3))
			{
				TerraPGE::Core::DoMultiThreading = !TerraPGE::Core::DoMultiThreading;
			}
			if (GetAsyncKeyState(VK_F4))
			{
				static bool ca = false;

				if (!ca)
					Scene->Txt->Delete();
				ca = true;
			}
			if (GetAsyncKeyState(VK_F5))
			{
				TerraPGE::Core::DoLighting = !TerraPGE::Core::DoLighting;
			}
			if (GetAsyncKeyState(VK_F6))
			{
				TerraPGE::Core::WireFrame = !TerraPGE::Core::WireFrame;
			}
			if (GetAsyncKeyState(VK_F7))
			{
				TerraPGE::Core::ShowTriLines = !TerraPGE::Core::ShowTriLines;
			}
			if (GetAsyncKeyState(VK_F8))
			{
				TerraPGE::Core::DebugClip = !TerraPGE::Core::DebugClip;
			}
			if (GetAsyncKeyState(VK_F10))
			{
				ShowStrs = !ShowStrs;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}
};


int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	Vec3 x(1, 0, 0);
	Vec3 y(0, 1, 0);

	Vec3 z = x.Cross(y);    // Using your cross product

	std::cout << x << " cross " << y << " = " << z << std::endl;

	Vec3 Target = Vec3(0, 0, 2);
	Vec3 Eye = Vec3(0, 0, -1);
	Vec3 WorldUp = Vec3(0, 1, 0);
	Vec3 Forward, Right, Up;

	Forward = (Target - Eye).Normalized();
	Right = (Forward.Cross(WorldUp)).Normalized();   // For left-handed
	Up = Right.Cross(Forward).Normalized();

	std::cout << "Target: " << Target << "\n"
		<< "Eye: " << Eye << "\n"
		<< "WorldUp: " << WorldUp << "\n"
		<< "Forward = " << Forward << "\n"
		<< "Right = " << Right << "\n"
		<< "Up = " << Up << "\n\n";

	float fovY = 90.0f;
	float aspect = 1.0f;
	float nearZ = 1.0f;
	float farZ = 10.0f;

	Vec3 worldPos(12, 5, -2);
	Vec4 Vw(worldPos.x, worldPos.y, worldPos.z, 1.0f);

	Camera* Cam = new Camera(Vec3(10, 3, -5), aspect, fovY, nearZ, farZ);
	Matrix TransOnly = Cam->Transform.Local;
	Cam->SetLocalViewAngles(Vec3(0, 90, 0));
	Matrix LocalMat = Cam->Transform.Local;

	Matrix Rx, Ry, Rz;

	Matrix::CreateRotationX(&Rx, 0);
	Matrix::CreateRotationY(&Ry, 90.0f);
	Matrix::CreateRotationZ(&Rz, 0);
	Matrix Rot = Rz * Ry * Rx;
	Vec3 EulerA = Rot.ExtractEuler();
	Matrix View = Cam->GetViewMatrix();

	Vec4 Vview = Vw * View;
	Vec4 Vclip = Vview * Cam->GetProjectionMatrix();

	// Perspective divide
	float ndcX = Vclip.x / Vclip.w;
	float ndcY = Vclip.y / Vclip.w;
	float ndcZ = Vclip.z / Vclip.w;

	std::cout << "Cam Pos: " << Cam->Transform.GetLocalPosition() << std::endl;
	std::cout << "Cam Rot: " << Cam->Transform.GetLocalEulerAngles() << std::endl;
	std::cout << "Cam RotX Mat: " << Rx << std::endl;
	std::cout << "Cam RotY Mat: " << Ry << std::endl;
	std::cout << "Cam RotZ Mat: " << Rz << std::endl;
	std::cout << "Cam Full Rot Mat: " << Rot << std::endl;
	std::cout << "Extracted Euler: " << EulerA << std::endl;
	std::cout << "Cam Local Matrix: " << LocalMat << std::endl;
	std::cout << "Cam Local Matrix (Trans only): " << TransOnly << std::endl;
	std::cout << "Cam View Matrix: " << View << std::endl;
	std::cout << "Cam Projection Matrix: " << Cam->GetProjectionMatrix() << std::endl;
	std::cout << "World Position  = " << Vw << "\n";
	std::cout << "View Position   = " << Vview << "\n";
	std::cout << "Clip Position   = " << Vclip << "\n";
	std::cout << "NDC             = ("
		<< ndcX << ", " << ndcY << ", " << ndcZ << ")\n";

	std::cout << "\nExpected (for verification):\n";
	std::cout << "View = [-3, 2, 2, 1]\n";
	std::cout << "Clip = [-3, 2, 1.11111, 2]\n";
	std::cout << "NDC  = [-1.5, 1, 0.555556]\n";

	system("pause");


	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(1648658); // example: break at specific leak ID

	BrushPP ClearBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	WndCreatorW Wnd = WndCreatorW(CS_OWNDC, L"GameEngine", L"Game Engine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, (DWORD)WndExModes::BorderLessEx, (DWORD)WndModes::BorderLess | (DWORD)WndModes::ClipChildren, 0, 0, TerraPGE::Core::sx, TerraPGE::Core::sy);

#ifdef _DEBUG
	WndCreatorA Console = GetConsoleWindow();
#endif

	//Console.Hide();

	ExampleScene* ExScene = new ExampleScene();

	TerraPGE::Run(Wnd, ExScene);

	//Console.Show();

#ifdef _DEBUG
	//system("pause");
	Console.Destroy();
#endif
}
