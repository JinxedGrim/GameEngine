#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <functional>
#include "Math.h"
#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"
#include <list>

constexpr auto VK_W = 0x57;
constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;
constexpr auto VK_D = 0x44;

struct TextureCoords
{
	float U = 0;
	float V = 0;
};

class Material
{
	public:
	Material()
	{
		this->AmbientColor = Vec3(255.0f, 0, 0);
		this->DiffuseColor = Vec3(255.0f * 0.75f, 0.0f, 0.0f);
		this->SpecularColor = Vec3(255.0f * 0.25f, 0.0f, 0.0f);
		this->Shininess = 96.0f;
	}
	Material(Vec3 AmbientColor, Vec3 DiffuseColor, Vec3 SpecularColor, float Shininess)
	{
		this->AmbientColor = AmbientColor;
		this->DiffuseColor = DiffuseColor;
		this->SpecularColor = SpecularColor;
		this->Shininess = Shininess;
	}

	bool LoadMaterial(std::string MtlFn, std::string MtlName)
	{
		std::ifstream mtlFile(MtlFn);
		if (!mtlFile.is_open())
		{
			// Error handling for failed MTL file loading
			// You can return a default material or throw an exception
			*this = Material();
			return false;
		}

		std::string line;
		while (std::getline(mtlFile, line))
		{
			std::stringstream ss(line);
			std::string keyword;
			ss >> keyword;

			if (keyword == "newmtl")
			{
				std::string name;
				ss >> name;

				if (name == MtlName)
				{
					this->MaterialName = name;
					// Parse the properties for the material
					while (std::getline(mtlFile, line))
					{
						std::stringstream ssProp(line);
						std::string propKeyword;
						ssProp >> propKeyword;

						if (propKeyword == "Ka")
						{
							ssProp >> this->AmbientColor.x >> this->AmbientColor.y >> this->AmbientColor.z;
							this->AmbientColor *= 255.0f;
						}
						else if (propKeyword == "Kd")
						{
							ssProp >> this->DiffuseColor.x >> this->DiffuseColor.y >> this->DiffuseColor.z;
							this->DiffuseColor *= 255.0f;
						}
						else if (propKeyword == "Ks")
						{
							ssProp >> this->SpecularColor.x >> this->SpecularColor.y >> this->SpecularColor.z;
							this->SpecularColor *= 255.0f;
						}
						else if (propKeyword == "Ns")
						{
							ssProp >> this->Shininess;
						}
						else if (propKeyword == "map_Kd")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{

							}
						}
						// Add more properties as needed

						// Check if the end of material is reached
						if (line.find("newmtl") != std::string::npos)
							break;
					}

					break; // Exit the loop once the material is found
				}
			}
		}

		mtlFile.close();

#ifdef _DEBUG
		std::cout << "Material Loaded: " << this->MaterialName << "\nAmbientColor: " << this->AmbientColor << "\nDiffuseColor: " << this->DiffuseColor << "\nSpecularColor: " << this->SpecularColor << "\nShininess: " << this->Shininess << std::endl << std::endl;;
#endif

		return true;
	}

	Vec3 AmbientColor = Vec3(255.0f * 0.15f, 0, 0);
	Vec3 DiffuseColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	Vec3 SpecularColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	float Shininess = 32.0f;

	std::string MaterialName = "Default";
};

class Triangle
{
public:

	Triangle()
	{
		memset(Points, 0, sizeof(Points));
	}

	Triangle(const Vec3& P1, const Vec3& P2, const Vec3& P3)
	{
		this->Points[0] = P1;
		this->Points[1] = P2;
		this->Points[2] = P3;
	}

	void Translate(const Vec3& Value)
	{
		this->Points[0] += Value;
		this->Points[1] += Value;
		this->Points[2] += Value;
	}

	const void Translated(Triangle* Out, const Vec3& Value)
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

	const void Scaled(Triangle* Out, const Vec3& Value)
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

