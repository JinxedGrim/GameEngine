#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include "Math.h"
#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"
#include <list>

constexpr auto VK_W = 0x57;
constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;
constexpr auto VK_D = 0x44;

class Triangle
{
public:

	Triangle()
	{
		this->Points[0] = Vec3();
		this->Points[1] = Vec3();
		this->Points[2] = Vec3();

		this->r = 255;
		this->b = 255;
		this->g = 255;
	}

	Triangle(const Vec3 P1, const Vec3 P2, const Vec3 P3)
	{
		this->Points[0] = P1;
		this->Points[1] = P2;
		this->Points[2] = P3;

		this->r = 255;
		this->b = 255;
		this->g = 255;
	}

	void Translate(Vec3 Value)
	{
		this->Points[0] += Value;
		this->Points[1] += Value;
		this->Points[2] += Value;
	}

	const void Translated(Triangle* Out, const Vec3 Value)
	{
		Out->Points[0] = this->Points[0] + Value;
		Out->Points[1] = this->Points[1] + Value;
		Out->Points[2] = this->Points[2] + Value;
	}

	void Scale(const Vec3 &Value)
	{
		this->Points[0] *= Value;
		this->Points[1] *= Value;
		this->Points[2] *= Value;
	}

	const void Scaled(Triangle* Out, const Vec3 Value)
	{
		Out->Points[0] = this->Points[0] * Value;
		Out->Points[1] = this->Points[1] * Value;
		Out->Points[2] = this->Points[2] * Value;
	}

	void Rotate(const Matrix &Rot)
	{
		this->Points[0] *= Rot;
		this->Points[1] *= Rot;
		this->Points[2] *= Rot;
	}

	const void Rotated(Triangle* Out, const Matrix &Rot)
	{
		Out->Points[0] = this->Points[0] * Rot;
		Out->Points[1] = this->Points[1] * Rot;
		Out->Points[2] = this->Points[2] * Rot;
	}

	void ApplyMatrix(const Matrix &Mat)
	{
		this->Points[0] *= Mat;
		this->Points[1] *= Mat;
		this->Points[2] *= Mat;
	}

	int ClipAgainstPlane(const Vec3& PointOnPlane, const Vec3& PlaneNormalized, Triangle &Out1, Triangle &Out2, bool DebugClip = false)
	{
		Vec3* InsidePoints[3];
		Vec3* OutsidePoints[3];
		int InsideCount = 0;
		int OutsideCount = 0;

		auto Dist = [&](const Vec3& Point)
		{
			return(PlaneNormalized.Dot(Point) - PlaneNormalized.Dot(PointOnPlane));
		};

		if (Dist(this->Points[0]) >= 0)
		{
			InsidePoints[InsideCount++] = &this->Points[0];
		}
		else
		{
			OutsidePoints[OutsideCount++] = &this->Points[0];
		}

		if (Dist(this->Points[1]) >= 0)
		{
			InsidePoints[InsideCount++] = &this->Points[1];
		}
		else
		{
			OutsidePoints[OutsideCount++] = &this->Points[1];
		}

		if (Dist(this->Points[2]) >= 0)
		{
			InsidePoints[InsideCount++] = &this->Points[2];
		}
		else
		{
			OutsidePoints[OutsideCount++] = &this->Points[2];
		}

		int Count = 0;

		switch (InsideCount)
		{
			case 3:
			{
				Out1 = *this;
				Count = 1;
				break;
			}
			case 0:
			{
				Count = 0;
				break;
			}
			case 1:
			{
				if (OutsideCount != 2)
					break;

				if (!DebugClip)
				{
					Out1.r = this->r;
					Out1.g = this->g;
					Out1.b = this->b;
				}
				else
				{
					Out1.r = 0;
					Out1.g = 0;
					Out1.b = 255;
				}

				Out1.Points[0] = *InsidePoints[0];
				Out1.Points[1] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);
				Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[1], PointOnPlane, PlaneNormalized);
				Count = 1;
				break;
			}
			case 2:
			{
				if (OutsideCount == 1)
				{
					if (!DebugClip)
					{
						Out1.r = this->r;
						Out1.g = this->g;
						Out1.b = this->b;
					}
					else
					{
						Out1.r = 0;
						Out1.g = 255;
						Out1.b = 0;
					}

					if (!DebugClip)
					{
						Out2.r = this->r;
						Out2.g = this->g;
						Out2.b = this->b;
					}
					else
					{
						Out2.r = 255;
						Out2.g = 0;
						Out2.b = 0;
					}

					Out1.Points[0] = *InsidePoints[0];
					Out1.Points[1] = *InsidePoints[1];
					Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);

					Out2.Points[0] = *InsidePoints[1];
					Out2.Points[1] = Out1.Points[2];
					Out2.Points[2] = InsidePoints[1]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);

