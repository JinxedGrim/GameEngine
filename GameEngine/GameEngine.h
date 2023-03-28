#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include "Math.h"
#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"

class Triangle
{
public:

	Triangle()
	{
		this->Points[0] = Vec3();
		this->Points[1] = Vec3();
		this->Points[2] = Vec3();
	}

	Triangle(Vec3 P1, Vec3 P2, Vec3 P3)
	{
		this->Points[0] = P1;
		this->Points[1] = P2;
		this->Points[2] = P3;
	}

	void Translate(Vec3 Value)
	{
		this->Points[0] += Value;
		this->Points[1] += Value;
		this->Points[2] += Value;
	}

	void Translated(Triangle* Out, Vec3 Value)
	{
		Out->Points[0] = this->Points[0] + Value;
		Out->Points[1] = this->Points[1] + Value;
		Out->Points[2] = this->Points[2] + Value;
	}

	void Scale(Vec3 Value)
	{
		this->Points[0] *= Value;
		this->Points[1] *= Value;
		this->Points[2] *= Value;
	}

	void Scaled(Triangle* Out, Vec3 Value)
	{
		Out->Points[0] = this->Points[0] * Value;
		Out->Points[1] = this->Points[1] * Value;
		Out->Points[2] = this->Points[2] * Value;
	}

	void Rotate(Matrix Rot)
	{
		this->Points[0] *= Rot;
		this->Points[1] *= Rot;
		this->Points[2] *= Rot;
	}

	void Rotated(Triangle* Out, Matrix Rot)
	{
		Out->Points[0] = this->Points[0] * Rot;
		Out->Points[1] = this->Points[1] * Rot;
		Out->Points[2] = this->Points[2] * Rot;
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
		this->TriangleCount = Triangles.size();
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
			Triangles.at(i).Points[0] *= Value;
			Triangles.at(i).Points[1] *= Value;
			Triangles.at(i).Points[2] *= Value;
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

		this->VertexCount = VertCache.size();
		this->TriangleCount = Triangles.size();

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

class Renderable
{
public:
	Renderable()
	{
		this->mesh = Mesh();
		this->Pos = Vec3();
		this->TransformMatrix = Matrix();
	}

	Renderable(Vec3 Pos_, Mesh mesh_)
	{
		this->Pos = Pos_;
		this->mesh = mesh_;
	}

	Mesh mesh = Mesh();
	Vec3 Pos = Vec3();
	Matrix TransformMatrix = Matrix();
};


Mesh Cube = Mesh({
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

typedef void(__fastcall* DoDraw_t)(const GdiPP&, const float);

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

		this->CalcProjectionMat();

		this->EulerRotation = Vec3(0, 0, 0);
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
		Triangle OutTri;

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


	Vec3 Pos = Vec3(0, 0, 0);
	Vec3 EulerRotation = Vec3(0, 0, 0);
	Matrix ProjectionMatrix = {};
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

	void Run(WndCreatorW& Wnd, GdiPP& Gdi, BrushPP& ClearBrush, DoDraw_t DrawCallBack)
	{
		// Init Variables
		sx = Wnd.GetWindowSz().x;
		sy = Wnd.GetWindowSz().y;

		MSG msg = { 0 };
		double ElapsedTime = 0.0f;
		double FpsCounter = 0.0f;
		int FrameCounter = 0;
		int Fps = 0;

		while (PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE) && !GetAsyncKeyState(VK_RETURN))
		{
			auto Start = std::chrono::system_clock::now();
			// Some computation here

			// Translate and Dispatch message to WindowProc
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

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

			auto End = std::chrono::system_clock::now();
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

	void RenderMesh(Camera &Cam, Renderable R)
	{

	}
}
