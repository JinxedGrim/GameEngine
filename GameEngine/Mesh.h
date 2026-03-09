#pragma once
#include <memory>
#include "Math.h"
#include "Texture.h"
#include "Materials.h"

struct Vertex
{
	Vec4 Points[3];
	Vec3 Normals[3];
};

struct VertexAttributes
{
	Vec3 Attribute[3];
};

struct ClipVertex
{
	Vec4 Clip;
	Vec4 World;
	Vec3 Normal;
};

class Triangle
{
	public:
	Triangle()
	{
		memset(Points.Points, 0, sizeof(Points.Points));
		memset(Points.Normals, 0, sizeof(Points.Normals));
	}

	Triangle(const Vec3& P1, const Vec3& P2, const Vec3& P3)
	{
		this->Points.Points[0] = P1;
		this->Points.Points[1] = P2;
		this->Points.Points[2] = P3;
	}

	Triangle(const Triangle& B)
	{
		*this = B;
	}

	void Translate(const Vec3& Value)
	{
		this->Points.Points[0] += Value;
		this->Points.Points[1] += Value;
		this->Points.Points[2] += Value;
	}

	const void Translated(Triangle* Out, const Vec3& Value)
	{
		Out->Points.Points[0] = this->Points.Points[0] + Value;
		Out->Points.Points[1] = this->Points.Points[1] + Value;
		Out->Points.Points[2] = this->Points.Points[2] + Value;
	}

	void Scale(const Vec3& Value)
	{
		this->Points.Points[0] *= Value;
		this->Points.Points[1] *= Value;
		this->Points.Points[2] *= Value;
	}

	const void Scaled(Triangle* Out, const Vec3& Value)
	{
		Out->Points.Points[0] = this->Points.Points[0] * Value;
		Out->Points.Points[1] = this->Points.Points[1] * Value;
		Out->Points.Points[2] = this->Points.Points[2] * Value;
	}

	void Rotate(const Matrix& Rot)
	{
		this->Points.Points[0] *= Rot;
		this->Points.Points[1] *= Rot;
		this->Points.Points[2] *= Rot;
	}

	const void Rotated(Triangle* Out, const Matrix& Rot)
	{
		Out->Points.Points[0] = this->Points.Points[0] * Rot;
		Out->Points.Points[1] = this->Points.Points[1] * Rot;
		Out->Points.Points[2] = this->Points.Points[2] * Rot;
	}

	void ApplyMatrix(const Matrix& MatToApply)
	{
		this->Points.Points[0] *= MatToApply;
		this->Points.Points[1] *= MatToApply;
		this->Points.Points[2] *= MatToApply;
	}

	void ApplyPerspectiveDivide()
	{
		this->TexCoords[0].CorrectPerspective(this->Points.Points[0].w);
		this->TexCoords[1].CorrectPerspective(this->Points.Points[1].w);
		this->TexCoords[2].CorrectPerspective(this->Points.Points[2].w);

		this->Points.Points[0].CorrectPerspective();
		this->Points.Points[1].CorrectPerspective();
		this->Points.Points[2].CorrectPerspective();
	}


	const bool IsWindingCCW(const Vec4& a, const Vec4& b, const Vec4& c) const
	{
		Vec3 ab = (b / b.w).GetVec3() - (a / a.w).GetVec3();
		Vec3 ac = (c / c.w).GetVec3() - (a / a.w).GetVec3();
		Vec3 n = ab.Cross(ac);

		return n.z >= 0;  // CCW in LH DX
	}