					Count = 2;
				}
			}
		}

		return Count;
	}

	Vec3 Points[3];
	int r = 255;
	int b = 255;
	int g = 255;
};

class Mesh
{
public:

	Mesh()
	{

	}

	Mesh(std::vector<Triangle> Triangles, std::string MeshName = "")
	{
		this->Triangles = Triangles;
		this->MeshName = MeshName;
		this->TriangleCount = (int)Triangles.size();
	}

	Mesh(std::string Fn)
	{
		this->LoadMesh(Fn);
	}

	void TranslateTriangles(Vec3 Value)
	{
		for (int i = 0; i < this->Triangles.size(); i++)
		{
			Triangles.at(i).Translate(Value);
		}
	}

	void ScaleTriangles(Vec3 Value)
	{
		for (int i = 0; i < this->Triangles.size(); i++)
		{
			this->Triangles.at(i).Points[0] *= Value;
			this->Triangles.at(i).Points[1] *= Value;
			this->Triangles.at(i).Points[2] *= Value;
		}
	}

	Mesh ScaledTriangles(Vec3 Value)
	{
		Mesh Out = *this;
		for (int i = 0; i < this->Triangles.size(); i++)
		{
			Out.Triangles.at(i).Points[0] * Value;
			Out.Triangles.at(i).Points[1] * Value;
			Out.Triangles.at(i).Points[2] * Value;
		}
	}

	bool LoadMesh(std::string FnPath)
	{
		std::ifstream ifs = std::ifstream(FnPath);

		if (!ifs.is_open())
			return false;

		std::vector<Vec3> VertCache;

		while (!ifs.eof())
		{
			char Line[128];
			char Unused;
			std::stringstream SS;

			ifs.getline(Line, 128);

			SS << Line;

			if (Line[0] == 'v')
			{
				Vec3 Vert;
				SS >> Unused >> Vert.x >> Vert.y >> Vert.z;
				VertCache.push_back(Vert);
			}

			if (Line[0] == 'f')
			{
				int idx[3];
				SS >> Unused >> idx[0] >> idx[1] >> idx[2];

				if (idx[0] <= 0 || idx[0] > VertCache.size() ||
					idx[1] <= 0 || idx[1] > VertCache.size() ||
					idx[2] <= 0 || idx[2] > VertCache.size()) 
				{
					return false;
				}

				Triangle Tmp = Triangle(VertCache[(long long)idx[0] - 1], VertCache[(long long)idx[1] - 1], VertCache[(long long)idx[2] - 1]);

				this->Triangles.push_back(Tmp);
			}
		}

		FnPath = FnPath.substr(FnPath.find_last_of("/\\") + 1);

		FnPath = FnPath.substr(0, FnPath.find_last_of(".obj") - 3);

		this->MeshName = FnPath;

		this->VertexCount = (int)VertCache.size();
		this->TriangleCount = (int)Triangles.size();

#ifdef _DEBUG
		std::cout << "MeshLoaded: " << MeshName << " Verts: " << this->VertexCount << " Tris: " << this->TriangleCount << std::endl;
#endif

		return true;
	}