	int ClipAgainstPlane(const Vec3& PointOnPlane, const Vec3& PlaneNormalized, Triangle& Out1, Triangle& Out2, bool DebugClip = false)
	{
		Vec3& p0 = this->Points[0];
		Vec3& p1 = this->Points[1];
		Vec3& p2 = this->Points[2];

		float PlanePointDot = PlaneNormalized.Dot(PointOnPlane);
		float dist0 = PlaneNormalized.Dot(p0) - PlanePointDot;
		float dist1 = PlaneNormalized.Dot(p1) - PlanePointDot;
		float dist2 = PlaneNormalized.Dot(p2) - PlanePointDot;

		Vec3* InsidePoints[3] = {};
		Vec3* OutsidePoints[3] = {};
		int InsideCount = 0;
		int OutsideCount = 0;

		if (dist0 >= 0)
			InsidePoints[InsideCount++] = &p0;
		else
			OutsidePoints[OutsideCount++] = &p0;

		if (dist1 >= 0)
			InsidePoints[InsideCount++] = &p1;
		else
			OutsidePoints[OutsideCount++] = &p1;

		if (dist2 >= 0)
			InsidePoints[InsideCount++] = &p2;
		else
			OutsidePoints[OutsideCount++] = &p2;

		int Count = 0;

		if (InsideCount == 3)
		{
			Out1 = *this;
			Count = 1;
		}
		else if (InsideCount == 1)
		{
			Out1 = *this;

			if (DebugClip)
			{
				Out1.Col = Vec3(0, 0, 255);
				Out1.OverRideMaterialColor = true;
			}

			Out1.Points[0] = *InsidePoints[0];
			Out1.Points[1] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);
			Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[1], PointOnPlane, PlaneNormalized);
			Count = 1;
		}
		else if (InsideCount == 2)
		{
			Out1 = *this;
			Out2 = *this;
			if (DebugClip)
			{
				Out1.Col = Vec3(0, 255, 0);
				Out1.OverRideMaterialColor = true;
				Out2.Col = Vec3(255, 0, 0);
				Out2.OverRideMaterialColor = true;
			}

			Out1.Points[0] = *InsidePoints[0];
			Out1.Points[1] = *InsidePoints[1];
			Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);

			Out2.Points[0] = *InsidePoints[1];
			Out2.Points[1] = Out1.Points[2];
			Out2.Points[2] = InsidePoints[1]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized);

			Count = 2;
		}

		return Count;
	}

	Vec3 Points[3];
	Vec3 Col = Vec3(255, 255, 255);
	Vec2 TexCoords = Vec2();
	Vec3 Normal = Vec3();
	Material Mat = Material();
	bool OverRideMaterialColor = false;
	bool UseMeshMat = false;
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
		this->NormalCount = 0;
		this->VertexCount = 0;
	}

	Mesh(std::string Fn)
	{
		this->LoadMesh(Fn);
	}

	Mesh(Mesh m, Material Mat)
	{
		*this = m;
		this->ChangeMatInfo(Mat);
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

	void ChangeMatInfo(Material Mat)
	{
		this->Mat = Mat;
		for (auto & Tri : this->Triangles)
		{
			Tri.Mat = this->Mat;
		}
	}

	bool LoadMesh(std::string FnPath)
	{
		std::ifstream ifs = std::ifstream(FnPath);

		if (!ifs.is_open())
			return false;

		std::vector<Vec3> VertCache;
		std::vector<Vec3> NormalCache;
		std::vector<Vec2> TexCache;

		std::string MtlLibFn = "";
		Material CurrMat = Material();

		while (!ifs.eof())
		{
			char Line[128];
			char Unused;
			std::stringstream SS;

			ifs.getline(Line, 128);

			SS << Line;
			std::string Str = SS.str();

			if (Str.find("v ") != std::string::npos)
			{
				Vec3 Vert;
				SS >> Unused >> Vert.x >> Vert.y >> Vert.z;
				VertCache.push_back(Vert);
			}
			else if (Str.find("vn ") != std::string::npos)
			{
				Vec3 Normal;
				SS >> Unused >> Normal.x >> Normal.y >> Normal.z;
				NormalCache.push_back(Normal);
			}
			else if (Str.find("vt ") != std::string::npos)
			{
				Vec2 TexCoord;
				SS >> Unused >> TexCoord.x >> TexCoord.y;
				TexCache.push_back(TexCoord);
			}
			else if (Str.find("f ") != std::string::npos)
			{
				std::vector<std::string> Indices;
				std::string IndexStr;
				while (SS >> IndexStr)
				{
					if (IndexStr[0] == 'f')
						continue; // Skip the 'f' character

					Indices.push_back(IndexStr);
				}
				if (Indices.size() == 3)
				{
					Triangle Tmp;
					for (int i = 0; i < 3; i++)
					{
						std::stringstream IndexSS(Indices[i]);
						std::string IndexPart;

						int vertexIndex, texCoordIndex, normalIndex;

						if (std::getline(IndexSS, IndexPart, '/'))
							vertexIndex = std::stoi(IndexPart) - 1;

						if (std::getline(IndexSS, IndexPart, '/'))
						{
							if (!IndexPart.empty())
								texCoordIndex = std::stoi(IndexPart) - 1;
							else
								texCoordIndex = -1;
						}
						else
						{
							texCoordIndex = -1;
						}

						if (std::getline(IndexSS, IndexPart, '/'))
						{
							if (!IndexPart.empty())
								normalIndex = std::stoi(IndexPart) - 1;
							else
								normalIndex = -1;
						}
						else
						{
							normalIndex = -1;
						}

						Vec3 vertex = VertCache[vertexIndex];

						if (texCoordIndex >= 0)
							Tmp.TexCoords = TexCache[texCoordIndex];

						if (normalIndex >= 0)
							Tmp.Normal = NormalCache[normalIndex];

						Tmp.Points[i] = vertex;
					}

					Tmp.Mat = CurrMat; // Assign the current material to the triangle

					this->Triangles.push_back(Tmp);
				}
			}
			else if (Str.find("usemtl ") != std::string::npos)
			{
				//char Prefix[7];
				std::string MaterialFn;
				SS >> Unused >> Unused >> Unused >> Unused >> Unused >> Unused >> MaterialFn;
				CurrMat.LoadMaterial(MtlLibFn, MaterialFn); // Update the current material
				this->MatCount++;

				if (this->MatCount > 1)
				{
					this->Mat.MaterialName = "Multiple";
					this->UseSingleMat = false;
				}
				else
				{
					this->Mat = CurrMat;
				}
			}
			else if (Str.find("mtllib ") != std::string::npos)
			{
				SS >> Unused >> Unused >> Unused >> Unused >> Unused >> Unused >> MtlLibFn;
			}
		}

		FnPath = FnPath.substr(FnPath.find_last_of("/\\") + 1);
		FnPath = FnPath.substr(0, FnPath.find_last_of(".obj") - 3);

		this->MeshName = FnPath;
		this->Vertices = VertCache;
		this->Normals = NormalCache;
		this->NormalCount = (int)NormalCache.size();
		this->TexCoords = TexCache;
		this->VertexCount = (int)VertCache.size();
		this->TriangleCount = (int)Triangles.size();

#ifdef _DEBUG
		std::cout << "MeshLoaded: " << MeshName << "\nVerts: " << this->VertexCount << "\nFaces: " << this->TriangleCount << "\nNormals: " << this->NormalCount << "\nMatName: " << this->Mat.MaterialName << "\nMatCount: " << this->MatCount << std::endl << std::endl;
#endif

		return true;
	}

	std::string MeshName = "";
	std::vector<Vec3> Vertices = {};
	std::vector<Triangle> Triangles = {};
	std::vector<Vec2> TexCoords = {};
	std::vector<Vec3> Normals = {};
	Material Mat = Material();
	int VertexCount = 0;
	int TriangleCount = 0;
	int NormalCount = 0;
	int MatCount = 0;
	bool UseSingleMat = true;
};

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
	SimpleLightSrc(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff)
	{
		this->LightPos = Pos;
		this->LightDir = LightDir;
		this->Color = LightColor;
		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;

		this->LightMesh = Mesh();
		this->Render = false;
	}
	SimpleLightSrc(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh)
	{
		this->LightPos = Pos;
		this->LightDir = LightDir;
		this->Color = LightColor;
		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;
		this->LightMesh = LightMesh;
		this->Render = false;
	}

	Vec3 LightDir = Vec3(0, 0, -1);
	Vec3 LightPos = Vec3(0, 0, 0);
	Vec3 Color = Vec3(253, 251, 211);
	//Vec3 Color = Vec3(255, 255, 255);
	Mesh LightMesh = Mesh();
	bool Render = false;

	float AmbientCoeff = 0.1f;  // Lowest level of light possible (only the objects mat ambient color)
	float SpecularCoeff = 0.5f; // How much the lights color will combine with the objects specular color
	float DiffuseCoeff = 0.25f; // How much the light will combine with the objects diffuse mat color
};

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

	Matrix PointAt(const Vec3 &Pos, const Vec3 &Target, const Vec3 &Up)
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

	void CalcCamViewMatrix(const Vec3 &Target)
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