	int ClipAgainstPlane(const Vec3& PointOnPlane, const Vec3& PlaneNormalized, Triangle& Out1, Triangle& Out2, bool DebugClip = false)
	{
		// Attributes
		Vec4 p0 = this->Points.Points[0];
		Vec4 p1 = this->Points.Points[1];
		Vec4 p2 = this->Points.Points[2];

		TextureCoords& t0 = this->TexCoords[0];
		TextureCoords& t1 = this->TexCoords[1];
		TextureCoords& t2 = this->TexCoords[2];
		//

		// Attribute storage
		Vec4* InsidePoints[3] = {};
		Vec4* OutsidePoints[3] = {};
		TextureCoords* InsideTex[3] = {};
		TextureCoords* OutsideTex[3] = {};
		//

		float PlanePointDot = PlaneNormalized.Dot(PointOnPlane);
		float dist0 = PlaneNormalized.Dot(p0) - PlanePointDot;
		float dist1 = PlaneNormalized.Dot(p1) - PlanePointDot;
		float dist2 = PlaneNormalized.Dot(p2) - PlanePointDot;
		int InsideCount = 0;
		int OutsideCount = 0;
		int InsideTexCount = 0;
		int OutsideTexCount = 0;

		if (dist0 >= 0)
		{
			InsidePoints[InsideCount++] = &p0; 
			InsideTex[InsideTexCount++] = &t0;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p0; 
			OutsideTex[OutsideTexCount++] = &t0;
		}

		if (dist1 >= 0)
		{
			InsidePoints[InsideCount++] = &p1; 
			InsideTex[InsideTexCount++] = &t1;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p1; 
			OutsideTex[OutsideTexCount++] = &t1;
		}

		if (dist2 >= 0)
		{
			InsidePoints[InsideCount++] = &p2; 
			InsideTex[InsideTexCount++] = &t2;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p2; 
			OutsideTex[OutsideTexCount++] = &t2;
		}

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
				Out1.OverrideTextureColor = true;
			}

			Out1.Points.Points[0] = *InsidePoints[0];
			Out1.TexCoords[0] = *InsideTex[0];