	std::string MeshName = "";
	std::vector<Triangle> Triangles = {};
	int VertexCount = 0;
	int TriangleCount = 0;
};

Mesh CubeMesh = Mesh({
	//SOUTH SIDE OF CUBE
	{ {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f} },
	{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },

	// EAST FACE
	{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f} },
	{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} },

	// NORTH FACE
	{ {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} },
	{ {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },

	// WEST FACE
	{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
	{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },

	// TOP FACE
	{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} },
	{ {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f} },

	// BOTTOM FACE
	{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
	{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
	}, "Cube");

class Renderable
{
public:
	Renderable()
	{
		this->mesh = Mesh();
		this->Pos = Vec3();
		this->RotationRads = Vec3();
		this->Scalar = Vec3();
	}

	Renderable(Mesh mesh, Vec3 Scalar, Vec3 RotationRads, Vec3 Pos)
	{
		this->Pos = Pos;
		this->mesh = mesh;
		this->Scalar = Scalar;
		this->RotationRads = RotationRads;
	}

	Mesh mesh = Mesh();
	Vec3 Pos = Vec3();
	Vec3 Scalar = Vec3();
	Vec3 RotationRads = Vec3();
};

class Cube : public Renderable
{
public:
    Cube() : Renderable(CubeMesh, Vec3(), Vec3(1, 1, 1), Vec3(0, 0, 0))
	{

	}
	Cube(Vec3 Scalar, Vec3 RotationRads, Vec3 Pos) : Renderable(CubeMesh, Scalar, RotationRads, Pos)
	{

	}
};

class SimpleLightSrc
{
public:
	SimpleLightSrc(float Ambient)
	{
		this->Ambient = Ambient;
		this->LightDir = Vec3(0, 0, -1);
		this->LightMesh = Mesh();
		this->Render = false;
	}
	SimpleLightSrc(float Ambient, Vec3 Dir)
	{
		this->Ambient = Ambient;
		this->LightDir = Dir;
		this->LightMesh = Mesh();
		this->Render = false;
	}
	SimpleLightSrc(float Ambient, Vec3 Dir, Mesh m, bool Render)
	{
		this->Ambient = Ambient;
		this->LightDir = Dir;
		this->LightMesh = m;
		this->Render = Render;
	}

	static float CalcDiffuse(Vec3 ObjNormal, Vec3 Dir)
	{
		return ObjNormal.Dot(Dir.Normalized());
	}

	float CalcDiffuse(Vec3 ObjNormal)
	{
		return ObjNormal.Dot(this->LightDir.Normalized());
	}

	float Ambient = 1.0f;
	Vec3 LightDir = Vec3(0, 0, -1);
	Mesh LightMesh = Mesh();
	bool Render = false;
};

typedef void(__fastcall* DoTick_T)(const GdiPP&, const float);

class Camera
{
public:
	Camera(Vec3 Position, float AspectRatio, float Fov, float Near, float Far)
	{
		this->Pos = Position;

		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };

		this->CalcProjectionMat();

	}

	Camera(Vec3 Position, Vec3 TargetLook, Vec3 CamUp, float AspectRatio, float Fov, float Near, float Far)
	{
		this->Pos = Position;

		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->CamUp = CamUp;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->CalcProjectionMat();
	}

	static Matrix CalcProjectionMat(float AspectRatio, float Fov, float Near, float Far)
	{
		Matrix Result = {};

		float FovRads = 1.0f / tanf(ToRad(Fov * 0.5f));

		Result.fMatrix[0][0] = AspectRatio * FovRads;
		Result.fMatrix[1][1] = FovRads;
		Result.fMatrix[2][2] = Far / (Far - Near);
		Result.fMatrix[3][2] = (-Far * Near) / (Far - Near);
		Result.fMatrix[2][3] = 1.0f;
		Result.fMatrix[3][3] = 0.0f;

		return Result;
	}

	void CalcProjectionMat()
	{
		float FovRads = abs(1.0f / tanf((ToRad(Fov / 2))));

		this->ProjectionMatrix.fMatrix[0][0] = this->AspectRatio * FovRads;
		this->ProjectionMatrix.fMatrix[1][1] = FovRads;
		this->ProjectionMatrix.fMatrix[2][2] = this->Far / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[3][2] = (-this->Far * this->Near) / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[2][3] = 1.0f;
		this->ProjectionMatrix.fMatrix[3][3] = 0.0f;
	}

	Mesh ProjectMesh(Mesh InMesh, Matrix Mat)
	{
		Mesh OutMesh = Mesh();

		for (int i = 0; i < InMesh.Triangles.size(); i++)
		{
			Triangle NewTri;
			
			NewTri.Points[0] = InMesh.Triangles.at(i).Points[0] * Mat;
			NewTri.Points[1] = InMesh.Triangles.at(i).Points[1] * Mat;
			NewTri.Points[2] = InMesh.Triangles.at(i).Points[2] * Mat;

			OutMesh.Triangles.push_back(NewTri);
		}

		return OutMesh;
	}

	Triangle ProjectTriangle(const Triangle* InTriangle, Matrix& Mat)
	{
		Triangle OutTri;

		OutTri.Points[0] = InTriangle->Points[0] * Mat;
		OutTri.Points[1] = InTriangle->Points[1] * Mat;
		OutTri.Points[2] = InTriangle->Points[2] * Mat;

		return OutTri;
	}

	Mesh ProjectMesh(Mesh InMesh)
	{
		Mesh OutMesh = Mesh();

		for (int i = 0; i < InMesh.Triangles.size(); i++)
		{
			Triangle NewTri;

			NewTri.Points[0] = InMesh.Triangles.at(i).Points[0] * this->ProjectionMatrix;
			NewTri.Points[1] = InMesh.Triangles.at(i).Points[1] * this->ProjectionMatrix;
			NewTri.Points[2] = InMesh.Triangles.at(i).Points[2] * this->ProjectionMatrix;

			OutMesh.Triangles.push_back(NewTri);
		}

		return OutMesh;
	}

	Triangle ProjectTriangle(const Triangle* InTriangle)
	{
		Triangle OutTri = *InTriangle;

		OutTri.Points[0] = InTriangle->Points[0] * this->ProjectionMatrix;
		OutTri.Points[1] = InTriangle->Points[1] * this->ProjectionMatrix;
		OutTri.Points[2] = InTriangle->Points[2] * this->ProjectionMatrix;

		return OutTri;
	}

	void ProjectTriangle(const Triangle* InTriangle, Triangle &OutTri)
	{
		OutTri.Points[0] = InTriangle->Points[0] * this->ProjectionMatrix;
		OutTri.Points[1] = InTriangle->Points[1] * this->ProjectionMatrix;
		OutTri.Points[2] = InTriangle->Points[2] * this->ProjectionMatrix;
	}

	Triangle TriangleProjected(const Triangle* InTriangle)
	{
		return
		{
			InTriangle->Points[0] * this->ProjectionMatrix,
			InTriangle->Points[1] * this->ProjectionMatrix,
			InTriangle->Points[2] * this->ProjectionMatrix
		};
	}

	Matrix PointAt(Vec3 &Pos, Vec3 &Target, Vec3 &Up)
	{
		Vec3 NewForward = (Target - Pos).Normalized();

		Vec3 NewUp = (Up - (NewForward * Up.Dot(NewForward))).Normalized();

		Vec3 NewRight = NewUp.Cross(NewForward);

		Matrix DimensioningAndTrans;
		DimensioningAndTrans.fMatrix[0][0] = NewRight.x;	    DimensioningAndTrans.fMatrix[0][1] = NewRight.y;	    DimensioningAndTrans.fMatrix[0][2] = NewRight.z;      DimensioningAndTrans.fMatrix[0][3] = 0.0f;
		DimensioningAndTrans.fMatrix[1][0] = NewUp.x;		    DimensioningAndTrans.fMatrix[1][1] = NewUp.y;		    DimensioningAndTrans.fMatrix[1][2] = NewUp.z;         DimensioningAndTrans.fMatrix[1][3] = 0.0f;
		DimensioningAndTrans.fMatrix[2][0] = NewForward.x;		DimensioningAndTrans.fMatrix[2][1] = NewForward.y;		DimensioningAndTrans.fMatrix[2][2] = NewForward.z;    DimensioningAndTrans.fMatrix[2][3] = 0.0f;
		DimensioningAndTrans.fMatrix[3][0] = Pos.x;				DimensioningAndTrans.fMatrix[3][1] = Pos.y;				DimensioningAndTrans.fMatrix[3][2] = Pos.z;			  DimensioningAndTrans.fMatrix[3][3] = 1.0f;

		return DimensioningAndTrans;
	}

	void CalcCamViewMatrix(Vec3 &Target)
	{
		this->ViewMatrix = this->PointAt(this->Pos, Target, this->CamUp).QuickInversed();
	}

	Vec3 GetNewVelocity(const Vec3 &Direction)
	{
		return Direction * this->Velocity;
	}

	Vec3 GetNewVelocity()
	{
		return this->LookDir * this->Velocity;
	}

	Vec3 Pos = Vec3(0, 0, 0);
	Vec3 ViewAngles = Vec3(0, 0, 0);
	Vec3 InitialLook = Vec3(0, 0, 1);
	Vec3 LookDir = Vec3(0, 0, 0);
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0 };
	float Velocity = 8.0f;

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};
	Matrix CamRotation = {};
	float Fov = 0.f;
	float AspectRatio = 0.f;
	float Near = 0.f;
	float Far = 0.f;
};