typedef void(__fastcall* DoTick_T)(const GdiPP&, const WndCreator&, const float&);

auto Shader_Phong_LOW_LOD = [&](const Vec3& FragPos, const Vec3& FragNormal, const Material& Mat, const Vec3& LightColor, const float& LightAmbient, const float& LightDiffuse, const float& LightSpecular)
{

};

auto Shader_Phong = [&](const Vec3& FragPos, const Vec3& FragNormal, const Material& Mat, const Vec3& LightColor, const float& LightAmbient, const float& LightDiffuse, const float& LightSpecular)
{

};

namespace Engine
{
	std::string FpsStr = "Fps: ";
	std::wstring FpsWStr = L"Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	bool FpsEngineCounter = true;
	bool DoCull = true;
	bool DoLighting = true;
	bool WireFrame = false;
    bool ShowTriLines = false;
	bool DebugClip = false;
	bool LockCursor = true;
	bool CursorShow = false;
	bool UpdateMouseIn = true;


	POINT PrevMousePos;
	POINT DeltaMouse;
	float Sensitivity = 0.1f;
	int Fps = 0;

	void UpdateScreenInfo(GdiPP& Gdi)
	{
		Gdi.UpdateClientRgn();
		sx = Gdi.ClientRect.right - Gdi.ClientRect.left;
		sy = Gdi.ClientRect.bottom - Gdi.ClientRect.top;
	}