			float t = 0.0f;
			Out1.Points.Points[1] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.TexCoords[1] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out1.Points.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[1], PointOnPlane, PlaneNormalized, &t);
			Out1.TexCoords[2] = InsideTex[0]->Lerp(*OutsideTex[1], t);

			Count = 1;
		}
		else if (InsideCount == 2)
		{
			Out1 = *this;
			Out2 = *this;

			if (DebugClip)
			{
				Out1.Col = Vec3(0, 255, 0);
				Out1.OverrideTextureColor = true;
				Out2.Col = Vec3(255, 0, 0);
				Out2.OverrideTextureColor = true;
			}


			Out1.Points.Points[0] = *InsidePoints[0];
			Out1.Points.Points[1] = *InsidePoints[1];
			Out1.TexCoords[0] = *InsideTex[0];
			Out1.TexCoords[1] = *InsideTex[1];

			float t = 0.0f;
			Out1.Points.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.TexCoords[2] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out2.Points.Points[0] = *InsidePoints[1];
			Out2.Points.Points[1] = Out1.Points.Points[2];
			Out2.TexCoords[0] = *InsideTex[1];
			Out2.TexCoords[1] = Out1.TexCoords[2];

			Out2.Points.Points[2] = InsidePoints[1]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out2.TexCoords[2] = InsideTex[1]->Lerp(*OutsideTex[0], t);

			Count = 2;
		}

		return Count;
	}


	int ClipAgainstPlane(const Vec4& plane, Triangle& Out1, Triangle& Out2, bool DebugClip) const
	{
		// Attributes to interp
		Vec4 in[3] = { Points.Points[0], Points.Points[1], Points.Points[2] };
		Vec4 world[3] = { WorldSpaceVerts.Points[0], WorldSpaceVerts.Points[1], WorldSpaceVerts.Points[2] };
		Vec3 Normals[3] = { VertexNormals[0], VertexNormals[1], VertexNormals[2] };

		float w[3] = { Points.Points[0].w, Points.Points[1].w, Points.Points[2].w }; // store original w
		float d[3];
		bool inside[3];

		int inCount = 0;
		for (int i = 0; i < 3; i++)
		{
			d[i] = plane.Dot(in[i]);
			inside[i] = d[i] >= 0.0f;
			if (inside[i]) inCount++;
		}

		if (inCount == 0) return 0;

		auto IntersectPos = [&](int a, int b)
		{
			return d[a] / (d[a] - d[b]);
		};

		auto InterpolatePos = [&](int a, int b, Vec4* Inside, float t)
		{
			Vec4 pos = Inside[a] + (Inside[b] - Inside[a]) * t;

			// Interpolate w
			pos.w = w[a] + (w[b] - w[a]) * t;

			return pos;
		};

		auto InterpolateVec3 = [&](int a, int b, Vec3* Inside, float t)
		{
				Vec3 pos = Inside[a] + (Inside[b] - Inside[a]) * t;
				return pos;
		};

		std::vector<Vec4> outPos;
		std::vector<Vec4> outWorld;
		std::vector<Vec3> outNormal;

		for (int i = 0; i < 3; i++)
		{
			int j = (i + 1) % 3;

			if (inside[i])
			{
				outPos.push_back(in[i]);
				outWorld.push_back(world[i]);
				outNormal.push_back(Normals[i]);
			}

			if (inside[i] ^ inside[j])
			{
				float t = IntersectPos(i, j);
				outPos.push_back(InterpolatePos(i, j, in, t));
				outWorld.push_back(InterpolatePos(i, j, world, t));
				outNormal.push_back(InterpolateVec3(i, j, Normals, t));
			}
		}

		if (outPos.size() == 3)
		{
			Out1 = *this;
			Out1.Points.Points[0] = outPos[0];
			Out1.Points.Points[1] = outPos[1];
			Out1.Points.Points[2] = outPos[2];
			Out1.Points.Normals[0] = outNormal[0];
			Out1.Points.Normals[1] = outNormal[1];
			Out1.Points.Normals[2] = outNormal[2];
			Out1.WorldSpaceVerts.Points[0] = outWorld[0];
			Out1.WorldSpaceVerts.Points[1] = outWorld[1];
			Out1.WorldSpaceVerts.Points[2] = outWorld[2];

			if (DebugClip && this->OverrideTextureColor != true)
			{
				bool Same = true;

				for (int i = 0; i < 3; i++)
				{
					if (!(Points.Points[i].GetVec3() == Out1.Points.Points[i].GetVec3()))
					{
						Same = false;
						break;
					}
				}

				if (!Same)
				{
					Out1.Col = Vec3(0, 255, 0);
					Out1.OverrideTextureColor = true;
				}
			}

			return 1;
		}

		if (outPos.size() == 4)
		{
			Out1 = *this;
			Out1.Points.Points[0] = outPos[0];
			Out1.Points.Points[1] = outPos[1];
			Out1.Points.Points[2] = outPos[2];
			Out1.Points.Normals[0] = outNormal[0];
			Out1.Points.Normals[1] = outNormal[1];
			Out1.Points.Normals[2] = outNormal[2];
			Out1.WorldSpaceVerts.Points[0] = outWorld[0];
			Out1.WorldSpaceVerts.Points[1] = outWorld[1];
			Out1.WorldSpaceVerts.Points[2] = outWorld[2];

			Out2 = *this;
			Out1.Points.Points[0] = outPos[0];
			Out1.Points.Points[1] = outPos[2];
			Out1.Points.Points[2] = outPos[3];
			Out1.Points.Normals[0] = outNormal[0];
			Out1.Points.Normals[1] = outNormal[2];
			Out1.Points.Normals[2] = outNormal[3];
			Out1.WorldSpaceVerts.Points[0] = outWorld[0];
			Out1.WorldSpaceVerts.Points[1] = outWorld[2];
			Out1.WorldSpaceVerts.Points[2] = outWorld[3];

			if (DebugClip)
			{
				Out1.Col = Vec3(255, 0, 0);
				Out1.OverrideTextureColor = true;
				Out2.Col = Vec3(0, 0, 255);
				Out2.OverrideTextureColor = true;
			}

			return 2;
		}

		return 0;
	}


	Vertex Points;

	Vertex WorldSpaceVerts = {};

	Vec3 FaceNormal = Vec3();
	Vec3 VertexNormals[3] = {};
	Vec3 NormalPositions[4] = {};
	Vec3 NormDirections[4] = {};

	TextureCoords TexCoords[3] = {};
	size_t TexCoords_[3] = {};

	Vec3 Col = Vec3(255, 0, 0);
	Material* Material;

	bool UseMeshMaterial = false;
	bool HasTexture = false;
	bool OverrideTextureColor = false;
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
		this->ChangeMatInfo(Material::GetNullMaterial());
		this->CalculateNormals();
	}


	Mesh(std::string Fn)
	{
		this->LoadMesh(Fn);
	}


	Mesh(Mesh m, Material* Mat)
	{
		*this = m;
		if (this->Normals.size() == 0)
		{
			this->CalculateNormals();
		}

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
			this->Triangles.at(i).Points.Points[0] *= Value;
			this->Triangles.at(i).Points.Points[1] *= Value;
			this->Triangles.at(i).Points.Points[2] *= Value;
		}
	}


	Mesh ScaledTriangles(Vec3 Value)
	{
		Mesh Out = *this;
		for (int i = 0; i < this->Triangles.size(); i++)
		{
			Out.Triangles.at(i).Points.Points[0] * Value;
			Out.Triangles.at(i).Points.Points[1] * Value;
			Out.Triangles.at(i).Points.Points[2] * Value;
		}
	}


	void ChangeMatInfo(Material* MatToApply)
	{
		this->Materials.push_back(MatToApply);

		for (auto& Tri : this->Triangles)
		{
			Tri.Material = MatToApply;

			if (Tri.Material->Textures.size() > 0 && Tri.Material->Textures.at(0)->Used)
				Tri.HasTexture = true;
		}
	}


	bool LoadMesh(std::string FnPath, std::string Prefix="Assets\\")
	{
		std::ifstream ifs = std::ifstream(Prefix + FnPath);

		if (!ifs.is_open())
			return false;

		std::vector<Vec3> VertCache;
		std::vector<Vec3> NormalCache;
		std::vector<TextureCoords> TexCache;

		std::string MtlLibFn = "";
		Material* CurrMat = nullptr;

		TerraPGE::Core::LogInfo("[MATERIAL]", "Loading Mesh: " + Prefix + FnPath);

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
				SS >> Unused >> Unused >> Normal.x >> Normal.y >> Normal.z;
				NormalCache.push_back(Normal);
			}
			else if (Str.find("vt ") != std::string::npos)
			{
				TextureCoords TexCoord;
				SS >> Unused >> Unused >> TexCoord.u >> TexCoord.v;

				if (std::fabs(TexCoord.u) > 1.0f)
				{
					TexCoord.u = std::fabs(fmod(TexCoord.u, 1.0f));
				}
				if (std::fabs(TexCoord.v) > 1.0f)
				{
					TexCoord.v = std::fabs(fmod(TexCoord.v, 1.0f));
				}

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

						int vertexIndex = 0, texCoordIndex = 0, normalIndex = 0;

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
							Tmp.TexCoords[i] = TexCache[texCoordIndex];
						else
							Tmp.TexCoords[i] = { 0, 0 }; 

						if (normalIndex >= 0)
							Tmp.FaceNormal = NormalCache[normalIndex];

						Tmp.Points.Points[i] = vertex;
					}

					Tmp.Material = CurrMat; 
					Tmp.HasTexture = (Tmp.Material != nullptr && Tmp.Material->Textures.size() > 0 && Tmp.Material->Textures.at(0) != nullptr);

					this->Triangles.push_back(Tmp);
				}
			}
			else if (Str.find("usemtl ") != std::string::npos)
			{
				//char Prefix[7];
				std::string MaterialFn;
				SS >> Unused >> Unused >> Unused >> Unused >> Unused >> Unused >> MaterialFn;
				CurrMat = Material::LoadMaterialFile(MtlLibFn, MaterialFn, Prefix);
				this->MatCount++;
				this->Materials.push_back(CurrMat);
			}
			else if (Str.find("mtllib ") != std::string::npos)
			{
				SS >> Unused >> Unused >> Unused >> Unused >> Unused >> Unused >> MtlLibFn;
			}
		}

		TerraPGE::Core::LogInfo("[Mesh]", "Finished Loading");

		FnPath = FnPath.substr(FnPath.find_last_of("/\\") + 1);
		FnPath = FnPath.substr(0, FnPath.find_last_of(".obj") - 3);

		this->MeshName = FnPath;
		this->VertexCache = VertCache;
		this->Normals = NormalCache;
		this->NormalCount = (int)NormalCache.size();
		this->TexCoords = TexCache;
		this->TexCoordsCount = (int)TexCache.size();
		this->VertexCount = (int)VertCache.size();
		this->TriangleCount = (int)Triangles.size();

		if (Normals.size() == 0)
			this->CalculateNormals();