namespace Engine
{
	std::string FpsStr = "Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	bool FpsEngineCounter = true;
	bool DoCull = true;
	bool DoLighting = true;
	bool WireFrame = false;
    bool ShowTriLines = false;
	bool DebugClip = false;
	bool LockCursor = true;
	bool ShowCursor = false;

	POINT PrevMousePos;
	POINT DeltaMouse;
	float Sensitivity = 0.6f;
	int Fps = 0;

	void Run(WndCreatorW& Wnd, GdiPP& Gdi, BrushPP& ClearBrush, DoTick_T DrawCallBack)
	{
		// Init Variables
		sx = Wnd.GetClientArea().Width;
		sy = Wnd.GetClientArea().Height;
		Gdi.UpdateClientRgn();

		MSG msg = { 0 };
		double ElapsedTime = 0.0f;
		double FpsCounter = 0.0f;
		int FrameCounter = 0;
		auto Start = std::chrono::system_clock::now();
		auto End = std::chrono::system_clock::now();
		SetCursorPos(sx / 2, sy / 2);
		GetCursorPos(&PrevMousePos);

		while (!GetAsyncKeyState(VK_RETURN))
		{
			Start = std::chrono::system_clock::now();

			PeekMessageW(&msg, Wnd.Wnd, 0, 0, PM_REMOVE);

			// Translate and Dispatch message to WindowProc
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			if (Wnd.HasFocus())
			{
				POINT CurrMouse;
				GetCursorPos(&CurrMouse);
				DeltaMouse.x = CurrMouse.x - PrevMousePos.x;
				DeltaMouse.y = -(CurrMouse.y - PrevMousePos.y);
				DeltaMouse.x *= Sensitivity;
				DeltaMouse.y *= Sensitivity;

				if(LockCursor)
					SetCursorPos(sx / 2, sy / 2);

				GetCursorPos(&PrevMousePos);
			}

			// Check Msg
			if (msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY)
			{
				break;
			}

			Gdi.Clear(GDIPP_FILLRECT, ClearBrush);

			DrawCallBack(Gdi, (float)ElapsedTime);

			if (FpsEngineCounter)
			{
				Gdi.DrawStringA(20, 20, FpsStr + std::to_string(Fps), RGB(255, 0, 0), TRANSPARENT);
			}

			Gdi.DrawDoubleBuffer();

			End = std::chrono::system_clock::now();

			std::chrono::duration<double> Time = End - Start;

			ElapsedTime = Time.count();

			if (FpsEngineCounter)
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
			}
		}