	void Run(WndCreator& Wnd, GdiPP& Gdi, BrushPP& ClearBrush, DoTick_T DrawCallBack)
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
		SetCursorPos(sx / 2, sy / 2);
		GetCursorPos(&PrevMousePos);

		while (!GetAsyncKeyState(VK_RETURN))
		{
			Start = std::chrono::system_clock::now();

			PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);

			// Translate and Dispatch message to WindowProc
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

			// Check Msg
			if (msg.message == WM_QUIT || msg.message == WM_CLOSE || msg.message == WM_DESTROY)
			{
				break;
			}


			// Check window focus and calc mouse deltas
			if (Wnd.HasFocus())
			{
				if (UpdateMouseIn)
				{
					POINT CurrMouse;
					GetCursorPos(&CurrMouse);
					DeltaMouse.x = CurrMouse.x - PrevMousePos.x;
					DeltaMouse.y = -(CurrMouse.y - PrevMousePos.y);
					DeltaMouse.x *= Sensitivity;
					DeltaMouse.y *= Sensitivity;
				}

				if (LockCursor)
					SetCursorPos(sx / 2, sy / 2);

				GetCursorPos(&PrevMousePos);

				if (CursorShow == false)
				{
					while (ShowCursor(FALSE) >= 0) {}
				}
				else
				{
					while (ShowCursor(TRUE) <= 0) {}
				}
			}

			// clear the screen
			Gdi.Clear();

			// call draw code
			DrawCallBack(Gdi, Wnd, (float)ElapsedTime);