#ifdef _DEBUG
		std::cout << VertexCache.size() << "   Normals:   ";
		std::cout << Normals.size() << "   TexturedCahce:   " << TexCache.size();
		std::cout << "   Faces:   " << Triangles.size() << "\n\n";
#endif

		return true;
	}


	void CalculateNormals()
	{
		for (Triangle& Tri : this->Triangles)
		{
			Vec3 Norm = (Tri.Points.Points[1] - Tri.Points.Points[0]).GetVec3().CrossNormalized((Tri.Points.Points[2] - Tri.Points.Points[0]));
			this->Normals.push_back(Norm);
			Tri.FaceNormal = Norm;

			for (int i = 0; i < 3; i++)
			{
				Tri.VertexNormals[i] = Norm;
			}
		}

		this->NormalCount = (int)this->Normals.size();
	}


	// Make this obsolete
	std::vector<Triangle> Triangles = {};

	std::string MeshName = "";
	std::vector<Vec3> VertexCache = {};
	std::vector<Vec3> NormalCache = {};
	std::vector<TextureCoords> TexCoords = {};
	std::vector<Material*> Materials = {};
	std::vector<Vec3> Normals = {};

	int VertexCount = 0;
	int TriangleCount = 0;
	int NormalCount = 0;
	int TexCount = 0;
	int TexCoordsCount = 0;
	int MatCount = 0;
	bool UseSingleMat = true;
	bool BackfaceCulling = true;
};