		Wnd.Destroy();
	}

	void RenderMesh(GdiPP& Gdi, Camera& Cam, Mesh& MeshToRender, Vec3 Scalar, Vec3 RotationRads, Vec3 Pos, Vec3 LightDir, float Ambient)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar.x, Scalar.y, Scalar.z); // Scalar Matrix
		Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		std::vector<Triangle> TrisToRender = {};

		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space
			Triangle Proj;

			Proj = Tri;

			Proj.ApplyMatrix(ObjectMatrix);

			// calc surface normal
			Vec3 TriNormal = (Proj.Points[1] - Proj.Points[0]).CrossNormalized((Proj.Points[2] - Proj.Points[0])); // this line and the if statement is used for culling

			if ((TriNormal.Dot(Proj.Points[0] - Cam.Pos) < 0.0f) || !DoCull) // culling
			{
				float Intensity = 1.0f;

				// 3d Space -> Viewed Space
				Proj.ApplyMatrix(Cam.ViewMatrix);

				Triangle Clipped[2];
				Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };
				int Count = Proj.ClipAgainstPlane(Cam.NearPlane, PlaneNormal.Normalized(), Clipped[0], Clipped[1], DebugClip);

				if (Count == 0)
					continue;

				for (int i = 0; i < Count; i++)
				{
					Triangle ToProj = Clipped[i];

					// 3d Space -> Screen Space
					ToProj = Cam.ProjectTriangle(&ToProj); // Project from 3D Space To Screen Space

					// Offset to normalized space
					ToProj.Translate(Vec3(1.0f, 1.0f, 0.0f));
					ToProj.Scale(Vec3((float)(Engine::sx * 0.5f), (float)(Engine::sy * 0.5f), 1));
					ToProj.Translate(Vec3(-1, -1, 0));

					//Proj.r *= (Intensity);
					//Proj.g *= (Intensity);
					//Proj.b *= (Intensity);

					// Calc lighting intensity
					if (DoLighting)
					{
						Intensity = TriNormal.Dot(LightDir.Normalized());
						ToProj.r = std::clamp<float>(((ToProj.r * Ambient) + (ToProj.r * abs(Intensity))), 0, 255);
						ToProj.g = std::clamp<float>(((ToProj.g * Ambient) + (ToProj.g * abs(Intensity))), 0, 255);
						ToProj.b = std::clamp<float>(((ToProj.b * Ambient) + (ToProj.b * abs(Intensity))), 0, 255);
					}

					TrisToRender.push_back(ToProj);
				}
			}
		}

		// sort faces 
		std::sort(TrisToRender.begin(), TrisToRender.end(), [](Triangle& t1, Triangle& t2)
		{
			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;
			return z1 > z2;
		});

		for (const auto& Proj : TrisToRender)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to 
			//  ensure we only test new triangles generated against planes
			Triangle Clipped[2];
			std::list<Triangle> ListTris;

			// Add initial triangle
			ListTris.push_back(Proj);
			int NewTris = 1;

			for (int p = 0; p < 4; p++)
			{
				int NewTrisToAdd = 0;
				while (NewTris > 0)
				{
					// Take triangle from front of queue
					Triangle Test = ListTris.front();
					ListTris.pop_front();
					NewTris--;

					// Clip it against a plane. We only need to test each 
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
						case 0:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], Engine::DebugClip);
							break;
						}
						case 1:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], Engine::DebugClip);
							break;
						}
						case 2:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Engine::DebugClip);
							break;
						}
						case 3:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ (float)sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Engine::DebugClip);
							break;
						}
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < NewTrisToAdd; w++)
					{
						ListTris.push_back(Clipped[w]);
					}
				}
				NewTris = ListTris.size();
			}
			// draw
			for (const auto& Proj : ListTris)
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
		}
	}

	void RenderRenderable(GdiPP &Gdi, Camera &Cam, Renderable &R, SimpleLightSrc LightSrc)
	{
		//v	Matrix RotM = Matrix::CreateRotationMatrix(FTheta * RotSpeedX, FTheta * RotSpeedY, FTheta * RotSpeedZ);
		//Matrix TransMat = Matrix::CreateTranslationMatrix(Vec3(1, 1, 10.0f));
		//Matrix WorldMatrix = Matrix::CreateIdentity();
		//WorldMatrix = (WorldMatrix * RotM) * TransMat;

		//std::vector<Triangle> TrisToRender = {};

		//// Project and translate object 
		//for (const auto& Tri : Meshes.at(CurrMesh).Triangles)
		//{
		//	// 3D Space
		//	Triangle Proj;

		//	Proj = Tri;

		//	Proj.ApplyMatrix(WorldMatrix);

		//	// calc surface normal
		//	Vec3 TriNormal = (Proj.Points[1] - Proj.Points[0]).CrossNormalized((Proj.Points[2] - Proj.Points[0])); // this line and the if statement is used for culling

		//	if ((TriNormal.Dot(Proj.Points[0] - Cam.Pos) < 0.0f) || !DoCull) // culling
		//	{
		//		Intensity = 1.0f;

		//		// Calc lighting intensity
		//		if (DoLighting)
		//			Intensity = TriNormal.Dot(LightSrc.Normalized());

		//		// 3d Space -> Viewed Space
		//		Proj.ApplyMatrix(Cam.ViewMatrix);

		//		// 3d Space -> Screen Space
		//		Proj = Cam.ProjectTriangle(&Proj); // Project from 3D Space To Screen Space

		//		// Offset to normalized space
		//		Proj.Translate(Vec3(1.0f, 1.0f, 0.0f));
		//		Proj.Scale(Vec3((float)(Engine::sx * 0.5f), (float)(Engine::sy * 0.5f), 1));
		//		Proj.Translate(Vec3(-1, -1, 0));

		//		Proj.r *= Intensity;
		//		Proj.g *= Intensity;
		//		Proj.b *= Intensity;

		//		TrisToRender.push_back(Proj);
		//	}
		//}

		//// sort faces 
		//std::sort(TrisToRender.begin(), TrisToRender.end(), [](Triangle& t1, Triangle& t2)
		//	{
		//		float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
		//		float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;

		//		return z1 > z2;
		//	});

		//// draw
		//for (const auto& Proj : TrisToRender)
		//{
		//	if (!WireFrame)
		//	{
		//		if (!ShowTriLines)
		//			Gdi.DrawFilledTriangle((Proj.Points[0].x), (Proj.Points[0].y), (Proj.Points[1].x), (Proj.Points[1].y), (Proj.Points[2].x), (Proj.Points[2].y), BrushPP(RGB(Proj.r, Proj.g, Proj.b)), PenPP(PS_SOLID, 1, RGB(Proj.r, Proj.g, Proj.b)));
		//		else
		//			Gdi.DrawFilledTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, BrushPP(RGB(Proj.r, Proj.g, Proj.b)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));
		//	}
		//	else
		//	{
		//		Gdi.DrawTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, PenPP(PS_SOLID, 1, RGB(Proj.r, Proj.g, Proj.b)));
		//	}
		//}

		RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightDir, LightSrc.Ambient);
	}
}