			if (FpsEngineCounter)
			{
#ifdef UNICODE
				std::wstring Str = FpsWStr + std::to_wstring(Fps);
				Wnd.SetWndTitle(Str);
				Gdi.DrawStringW(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
				std::string Str = FpsStr + std::to_string(Fps);
				Wnd.SetWndTitle(Str);
				Gdi.DrawStringA(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
			}

			// Draw to screen
			Gdi.DrawDoubleBuffer();

			ElapsedTime = std::chrono::duration<double>(std::chrono::system_clock::now() - Start).count();

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

	void RenderMesh(GdiPP& Gdi, Camera& Cam, const Mesh& MeshToRender, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, Vec3 LightPos, const Vec3& LightColor, const float& LightAmbient, const float& LightDiffuse, const float& LightSpecular)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
		const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		std::vector<Triangle> TrisToRender = {};
		TrisToRender.reserve(MeshToRender.Triangles.size());

		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space
			Triangle Proj;

			Proj = Tri;

			Proj.ApplyMatrix(ObjectMatrix);

			// calc surface normal
			Vec3 TriNormal = (Proj.Points[1] - Proj.Points[0]).CrossNormalized((Proj.Points[2] - Proj.Points[0])); // this line and the if statement is used for culling

			if ((TriNormal.Dot(Proj.Points[0] - Cam.Pos) <= 0.0f) || !DoCull) // backface culling
			{
				float Intensity = 1.0f;

				// 3d Space -> Viewed Space
				Proj.ApplyMatrix(Cam.ViewMatrix);

				Triangle Clipped[2];
				Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };
				int Count = Proj.ClipAgainstPlane(Cam.NearPlane, PlaneNormal, Clipped[0], Clipped[1], DebugClip);

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

					// Calc lighting
					if (DoLighting && !ToProj.OverRideMaterialColor)
					{
						const Material* MatToUse;
						if (!MeshToRender.UseSingleMat)
							MatToUse = &ToProj.Mat;
						else
							MatToUse = &MeshToRender.Mat;

						Vec3 LDir = (LightPos - ((ToProj.Points[0] + ToProj.Points[1] + ToProj.Points[2]) / 3.0f)).Normalized();
						float Li = TriNormal.Dot(LDir);

						Intensity = std::max<float>(0.0f, Li);
						Vec3 RDir = LDir - TriNormal * 2.0f * Li;

						// Calculate the specular intensity
						float SpecularIntensity = pow(std::max<float>(0.0f, RDir.Dot(Cam.LookDir)), MatToUse->Shininess);

						Vec3 AmbientCol = (MatToUse->AmbientColor) * LightAmbient;
						Vec3 DiffuseCol = ((LightColor * Intensity) + (MatToUse->DiffuseColor * Intensity)) * LightDiffuse;
						Vec3 SpecularClr = ((LightColor * LightSpecular) + (MatToUse->SpecularColor * LightSpecular)) * SpecularIntensity;

						ToProj.Col.x = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
						ToProj.Col.y = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
						ToProj.Col.z = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
					}

					TrisToRender.push_back(ToProj);
				}
			}
		}

		// sort faces 
		std::sort(TrisToRender.begin(), TrisToRender.end(), [](const Triangle& t1, const Triangle& t2)
		{
			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;
			return z1 > z2;
		});

		for (const auto& Proj : TrisToRender)
		{
			Triangle Clipped[2];
			std::vector<Triangle> ListTris;

			ListTris.push_back(Proj);
			int NewTris = 1;

			for (int p = 0; p < 4; p++)
			{
				int NewTrisToAdd = 0;
				while (NewTris > 0)
				{
					Triangle Test = ListTris.front();
					ListTris.erase(ListTris.begin());
					NewTris--;

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

					for (int w = 0; w < NewTrisToAdd; w++)
					{
						ListTris.push_back(Clipped[w]);
					}
				}
				NewTris = (int)ListTris.size();
			}

			// draw
			for (const auto& Proj : ListTris)
			{
				if (!WireFrame)
				{
					if (!ShowTriLines)
						Gdi.DrawFilledTriangle((Proj.Points[0].x), (Proj.Points[0].y), (Proj.Points[1].x), (Proj.Points[1].y), (Proj.Points[2].x), (Proj.Points[2].y), BrushPP(RGB(Proj.Col.x, Proj.Col.y, Proj.Col.z)), PenPP(PS_SOLID, 1, RGB(Proj.Col.x, Proj.Col.y, Proj.Col.z)));
					else
						Gdi.DrawFilledTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, BrushPP(RGB(Proj.Col.x, Proj.Col.y, Proj.Col.z)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));
				}
				else
				{
					Gdi.DrawTriangle(Proj.Points[0].x, Proj.Points[0].y, Proj.Points[1].x, Proj.Points[1].y, Proj.Points[2].x, Proj.Points[2].y, PenPP(PS_SOLID, 1, RGB(Proj.Col.x, Proj.Col.y, Proj.Col.z)));
				}
			}
		}
	}

	void RenderRenderable(GdiPP &Gdi, Camera &Cam, Renderable &R, SimpleLightSrc LightSrc)
	{
		RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightPos, LightSrc.Color, LightSrc.AmbientCoeff, LightSrc.DiffuseCoeff, LightSrc.SpecularCoeff);
	}
}