const Mesh CubeMesh = Mesh(
	{
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
	},
	"Cube");


class Cube : public Mesh
{
	public:
	Cube(float Length, float Width, float Height, Material* Mat = Material::GetNullMaterial()) : Mesh(CubeMesh, Mat)
	{

		for (int i = 0; i < CubeMesh.Triangles.size(); i++)
		{
			this->Triangles[i].Points.Points[0] *= Vec3(Length, Width, Height);
			this->Triangles[i].Points.Points[1] *= Vec3(Length, Width, Height);
			this->Triangles[i].Points.Points[2] *= Vec3(Length, Width, Height);

			this->Triangles[i].HasTexture = true;
			this->Triangles[i].Material = Mat;

			if ((i + 1) % 2 == 0)
			{
				this->Triangles[i].TexCoords[0] = { 0.0f, 1.0f };
				this->Triangles[i].TexCoords[1] = { 1.0f, 0.0f };
				this->Triangles[i].TexCoords[2] = { 1.0f, 1.0f };

			}
			else
			{
				this->Triangles[i].TexCoords[0] = { 0.0f, 1.0f };
				this->Triangles[i].TexCoords[1] = { 0.0f, 0.0f };
				this->Triangles[i].TexCoords[2] = { 1.0f, 0.0f };

			}
		}

		this->MeshName = "Cube";
	}
};


class Sphere : public Mesh
{
	public:
	Sphere(float Radius, int LatitudeSegments, int LongitudeSegments, Material* Mat = Material::GetNullMaterial()) : Mesh()
	{
		this->Triangles = {};
		// whole function is a pasted parameterized equation

		for (int lat = 0; lat <= LatitudeSegments; ++lat)
		{
			float theta = (float)(lat * PI / LatitudeSegments);
			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			for (int lon = 0; lon <= LongitudeSegments; ++lon)
			{
				float phi = (float)(lon * 2 * PI / LongitudeSegments);
				float sinPhi = std::sin(phi);
				float cosPhi = std::cos(phi);

				float x = Radius * sinTheta * cosPhi;
				float y = Radius * cosTheta;
				float z = Radius * sinTheta * sinPhi;

				this->VertexCache.push_back(Vec3(x, y, z));
			}
		}

		for (int lat = 0; lat < LatitudeSegments; ++lat)
		{
			for (int lon = 0; lon < LongitudeSegments; ++lon)
			{
				int currentVertex = lat * (LongitudeSegments + 1) + lon;
				int nextVertex = currentVertex + 1;
				int topVertex = currentVertex + LongitudeSegments + 1;
				int topNextVertex = topVertex + 1;

				this->Triangles.push_back(Triangle(this->VertexCache[currentVertex], this->VertexCache[topVertex], this->VertexCache[nextVertex]));

				this->Triangles.push_back(Triangle(this->VertexCache[nextVertex], this->VertexCache[topVertex], this->VertexCache[topNextVertex]));

				this->VertexCount = (int)VertexCache.size();
				this->TriangleCount = (int)Triangles.size();
			}
		}

		this->MeshName = "Sphere";
		this->CalculateNormals();
		this->ChangeMatInfo(Mat);
	}
};