#pragma once
#define NOMINMAX
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <functional>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <filesystem>

#include "Math.h"
#include "Graphics/GdiPP.hpp"
#include "Graphics/WndCreator.hpp"
#include "Graphics/TerraGL.h"
#include "ParrallelPP.h"

#define FLOAT_LOWEST 0.0000001f

#ifdef SSE_SIMD_42_SUPPORT
#include "Intrinsics/VectorExHelper.h"
#endif

// The entire premise of this project is to build a game engine without using any libraries, but the std library and the WinAPI. 
// GdiPP is more or less straight WinAPI, but you do have to link with a dll which is sort of cheating but it's about the only way to do graphics without any libs like DX or OpenGl
// Direct2D and DirectWrite are justified the same way

// Windows only include
#include <Psapi.h>

//     TO DO 
// 3. Revamp Material and texture system ie put all materials in the mesh and pointers to them in the triangle
// 4. Caclulate Vertex norms for all 3 vertices and store them
// 5. figure out a better way of per pixel shading
// 6. redisign engine into a more API-like manner
// X. Audio system 
// X. Voice 
// X. More light types
// X. Ambient occlusion 
// X. Global illumination
// X. FXAA, TSAA, Etc
// X. Proper UI system
// X. Probably migrate to unicode

enum class ShaderTypes
{
	SHADER_TRIANGLE = 0,
	SHADER_FRAGMENT = 1,
};

enum class LightTypes
{
	PointLight = 0,
	DirectionalLight = 1,
	SpotLight = 2,
};

constexpr auto VK_W = 0x57;
constexpr auto VK_A = 0x41;
constexpr auto VK_S = 0x53;
constexpr auto VK_D = 0x44;

#include <iostream>
#include <fstream>
#include <vector>

class TextureCoords
{
	public:

	TextureCoords()
	{
		u = 0;
		v = 0;
		w = 1.0f;
	}

	TextureCoords(float _u, float _v, float _w = 1.0f)
	{
		u = _u;
		v = _v;
		w = _w;
	}

	float u = 0;
	float v = 0;
	float w = 1.0f;

	void __inline CorrectPerspective(float _w)
	{
		if (_w != 0)
		{
			this->u = this->u / _w;
			this->v = this->v / _w;
			this->w = 1.0f / _w;
		}
	}

	TextureCoords Lerp(const TextureCoords& end, float t) const
	{
		TextureCoords result;

		result.u = this->u + (end.u - this->u) * t;
		result.v = this->v + (end.v - this->v) * t;
		result.w = this->w + (end.w - this->w) * t;

		return result;
	}

	void Lerped(const TextureCoords& end, float t) 
	{
		this->u = this->u + (end.u - this->u) * t;
		this->v = this->v + (end.v - this->v) * t;
		this->w = this->w + (end.w - this->w) * t;
	}
};

class Texture
{
	public:
	
	enum class WrappingMode
	{
		Clamp,
		Repeat,
		MirroredRepeat
	};

	private:

	int Width = 0;
	int Height = 0;
	std::vector<unsigned char> PixelData = {};
	WrappingMode WrapMode;

	// Function to load a BMP file and store pixel data
	bool LoadBMP(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);

		if (!file || !file.is_open())
		{
			return false;
		}

		// Read the BMP header
		char header[54];
		file.read(header, 54);

		// Check the BMP signature
		if (header[0] != 'B' || header[1] != 'M')
		{
			file.close();
			return false;
		}

		// Extract width and height from the header
		Width = *(int*)&header[18];
		Height = *(int*)&header[22];
		int colorDepth = *(int*)&header[28]; // Bits per pixel

		// Calculate the number of bytes per pixel (based on color depth)
		int bytesPerPixel = colorDepth / 8;

		// Calculate the size of the pixel data (excluding padding)
		int dataSize = Width * Height * bytesPerPixel;

		// Read pixel data
		std::vector<unsigned char> rawPixelData(dataSize);
		file.read(reinterpret_cast<char*>(rawPixelData.data()), dataSize);

		file.close();

		// If color depth is greater than 3 (such as 4), convert to 3 (24 bits)
		if (bytesPerPixel > 3) {
			// Convert to 24-bit (3 bytes per pixel)
			PixelData.resize(Width * Height * 3);
			for (int i = 0, j = 0; i < dataSize; i += bytesPerPixel, j += 3) {
				PixelData[j] = rawPixelData[i];         // Red
				PixelData[j + 1] = rawPixelData[i + 1]; // Green
				PixelData[j + 2] = rawPixelData[i + 2]; // Blue
			}
		}
		else {
			// Color depth is already 3, no need to convert
			PixelData = std::move(rawPixelData);
		}

		return true;
	}

	bool LoadSPR(const std::string& filename) 
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);

		if (!file) {
			std::cerr << "Failed to open SPR file." << std::endl;
			return false;
		}

		// Read SPR header (assuming a simple format)
		int sprWidth = 0, sprHeight = 0;
		file.read(reinterpret_cast<char*>(&sprWidth), sizeof(int));
		file.read(reinterpret_cast<char*>(&sprHeight), sizeof(int));


		// Ensure that the header was read correctly
		if (sprWidth <= 0 || sprHeight <= 0) {
			std::cerr << "Invalid SPR file format." << std::endl;
			return false;
		}

		// Update the texture width and height
		this->Width = sprWidth;
		this->Height = sprHeight;

		// Read pixel data
		int dataSize = this->Width * this->Height * 3; // Assuming 3 channels (RGB)
		this->PixelData.resize(dataSize);
		file.read(reinterpret_cast<char*>(this->PixelData.data()), dataSize);

		file.close();

		return true;
	}

	public:

	bool Used = false;

	void SetWrapMode(WrappingMode WrapMode)
	{
		this->WrapMode = WrapMode;
	}

	Texture()
	{
		this->Used = false;
		this->Width = 0;
		this->Height = 0;
	}

	Texture(const std::string& Filename, WrappingMode Mode = WrappingMode::Clamp)
	{
		Width = 0;
		Height = 0;
		if (Filename.find(".bmp") != std::string::npos)
		{
			Used = LoadBMP(Filename);
		}
		else if (Filename.find(".spr") != std::string::npos)
		{
			Used = LoadSPR(Filename);
		}

		this->WrapMode = Mode;
	}

	Color GetPixelColor(float u, float v) const 
	{
		if (this->PixelData.empty())
		{
			return Color(255, 0, 0);
		}

		if (WrapMode == WrappingMode::Repeat)
		{
			u = u - std::floor(u); // Repeat mode
			v = v - std::floor(v);
		}
		else if (WrapMode == WrappingMode::MirroredRepeat)
		{
			if (static_cast<int>(std::floor(u)) % 2) { u = 1.0f - (u - std::floor(u)); }
			else { u = u - std::floor(u); }
			if (static_cast<int>(std::floor(v)) % 2) { v = 1.0f - (v - std::floor(v)); }
			else { v = v - std::floor(v); }
		}

		// Calculate pixel coordinates
		int x = PixelRound(u * (float)(this->Width - 1.0f));
		int y = PixelRound(v * (float)(this->Height - 1.0f));

		// Ensure coordinates are within bounds
		x = std::clamp<int>(x, 0, (this->Width - 1));
		y = std::clamp<int>(y, 0, (this->Height - 1));

		// Calculate the index in the pixel data
		int index = (x + this->Width * y) * 3;

		// Extract RGB values
   		float b = static_cast<float>(this->PixelData[index]);
		float g = static_cast<float>(this->PixelData[index + 1]);
		float r = static_cast<float>(this->PixelData[index + 2]);

		return Color(r, g, b);
	}
};

class Material
{
	public:

	static std::vector<Material*> LoadedMaterials;

	Material* FindMaterial(std::string Name)
	{
		for (Material* M : LoadedMaterials)
		{
			if (M->MaterialName == Name)
			{
				return M;
			}
		}

		return nullptr;
	}

	Material()
	{
		Material* RetMat = FindMaterial("DefaultMat");
		if (RetMat == nullptr)
		{
			Material* m = new Material(Vec3(255.0f, 0, 0), Vec3(255.0f * 0.75f, 0.0f, 0.0f), Vec3(255.0f * 0.25f, 0.0f, 0.0f), 96.0f, "DefaultMat");
			LoadedMaterials.push_back(m);
		}
		else
		{
			*this = *RetMat;
		}
	}
	Material(Vec3 AmbientColor, Vec3 DiffuseColor, Vec3 SpecularColor, float Shininess, std::string Name = "")
	{
		this->AmbientColor = AmbientColor;
		this->DiffuseColor = DiffuseColor;
		this->SpecularColor = SpecularColor;
		this->Shininess = Shininess;
		this->MaterialName = Name;
	}

	bool LoadMaterial(std::string MtlFn, std::string MtlName)
	{
		if (FindMaterial(MtlName) != nullptr)
		{
			*this = *FindMaterial(MtlName);
			return true;
		}

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
						else if (propKeyword == "map_Ka")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
								this->TexA = Texture(textureFilePath);
							}
						}
						else if (propKeyword == "map_Kd")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
								//this->TexD = Texture(textureFilePath);
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

		return true;
	}

	Vec3 AmbientColor = Vec3(255.0f * 0.15f, 0, 0);
	Vec3 DiffuseColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	Vec3 SpecularColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	float Shininess = 32.0f;

	Texture TexA = Texture();
	Texture TexD = Texture();

	std::string MaterialName = "DefaultMat";
};

std::vector<Material*> Material::LoadedMaterials;

class Ray;

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

	void ApplyMatrix(const Matrix &MatToApply)
	{
		this->Points[0] *= MatToApply;
		this->Points[1] *= MatToApply;
		this->Points[2] *= MatToApply;
	}

	void ApplyPerspectiveDivide()
	{
		this->TexCoords[0].CorrectPerspective(this->Points[0].w);
		this->TexCoords[1].CorrectPerspective(this->Points[1].w);
		this->TexCoords[2].CorrectPerspective(this->Points[2].w);

		this->Points[0].CorrectPerspective();
		this->Points[1].CorrectPerspective();
		this->Points[2].CorrectPerspective();
	}

	int ClipAgainstPlane(const Vec3& PointOnPlane, const Vec3& PlaneNormalized, Triangle& Out1, Triangle& Out2, bool DebugClip = false)
	{
		Vec3 p0 = this->Points[0].GetVec3();
		Vec3 p1 = this->Points[1].GetVec3();
		Vec3 p2 = this->Points[2].GetVec3();
		TextureCoords& t0 = this->TexCoords[0];
		TextureCoords& t1 = this->TexCoords[1];
		TextureCoords& t2 = this->TexCoords[2];

		float PlanePointDot = PlaneNormalized.Dot(PointOnPlane);
		float dist0 = PlaneNormalized.Dot(p0) - PlanePointDot;
		float dist1 = PlaneNormalized.Dot(p1) - PlanePointDot;
		float dist2 = PlaneNormalized.Dot(p2) - PlanePointDot;

		Vec3* InsidePoints[3] = {};
		Vec3* OutsidePoints[3] = {};
		TextureCoords* InsideTex[3] = {};
		TextureCoords* OutsideTex[3] = {};
		int InsideCount = 0;
		int OutsideCount = 0;
		int InsideTexCount = 0;
		int OutsideTexCount = 0;

		if (dist0 >= 0)
		{
			InsidePoints[InsideCount++] = &p0; InsideTex[InsideTexCount++] = &t0;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p0; OutsideTex[OutsideTexCount++] = &t0;
		}

		if (dist1 >= 0)
		{
			InsidePoints[InsideCount++] = &p1; InsideTex[InsideTexCount++] = &t1;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p1; OutsideTex[OutsideTexCount++] = &t1;
		}

		if (dist2 >= 0)
		{
			InsidePoints[InsideCount++] = &p2; InsideTex[InsideTexCount++] = &t2;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p2; OutsideTex[OutsideTexCount++] = &t2;
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

			Out1.Points[0] = *InsidePoints[0];
			Out1.TexCoords[0] = *InsideTex[0];

			float t = 0.0f;
			Out1.Points[1] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.TexCoords[1] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[1], PointOnPlane, PlaneNormalized, &t);
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


			Out1.Points[0] = *InsidePoints[0];
			Out1.Points[1] = *InsidePoints[1];
			Out1.TexCoords[0] = *InsideTex[0];
			Out1.TexCoords[1] = *InsideTex[1];

			float t = 0.0f;
			Out1.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.TexCoords[2] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out2.Points[0] = *InsidePoints[1];
			Out2.Points[1] = Out1.Points[2];
			Out2.TexCoords[0] = *InsideTex[1];
			Out2.TexCoords[1] = Out1.TexCoords[2];

			Out2.Points[2] = InsidePoints[1]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out2.TexCoords[2] = InsideTex[1]->Lerp(*OutsideTex[0], t);

			Count = 2;
		}

		return Count;
	}

	Vec4 Points[3] = {};
	Vec3 WorldSpaceVerts[3] = {};
	Vec4 ViewSpaceVerts[3] = {};

	Vec3 FaceNormal = Vec3();
	Vec3 VertexNormals[3] = {};
	Vec3 NormalPositions[4] = {};
	Vec3 NormDirections[4] = {};

	TextureCoords TexCoords[3] = {};

	Vec3 Col = Vec3(255, 0, 0);
	Material Mat = Material();

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
		this->ChangeMatInfo(Material());
		this->CalculateNormals();
	}

	Mesh(std::string Fn)
	{
		this->LoadMesh(Fn);
	}

	Mesh(Mesh m, Material Mat)
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

	void ChangeMatInfo(Material MatToApply)
	{
		this->Mat = MatToApply;
		for (auto & Tri : this->Triangles)
		{
			Tri.Mat = this->Mat;

			if(Tri.Mat.TexA.Used || Tri.Mat.TexD.Used)
				Tri.HasTexture = true;
		}
	}

	bool LoadMesh(std::string FnPath)
	{
		std::ifstream ifs = std::ifstream(FnPath);

		if (!ifs.is_open())
			return false;

		std::vector<Vec3> VertCache;
		std::vector<Vec3> NormalCache;
		std::vector<TextureCoords> TexCache;

		std::string MtlLibFn = "";
		Material CurrMat = Material();

		std::cout << "Loading Mesh: " << FnPath << std::endl;

		while (!ifs.eof())
		{
			char Line[128];
			char Unused;
			std::stringstream SS;

			ifs.getline(Line, 128);

			SS << Line;
			std::string Str = SS.str();

			std::cout << "Verts: ";

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

						// Store the texture coordinates for this vertex
						if (texCoordIndex >= 0)
							Tmp.TexCoords[i] = TexCache[texCoordIndex];
						else
							Tmp.TexCoords[i] = {0, 0}; // Default value if no texture coordinates provided

						if (normalIndex >= 0)
							Tmp.FaceNormal = NormalCache[normalIndex];

						Tmp.Points[i] = vertex;
					}

					Tmp.Mat = CurrMat; // Assign the current material to the triangle
					Tmp.HasTexture = Tmp.Mat.TexA.Used || Tmp.Mat.TexA.Used;

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

#ifdef _DEBUG
			std::cout << Vertices.size() << " Normals: ";
			std::cout << Normals.size() << " TexturedCahce: " << TexCache.size();
			std::cout << " Faces: " << Triangles.size() << "\r";
#endif

		}

		FnPath = FnPath.substr(FnPath.find_last_of("/\\") + 1);
		FnPath = FnPath.substr(0, FnPath.find_last_of(".obj") - 3);

		this->MeshName = FnPath;
		this->Vertices = VertCache;
		this->Normals = NormalCache;
		this->NormalCount = (int)NormalCache.size();
		this->TexCoords = TexCache;
		this->TexCoordsCount = (int)TexCache.size();
		this->VertexCount = (int)VertCache.size();
		this->TriangleCount = (int)Triangles.size();

		if (Normals.size() == 0)
			this->CalculateNormals();

#ifdef _DEBUG
		std::cout << Vertices.size() << " Normals: ";
		std::cout << Normals.size() << " TexturedCahce: " << TexCache.size();
		std::cout << " Faces: " << Triangles.size() << "\n\n";
#endif

		return true;
	}

	void CalculateNormals()
	{
		for (Triangle& Tri : this->Triangles)
		{
			Vec3 Norm = (Tri.Points[1] - Tri.Points[0]).GetVec3().CrossNormalized((Tri.Points[2] - Tri.Points[0]));
			this->Normals.push_back(Norm);
			Tri.FaceNormal = Norm;

			for (int i = 0; i < 3; i++)
			{
				Tri.VertexNormals[i] = Norm;
			}
		}

		this->NormalCount = (int)this->Normals.size();
	}

	std::string MeshName = "";
	std::vector<Vec3> Vertices = {};
	std::vector<Triangle> Triangles = {};
	std::vector<TextureCoords> TexCoords = {};
	std::vector<Vec3> Normals = {};

	std::vector<Material> ModelMats = {};

	Material Mat = Material();

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
	Cube(float Length, float Width, float Height, Material Mat = Material(), Texture* T = nullptr) : Mesh(CubeMesh, Mat)
	{
		for (int i = 0; i < CubeMesh.Triangles.size(); i++)
		{
			this->Triangles[i].Points[0] *= Vec3(Length, Width, Height);
			this->Triangles[i].Points[1] *= Vec3(Length, Width, Height);
			this->Triangles[i].Points[2] *= Vec3(Length, Width, Height);

			if (T != nullptr)
			{
				this->Triangles[i].HasTexture = true;
				this->Triangles[i].Mat = Mat;
				this->Triangles[i].Mat.TexA = *T;

				if ((i+1) % 2 == 0)
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
		}
	}

};

class Sphere : public Mesh
{
	public:
	Sphere(float Radius, int LatitudeSegments, int LongitudeSegments, Material Mat = Material()) : Mesh()
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

				this->Vertices.push_back(Vec3(x, y, z));
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

				this->Triangles.push_back(Triangle(this->Vertices[currentVertex], this->Vertices[topVertex], this->Vertices[nextVertex]));

				this->Triangles.push_back(Triangle(this->Vertices[nextVertex], this->Vertices[topVertex], this->Vertices[topNextVertex]));

				this->VertexCount = (int)Vertices.size();
				this->TriangleCount = (int)Triangles.size();
			}
		}
		this->CalculateNormals();
		this->ChangeMatInfo(Mat);
	}
};

class LightObject
{
	public:
	LightObject(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, float Far = 0.1f, float Near = 150.0f)
	{
		this->LightPos = Pos;
		this->LightDir = LightDir;
		this->Color = LightColor;
		
		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;

		this->LightMesh = CubeMesh;
		this->LightMesh.Mat.AmbientColor = LightColor;
		this->LightMesh.Mat.DiffuseColor = LightColor;
		this->LightMesh.Mat.SpecularColor = LightColor;

		this->Far = Far;
		this->Near = Near;

		this->Render = false;
	}

	LightObject(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, float Far = 0.1f, float Near = 150.0f)
	{
		this->LightPos = Pos;
		this->LightDir = LightDir;
		this->Color = LightColor;
		
		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;
	
		this->LightMesh = LightMesh;

		this->Far = Far;
		this->Near = Near;
		this->Render = false;
	}

	virtual Matrix CalcVpMat() = 0;

	Vec3 LightDir = Vec3(0, 0, -1);
	Vec3 LightPos = Vec3(0, 0, 0);
	Vec3 Color = Vec3(253, 251, 211);
	Mesh LightMesh = Mesh();
	bool Render = false;

	float AmbientCoeff = 0.1f;  // Lowest level of light possible (only the objects mat ambient color)
	float SpecularCoeff = 0.5f; // How much the lights color will combine with the objects specular color
	float DiffuseCoeff = 0.25f; // How much the light will combine with the objects diffuse mat color

	float Far = 0.1f;
	float Near = 150.0f;
};

class PointLight: public LightObject
{

};

class DirectionalLight: public LightObject
{
	public:
	float Left = -40.0f;
	float Right = 40.0f;
	float Top = 40.0f;
	float Bottom = -40.0f;
	Vec3 CenterPoint = Vec3();

	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float Left, float Right, float Bottom, float Top, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->Left = Left;
		this->Right = Right;
		this->Top = Bottom;
		this->Bottom = Top;
		this->CenterPoint = CenterPoint;
	}
	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float Left, float Right, float Bottom, float Top, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->Left = Left;
		this->Right = Right;
		this->Top = Bottom;
		this->Bottom = Top;
		this->CenterPoint = CenterPoint;
	}

	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->CenterPoint = CenterPoint;
	}

	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->CenterPoint = CenterPoint;
	}

	__inline Matrix CalcVpMat() override
	{
		Matrix LightProjectionMat = Matrix::CalcOrthoMatrix(Left, Right, Bottom, Top, Far, Near);
		Matrix LightViewMatrix = Matrix::CalcViewMatrix(((this->CenterPoint - this->LightPos).Normalized()) * 50.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
		return LightViewMatrix * LightProjectionMat;
	}
};

class SpotLight: public LightObject
{
	public:
	float CutoffAngle = 0.0f;
	float ConstantAttenuation = 0.0f;
	float LinearAttenuation = 0.0f;
	float QuadraticAttenuation = 0.0f;

	SpotLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float CutoffAngle, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->CutoffAngle = CutoffAngle;
		this->ConstantAttenuation = ConstantAttenuation;
		this->LinearAttenuation = LinearAttenuation;
		this->QuadraticAttenuation = QuadraticAttenuation;
	}

	//SpotLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, float CutoffAngle, float ConstantAttenuation = 0.0f, float LinearAttenuation = 0.0f, float QuadraticAttenuation = 0.0f) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	//{
	//	this->CutoffAngle = CutoffAngle;
	//	this->ConstantAttenuation = ConstantAttenuation;
	//	this->LinearAttenuation = LinearAttenuation;
	//	this->QuadraticAttenuation = QuadraticAttenuation;
	//}

	__inline Matrix CalcVpMat() override
	{

	}
};

class Camera
{
public:
	Camera()
	{

	}

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
		this->PointAt(TargetLook);
	}

	// Calculate the projection matrix with some screen args
	static Matrix CalcProjectionMat(float AspectRatio, float Fov, float Near, float Far)
	{
		Matrix Result = {};

		float FovRads = 1.0f / tanf(ToRad(Fov * 0.5f));

		Result.fMatrix[0][0] = AspectRatio / FovRads;
		Result.fMatrix[1][1] = -FovRads;  // THIS NEGATIVE IS FOR TERRAGL
		Result.fMatrix[2][2] = Far / (Far - Near);
		Result.fMatrix[3][2] = (-Far * Near) / (Far - Near);
		Result.fMatrix[2][3] = 1.0f;
		Result.fMatrix[3][3] = 0.0f;

		return Result;
	}

	//Calculate projection matrix of this camera
	void CalcProjectionMat()
	{
		float FovRads = abs(1.0f / tanf((ToRad(Fov * 0.5f))));

		this->ProjectionMatrix.fMatrix[0][0] = this->AspectRatio / FovRads;
		this->ProjectionMatrix.fMatrix[1][1] = -FovRads; // THIS NEGATIVE IS FOR TERRAGL
		this->ProjectionMatrix.fMatrix[2][2] = this->Far / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[3][2] = (-this->Far * this->Near) / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[2][3] = 1.0f;
		this->ProjectionMatrix.fMatrix[3][3] = 0.0f;
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

	static Matrix PointAt(const Vec3 &CamPos, const Vec3 &Target, const Vec3 &Up)
	{
		Vec3 NewForward = (Target - CamPos).Normalized();

		Vec3 NewUp = (Up - (NewForward * Up.Dot(NewForward))).Normalized();

		Vec3 NewRight = NewUp.Cross(NewForward);

		Matrix DimensioningAndTrans;
		DimensioningAndTrans.fMatrix[0][0] = NewRight.x;	    DimensioningAndTrans.fMatrix[0][1] = NewRight.y;	    DimensioningAndTrans.fMatrix[0][2] = NewRight.z;      DimensioningAndTrans.fMatrix[0][3] = 0.0f;
		DimensioningAndTrans.fMatrix[1][0] = NewUp.x;		    DimensioningAndTrans.fMatrix[1][1] = NewUp.y;		    DimensioningAndTrans.fMatrix[1][2] = NewUp.z;         DimensioningAndTrans.fMatrix[1][3] = 0.0f;
		DimensioningAndTrans.fMatrix[2][0] = NewForward.x;		DimensioningAndTrans.fMatrix[2][1] = NewForward.y;		DimensioningAndTrans.fMatrix[2][2] = NewForward.z;    DimensioningAndTrans.fMatrix[2][3] = 0.0f;
		DimensioningAndTrans.fMatrix[3][0] = CamPos.x;			DimensioningAndTrans.fMatrix[3][1] = CamPos.y;	    	DimensioningAndTrans.fMatrix[3][2] = CamPos.z;        DimensioningAndTrans.fMatrix[3][3] = 1.0f;

		return DimensioningAndTrans;
	}

	void __inline __fastcall PointAt(const Vec3& Target)
	{
		Vec3 NewForward = (Target - this->Pos).Normalized();

		Vec3 NewUp = (this->CamUp - (NewForward * this->CamUp.Dot(NewForward))).Normalized();

		Vec3 NewRight = NewUp.Cross(NewForward);

		this->ViewMatrix.fMatrix[0][0] = NewRight.x;	    this->ViewMatrix.fMatrix[0][1] = NewRight.y;	    this->ViewMatrix.fMatrix[0][2] = NewRight.z;      this->ViewMatrix.fMatrix[0][3] = 0.0f;
		this->ViewMatrix.fMatrix[1][0] = NewUp.x;		    this->ViewMatrix.fMatrix[1][1] = NewUp.y;		    this->ViewMatrix.fMatrix[1][2] = NewUp.z;         this->ViewMatrix.fMatrix[1][3] = 0.0f;
		this->ViewMatrix.fMatrix[2][0] = NewForward.x;		this->ViewMatrix.fMatrix[2][1] = NewForward.y;		this->ViewMatrix.fMatrix[2][2] = NewForward.z;    this->ViewMatrix.fMatrix[2][3] = 0.0f;
		this->ViewMatrix.fMatrix[3][0] = this->Pos.x;		this->ViewMatrix.fMatrix[3][1] = this->Pos.y;	    this->ViewMatrix.fMatrix[3][2] = this->Pos.z;     this->ViewMatrix.fMatrix[3][3] = 1.0f;
	}

	void __inline __fastcall CalcCamViewMatrix(const Vec3 &Target)
	{
		//this->ViewMatrix = this->PointAt(this->Pos, Target, this->CamUp).QuickInversed();
		this->PointAt(Target);
		this->ViewMatrix.QuickInverse();
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
	Vec3 NearPlane = { 0, 0, 0.1f };
	float Velocity = 8.0f;

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};
	Matrix CamRotation = {};
	float Fov = 0.f;
	float AspectRatio = 0.f;
	float Near = 0.f;
	float Far = 0.f;
};

class Ray
{
	//Ray(t) = (ox + t * dx, oy + t * dy, oz + t * dz)
	public:

	Ray(const Vec3 &OriginPoint, const Vec3 &Direction, const Vec3 SpeedMult = Vec3(1, 1, 1))
	{
		this->Origin = OriginPoint;
		this->Direction = Direction;
		this->SpeedMult = SpeedMult;
	}

	void GenerateMesh()
	{
		const int NumLineSegments = 20;  // Number of line segments
		const float LineRadius = 0.01f;  // Radius of the line segments

		std::vector<Triangle> Tris;

		// Generate triangles for the line mesh
		for (int i = 0; i < NumLineSegments; ++i) 
		{
			float t1 = float(i) / float(NumLineSegments - 1);
			float t2 = float(i + 1) / float(NumLineSegments - 1);

			// Calculate the positions of the vertices along the ray's path
			Vec3 Vertex1 = this->Origin + this->Direction * t1;
			Vec3 Vertex2 = this->Origin + this->Direction * t2;

			// Calculate perpendicular vectors for the line segments
			Vec3 Tangent = this->Direction.Normalized();
			Vec3 Bitangent = this->Direction.Cross(Vec3(0, 1, 0));
			Vec3 Normal = Tangent.Cross(Bitangent);

			// Calculate vertices for the triangles
			Vec3 tri1Vertices[3] = {
				Vertex1 - Bitangent * LineRadius,
				Vertex1 + Bitangent * LineRadius,
				Vertex2 - Bitangent * LineRadius
			};

			Vec3 tri2Vertices[3] = {
				Vertex2 - Bitangent * LineRadius,
				Vertex1 + Bitangent * LineRadius,
				Vertex2 + Bitangent * LineRadius
			};

			// Create triangles and add them to the array
			Triangle tri1(tri1Vertices[0], tri1Vertices[1], tri1Vertices[2]);
			Triangle tri2(tri2Vertices[0], tri2Vertices[1], tri2Vertices[2]);

			Tris.push_back(tri1);
			Tris.push_back(tri2);
		}

		mesh = Mesh(Tris, "Ray");
		mesh.BackfaceCulling = false;
		mesh.Mat.AmbientColor = Vec3(0, 255, 0);
	}

	void Step(float DeltaTime)
	{
		// Update the ray's position based on its direction and speed multiplier
		Origin.x += (Direction.x * SpeedMult.x) * DeltaTime;
		Origin.y += (Direction.y * SpeedMult.y) * DeltaTime;
		Origin.z += (Direction.z * SpeedMult.z) * DeltaTime;
	}

	Vec3 LocationAt(float T) const
	{
		// Calculate the location of the ray at time T
		return Vec3(
			Origin.x + (Direction.x * SpeedMult.x) * T,
			Origin.y + (Direction.y * SpeedMult.y) * T,
			Origin.z + (Direction.z * SpeedMult.z) * T);
	}

	Mesh mesh;
	Vec3 Origin = {0, 0, 0};
	Vec3 Direction = {1.0f, 0, 0};
	Vec3 SpeedMult = { 1, 1, 1 };
};

template<typename T>
class BufferObject
{
	__int64 Width;
	__int64 Height;
	T* Buffer = nullptr;

	BufferObject(__int64 Width, __int64 Height, __int64 Depth)
	{
		Buffer = new T[Width * Height];
	}

	~BufferObject()
	{
		delete[] Buffer;
	}
};

struct ShaderArgs
{
	ShaderTypes ShaderType = ShaderTypes::SHADER_TRIANGLE;
	LightTypes LightType = LightTypes::DirectionalLight;

	// Triangle Info (ALL SHADERS)
	Triangle* Tri = nullptr;
	const Material* Mat = nullptr;

	//Camera Info (ALL SHADERS)
	Matrix ModelMat;
	Matrix ViewMat;
	Matrix ProjectionMat;
	Vec3 CamPos = Vec3(0, 0, 0);
	Vec3 CamLookDir = Vec3(0, 0, 0);

	// Light Info (ALL SHADERS)
	LightObject** Lights;
	size_t LightCount;

	// Fragment Info (SHADER_TYPE == SHADER_FRAGMENT)
	Vec3 FragPos = Vec3(0, 0, 0);
	Vec3 FragNormal = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 BaryCoords = Vec3(0, 0, 0);
	Color FragColor = Color(0.0f, 0.0f, 0.0f);
	Vec2 PixelCoords = Vec2(0, 0);
	TextureCoords UVW = { 0.0f, 0.0f, 0.0f };

	// ShadowInfo
	Vec3 LightSpacePos;
	bool IsInShadow = false;

	ShaderArgs() 
	{

	}

	ShaderArgs(Triangle* Tri, const Material* Mat, const Vec3& CamPos, const Vec3& CamLookDir, const Matrix& ModelMatrix, const Matrix& ViewMatrix, const Matrix& ProjMatrix, LightObject** Lights, const size_t LightCount, const ShaderTypes SHADER_TYPE)
	{
		this->Tri = Tri;
		this->Mat = Mat;
		this->CamPos = CamPos;
		this->CamLookDir = CamLookDir;
		this->Lights = Lights;
		this->LightCount = LightCount;
		this->ShaderType = SHADER_TYPE;
		this->ModelMat = ModelMatrix;
		this->ViewMat = ViewMatrix;
		this->ProjectionMat = ProjMatrix;
	}
};

struct VertexShaderArgs
{
	Vec3 Vertex;
	Matrix MVP;

};

namespace EngineShaders
{
	const auto DefaultVertexShader = [](ShaderArgs& Args)
	{

	};

	const auto WHACK_SHADER = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}

		Vec3 LDir = (Light->LightPos - ((Args.Tri->Points[0] + Args.Tri->Points[1] + Args.Tri->Points[2]) / 3.0f)).Normalized();

		float Li = std::max<float>(0.0f, Args.Tri->FaceNormal.Dot(LDir));

		Vec3 AmbientCol = Args.Mat->AmbientColor * Light->AmbientCoeff;
		Vec3 DiffuseCol = Args.Mat->DiffuseColor * Li;

		Args.Tri->Col = (AmbientCol + DiffuseCol) * Light->Color;
	};

	const auto Shader_Phong_LOW_LOD = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}

		Vec3 LDir = (Light->LightPos - ((Args.Tri->Points[0] + Args.Tri->Points[1] + Args.Tri->Points[2]) / 3.0f)).Normalized();

		float Li = std::max<float>(0.0f, Args.Tri->FaceNormal.Dot(LDir));

		Vec3 AmbientCol = (Args.Mat->AmbientColor * Light->AmbientCoeff);
		Vec3 DiffuseCol = ((Light->Color * Li) + (Args.Mat->DiffuseColor * Li)) * Light->DiffuseCoeff;

		Args.Tri->Col.x = std::clamp<float>((AmbientCol.x + DiffuseCol.x), 0.0f, 255.0f);
		Args.Tri->Col.y = std::clamp<float>((AmbientCol.y + DiffuseCol.y), 0.0f, 255.0f);
		Args.Tri->Col.z = std::clamp<float>((AmbientCol.z + DiffuseCol.z), 0.0f, 255.0f);
	};

	const auto Shader_Phong = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - ((Args.Tri->Points[0] + Args.Tri->Points[1] + Args.Tri->Points[2]) / 3.0f)).Normalized();
		float Li = Args.Tri->FaceNormal.Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);
		Vec3 RDir = LDir - Args.Tri->FaceNormal * 2.0f * Li;

		float SpecularIntensity = pow(std::max<float>(0.0f, RDir.Dot(Args.CamLookDir)), Args.Mat->Shininess);

		Vec3 AmbientCol = (Args.Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Args.Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Args.Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		Args.Tri->Col.x = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		Args.Tri->Col.y = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		Args.Tri->Col.z = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
	};

	const auto Shader_Material = [](ShaderArgs& Args)
	{
		Args.Tri->Col = Args.Mat->AmbientColor;
	};

	const auto Shader_Frag_Phong = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		Vec3 LDir = Light->LightPos.GetDirectionToVector(Args.FragPos);
		float Li = Args.FragNormal.Dot(LDir);

		// Calculate the reflection direction using the formula: R = I - 2 * (I dot N) * N
		float Intensity = std::max<float>(0.0f, Li);
		//Vec3 RDir = (LDir - Args.FragNormal * 2.0f * Li).Normalized();
		Vec3 RDir = (-LDir).GetReflectection(Args.FragNormal);

		// Calculate the specular intensity
		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(Args.CamLookDir)), Args.Mat->Shininess);

		Vec3 AmbientCol = (Args.Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Args.Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Args.Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		Args.FragColor.R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
		Args.FragColor.A = 255.0f;
	};

	const auto Shader_Frag_Phong_Shadows = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		Vec3 LDir = Light->LightPos.GetDirectionToVector(Args.FragPos);
		float Li = Args.FragNormal.Dot(LDir);

		// Calculate the reflection direction using the formula: R = I - 2 * (I dot N) * N
		float Intensity = std::max<float>(0.0f, Li);
		//Vec3 RDir = (LDir - Args.FragNormal * 2.0f * Li).Normalized();
		Vec3 RDir = (-LDir).GetReflectection(Args.FragNormal);

		// Calculate the specular intensity
		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(Args.CamLookDir)), Args.Mat->Shininess);

		Vec3 AmbientCol = (Args.Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Args.Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Args.Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		float ShadowMult = 1.0f;

		if (Args.IsInShadow)
		{
			ShadowMult = 0.5f;
		}

		Args.FragColor.R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x) * ShadowMult, 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y) * ShadowMult, 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z) * ShadowMult, 0.0f, 255.0f);
		Args.FragColor.A = 255.0f;
	};

	const auto Shader_Gradient = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - Args.FragPos).Normalized();
		float Li = Args.FragNormal.Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);

		Args.FragColor.R = std::clamp<float>(((255.0f * Args.BaryCoords.x) * Intensity), 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>(((255.0f * Args.BaryCoords.y) * Intensity), 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>(((255.0f * Args.BaryCoords.z) * Intensity), 0.0f, 255.0f);
		Args.FragColor.A = 255.0f;
	};

	const auto Shader_Gradient_Centroid = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		Vec3 Centroid = ((Args.Tri->Points[0] + Args.Tri->Points[1] + Args.Tri->Points[2]) / 3.0f);
		Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Args.PixelCoords, Vec2(Centroid.x, Centroid.y), Vec2(Args.Tri->Points[1].x, Args.Tri->Points[1].y), Vec2(Args.Tri->Points[2].x, Args.Tri->Points[2].y));
		Vec3 BaryCoords2 = CalculateBarycentricCoordinatesScreenSpace(Args.PixelCoords, Vec2(Args.Tri->Points[0].x, Args.Tri->Points[0].y), Vec2(Args.Tri->Points[1].x, Args.Tri->Points[1].y), Vec2(Centroid.x, Centroid.y));

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - Args.FragPos).Normalized();
		float Li = Args.FragNormal.Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);

		Args.FragColor.R = std::clamp<float>(((255.0f * BaryCoords2.x) * Intensity), 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>(((255.0f * BaryCoords.y) * Intensity), 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>(((255.0f * BaryCoords.z) * Intensity), 0.0f, 255.0f);
		Args.FragColor.A = 255.0f;
	};

	const auto Shader_Tex_Phong = [](ShaderArgs& Args)
	{
		LightObject* Light = nullptr;
		if (Args.LightCount > 0)
		{
			Light = Args.Lights[0];
		}
		else
		{
			return;
		}

		Vec3 TexturCol = Vec3();

		if (Args.Tri->Mat.TexA.Used)
		{
			TexturCol = Args.Tri->Mat.TexA.GetPixelColor(Args.UVW.u, 1.0f - Args.UVW.v).GetRGB();
			Args.FragColor.R = std::clamp<float>(TexturCol.x, 0.0f, 255.0f);
			Args.FragColor.G = std::clamp<float>(TexturCol.y, 0.0f, 255.0f);
			Args.FragColor.B = std::clamp<float>(TexturCol.z, 0.0f, 255.0f);
			Args.FragColor.A = 255.0f;
		}
		else
		{
			Shader_Frag_Phong(Args);
		}
		Vec3 LDir = Light->LightPos.GetDirectionToVector(Args.FragPos);
		float Li = Args.FragNormal.Dot(LDir);

		// Calculate the reflection direction using the formula: R = I - 2 * (I dot N) * N
		float Intensity = std::max<float>(0.0f, Li);
		//Vec3 RDir = (LDir - Args.FragNormal * 2.0f * Li).Normalized();
		Vec3 RDir = (-LDir).GetReflectection(Args.FragNormal);

		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(Args.CamLookDir)), Args.Mat->Shininess);

		Vec3 AmbientCol = (TexturCol) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (TexturCol * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (TexturCol * Light->SpecularCoeff)) * SpecularIntensity;

		Args.FragColor.R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
		Args.FragColor.A = 255.0f;
	};

	const auto Shader_Texture_Only = [](ShaderArgs& Args)
	{
		Vec3 TexturCol = Vec3();

		if (Args.Tri->Mat.TexA.Used)
		{
			TexturCol = Args.Tri->Mat.TexA.GetPixelColor(Args.UVW.u, 1.0f - Args.UVW.v).GetRGB();
			Args.FragColor.R = std::clamp<float>(TexturCol.x, 0.0f, 255.0f);
			Args.FragColor.G = std::clamp<float>(TexturCol.y, 0.0f, 255.0f);
			Args.FragColor.B = std::clamp<float>(TexturCol.z, 0.0f, 255.0f);
			Args.FragColor.A = 255.0f;
		}
		else
		{
			Shader_Frag_Phong(Args);
		}
	};
}

class Renderable
{
	public:
	Renderable() = delete;

	Renderable(const Mesh mesh, const Camera& Cam, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, const std::function<void(ShaderArgs&)> Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		this->Cam = Cam;
		this->Pos = Pos;
		this->mesh = mesh;
		this->Scalar = Scalar;
		this->RotationRads = RotationRads;
		this->Shader = Shader;
		this->SHADER_TYPE = SHADER_TYPE;
	}

	Mesh mesh = Mesh();
	Vec3 Pos = Vec3();
	Vec3 Scalar = Vec3();
	Vec3 RotationRads = Vec3();
	Camera Cam;

	std::function<void(ShaderArgs&)> Shader = EngineShaders::Shader_Frag_Phong_Shadows;
	ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT;
};

typedef void(__fastcall* DoTick_T)(const GdiPP&, const WndCreator&, const float&, std::vector<Renderable>*, std::vector<LightObject*>*);

namespace TerraPGE
{
	std::string FpsStr = "Fps: ";
	std::wstring FpsWStr = L"Fps: ";

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	float FOV = 90.0f;
	float FNEAR = 0.1f;
	float FFAR = 150.0f;
	float ShadowMapBias = FLOAT_LOWEST;
	int ShadowMapHeight = 1024;
	int ShadowMapWidth = 1024;

	bool FpsEngineCounter = true;
	bool DoMultiThreading = false;
	bool DoCull = true;
	bool DoLighting = true;
	bool DoShadows = true;
	bool WireFrame = false;
    bool ShowTriLines = false;
	bool DebugClip = false;
	bool LockCursor = true;
	bool CursorShow = false;
	bool UpdateMouseIn = true;
	bool ShowNormals = false;
	bool DebugDepthBuffer = false;
	bool DebugShadowMap = false;
	bool NormalMapping = false;

	int CpuCores = 0;
	int GPUDevCount = 0;
	std::vector<std::wstring> GPUDevNames = {};
	std::wstring PrimaryGPUDevName;
	SIZE_T MaxMemoryMB = 0;
	ThreadManager ThreadPool;
#ifdef SSE_SIMD_42_SUPPORT
	CPUID CpuId(1);
	SupportedInstructions SimdInfo = { 0 };
#endif

	Vec2 PrevMousePos;
	Vec2 DeltaMouse;
	POINT Tmp;
	float Sensitivity = 0.1f;
	int Fps = 0;

	float* DepthBuffer = new float[sx * sy];
	float* ShadowMap = new float[ShadowMapWidth * ShadowMapHeight];

	SIZE_T GetUsedMemory()
	{
		PROCESS_MEMORY_COUNTERS_EX  Pmc;

		if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&Pmc, sizeof(Pmc)))
			return Pmc.PrivateUsage / 1024 / 1024;

		return 0;
	}

	void UpdateSystemInfo()
	{
		SYSTEM_INFO SysInf;
		MEMORYSTATUSEX MemInf;
		DISPLAY_DEVICE DispDev;

		DispDev.cb = sizeof(DispDev);
		MemInf.dwLength = sizeof(MEMORYSTATUSEX);
		int DevIdx = 0;

		GetSystemInfo(&SysInf);
		TerraPGE::CpuCores = SysInf.dwNumberOfProcessors;

		if (GlobalMemoryStatusEx(&MemInf))
			TerraPGE::MaxMemoryMB = (MemInf.ullTotalPhys / 1024 / 1024);

		if (EnumDisplayDevices(NULL, DevIdx, &DispDev, 0)) 
		{
			TerraPGE::GPUDevNames.push_back(DispDev.DeviceString);
			TerraPGE::GPUDevCount++;

			if (DispDev.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
				TerraPGE::PrimaryGPUDevName = DispDev.DeviceString;
		}

	}

	// Update screen info
	void UpdateScreenInfo(GdiPP& Gdi)
	{
		// Update GDI info
		Gdi.UpdateClientRgn();

		// Update screen dimenstions
		sx = Gdi.ClientRect.right - Gdi.ClientRect.left;
		sy = Gdi.ClientRect.bottom - Gdi.ClientRect.top;

		// Update DepthBuffer
		delete[] DepthBuffer;
		DepthBuffer = new float[sx * sy];
	}

	// Delete buffers
	void EngineCleanup()
	{
		// Cleanup buffers
		delete[] DepthBuffer;
		delete[] ShadowMap;
	}

	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	template<typename T>
	void __fastcall RenderTriangleThreaded(float* DepthBuffer, GdiPP& Gdi, T&& Shader, ShaderArgs Args)
	{
		// Setup variables
		Triangle& Tri = *Args.Tri;
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;
		Matrix Vp = (Args.ViewMat * Args.ProjectionMat);

		int x1 = PixelRound(Tri.Points[0].x);
		int y1 = PixelRound(Tri.Points[0].y);
		int x2 = PixelRound(Tri.Points[1].x);
		int y2 = PixelRound(Tri.Points[1].y);
		int x3 = PixelRound(Tri.Points[2].x);
		int y3 = PixelRound(Tri.Points[2].y);
		float u1 = 0.0f;
		float u2 = 0.0f;
		float u3 = 0.0f;
		float w1 = 0.0f;
		float w2 = 0.0f;
		float w3 = 0.0f;
		float v1 = 0.0f;
		float v2 = 0.0f;
		float v3 = 0.0f;

		if (Tri.Mat.TexA.Used)
		{
			u1 = Tri.TexCoords[0].u;
			u2 = Tri.TexCoords[1].u;
			u3 = Tri.TexCoords[2].u;
			v1 = Tri.TexCoords[0].v;
			v2 = Tri.TexCoords[1].v;
			v3 = Tri.TexCoords[2].v;
			w1 = Tri.TexCoords[0].w;
			w2 = Tri.TexCoords[1].w;
			w3 = Tri.TexCoords[2].w;
		}

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);
		if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
				{
					for (int j = ax; j < bx; j++)
					{
						tex_u = (1.0f - t) * tex_su + t * tex_eu;
						tex_v = (1.0f - t) * tex_sv + t * tex_ev;
						tex_w = (1.0f - t) * tex_sw + t * tex_ew;
						t += tstep;

						int idx = ContIdx(j, i, TerraPGE::sx);

						// Calculate the barycentric coordinates which we use for interpolation
						Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
						// Use Barycentric coords to interpolate our world position
						Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

						// World Pos -> Clipped Space
						Vec4 NdcPos = InterpolatedPos * Vp;
						// Clipped -> Ndc
						NdcPos.CorrectPerspective();

						// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
						// this line also normalizes that value to [0, 1]
						float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						// Depth test
						if (Depth < DepthBuffer[idx])
							DepthBuffer[idx] = Depth;
						else
							continue; // Don't draw pixel

						float ShadowValue = 0.0f;
						float ShadowDepth = 0.0f;
						int MapIdx = 0;

						if (DoShadows)
						{
							// Make this better TODO
							Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
							
							
							ShadowNdcPos.CorrectPerspective();

							ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
							ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

							MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

							ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
							ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							if (ShadowDepth > ShadowValue)
							{
								Args.IsInShadow = true;
							}
							else
							{
								Args.IsInShadow = false;
							}
						}

						// Debug depth buffer (Grayscale the pixel * depth val)
						if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
						{
							// I plan to add a way to visualize the shadow map here
							float Val = Depth;
							if (TerraPGE::DebugShadowMap)
							{
								if (ShadowDepth > ShadowValue)
								{
									Val = 0.3f;
								}
								else
								{
									Val = TerraPGE::ShadowMap[MapIdx];
								}
							}

							Args.FragColor.R = 255.0f * Val;
							Args.FragColor.G = 255.0f * Val;
							Args.FragColor.B = 255.0f * Val;
						}
						else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
						{
							// This entire else if is mainly for debugging clipping
							if (Tri.OverrideTextureColor)
							{
								Args.FragColor.R = Tri.Col.x;
								Args.FragColor.G = Tri.Col.y;
								Args.FragColor.B = Tri.Col.z;
							}
							else if (Tri.Mat.TexA.Used)
							{
								Vec3 TexturCol = Args.Tri->Mat.TexA.GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
								Args.FragColor.R = TexturCol.x;
								Args.FragColor.G = TexturCol.y;
								Args.FragColor.B = TexturCol.z;
							}
							else
							{
								Args.FragColor.R = Tri.Mat.AmbientColor.x;
								Args.FragColor.G = Tri.Mat.AmbientColor.y;
								Args.FragColor.B = Tri.Mat.AmbientColor.z;
							}
						}
						else
						{
							Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

							// Set up some shader args and call fragment shader=
							Args.FragPos = InterpolatedPos;
							Args.FragNormal = InterpolatedNormal;
							Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
							Args.BaryCoords = BaryCoords;
							Args.PixelCoords = Vec2((float)j, (float)i);
							Shader(Args);
						}

						// Set pixel in pixel buffer
						COLORREF PixelClr = RGB(PixelRoundFloor(Args.FragColor.R), PixelRoundFloor(Args.FragColor.G), PixelRoundFloor(Args.FragColor.B));
						Gdi.QuickSetPixel(j, i, PixelClr);
					}
				};
				TerraPGE::ThreadPool.EnqueueTask(Func);
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
				{
					for (int j = ax; j < bx; j++)
					{
						tex_u = (1.0f - t) * tex_su + t * tex_eu;
						tex_v = (1.0f - t) * tex_sv + t * tex_ev;
						tex_w = (1.0f - t) * tex_sw + t * tex_ew;
						t += tstep;

						int idx = ContIdx(j, i, TerraPGE::sx);

						// Calculate the barycentric coordinates which we use for interpolation
						Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
						// Use Barycentric coords to interpolate our world position
						Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

						// World Pos -> Clipped Space
						Vec4 NdcPos = InterpolatedPos * Vp;
						// Clipped -> Ndc
						NdcPos.CorrectPerspective();

						// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
						// this line also normalizes that value to [0, 1]
						float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						// Depth test
						if (Depth < DepthBuffer[idx])
							DepthBuffer[idx] = Depth;
						else
							continue; // Don't draw pixel

						float ShadowValue = 0.0f;
						float ShadowDepth = 0.0f;
						int MapIdx = 0;

						if (DoShadows)
						{
							Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
							ShadowNdcPos.CorrectPerspective();

							ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
							ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

							MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

							ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
							ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							if (ShadowDepth > ShadowValue)
							{
								Args.IsInShadow = true;
							}
							else
							{
								Args.IsInShadow = false;
							}
						}

						// Debug depth buffer (Grayscale the pixel * depth val)
						if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
						{
							// I plan to add a way to visualize the shadow map here
							float Val = Depth;
							if (TerraPGE::DebugShadowMap)
							{
								if (ShadowDepth > ShadowValue)
								{
									Val = 0.3f;
								}
								else
								{
									Val = TerraPGE::ShadowMap[MapIdx];
								}
							}

							Args.FragColor.R = 255.0f * Val;
							Args.FragColor.G = 255.0f * Val;
							Args.FragColor.B = 255.0f * Val;
						}
						else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
						{
							// This entire else if is mainly for debugging clipping
							if (Tri.OverrideTextureColor)
							{
								Args.FragColor.R = Tri.Col.x;
								Args.FragColor.G = Tri.Col.y;
								Args.FragColor.B = Tri.Col.z;
							}
							else if (Tri.Mat.TexA.Used)
							{
								Vec3 TexturCol = Args.Tri->Mat.TexA.GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
								Args.FragColor.R = TexturCol.x;
								Args.FragColor.G = TexturCol.y;
								Args.FragColor.B = TexturCol.z;
							}
							else
							{
								Args.FragColor.R = Tri.Mat.AmbientColor.x;
								Args.FragColor.G = Tri.Mat.AmbientColor.y;
								Args.FragColor.B = Tri.Mat.AmbientColor.z;
							}
						}
						else
						{
							Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

							// Set up some shader args and call fragment shader=
							Args.FragPos = InterpolatedPos;
							Args.FragNormal = InterpolatedNormal;
							Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
							Args.BaryCoords = BaryCoords;
							Args.PixelCoords = Vec2((float)j, (float)i);
							Shader(Args);
						}

						// Set pixel in pixel buffer
						COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
						Gdi.QuickSetPixel(j, i, PixelClr);
					}
				};

				TerraPGE::ThreadPool.EnqueueTask(Func);
			}
		}

		TerraPGE::ThreadPool.WaitUntilAllTasksFinished();
	}

	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	template<typename T>
	void __fastcall RenderTriangle(float* DepthBuffer, GdiPP& Gdi, T&& Shader, ShaderArgs Args)
	{
		// Setup variables
		Triangle& Tri = *Args.Tri;
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;
		Matrix Vp = (Args.ViewMat * Args.ProjectionMat);

		int x1 = PixelRound(Tri.Points[0].x);
		int y1 = PixelRound(Tri.Points[0].y);
		int x2 = PixelRound(Tri.Points[1].x);
		int y2 = PixelRound(Tri.Points[1].y);
		int x3 = PixelRound(Tri.Points[2].x);
		int y3 = PixelRound(Tri.Points[2].y);
		float u1 = 0.0f;
		float u2 = 0.0f;
		float u3 = 0.0f;
		float v1 = 0.0f;
		float v2 = 0.0f;
		float v3 = 0.0f;
		float w1 = 0.0f;
		float w2 = 0.0f;
		float w3 = 0.0f;

		if (Tri.Mat.TexA.Used)
		{
			u1 = Tri.TexCoords[0].u;
			u2 = Tri.TexCoords[1].u;
			u3 = Tri.TexCoords[2].u;
			v1 = Tri.TexCoords[0].v;
			v2 = Tri.TexCoords[1].v;
			v3 = Tri.TexCoords[2].v;
			w1 = Tri.TexCoords[0].w;
			w2 = Tri.TexCoords[1].w;
			w3 = Tri.TexCoords[2].w;
		}

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);
		if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					tex_w = (1.0f - t) * tex_sw + t * tex_ew;
					t += tstep;

					int idx = ContIdx(j, i, TerraPGE::sx);

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

					// World Pos -> Clipped Space
					Vec4 NdcPos = InterpolatedPos * Vp;
					// Clipped -> Ndc
					NdcPos.CorrectPerspective();

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					// Depth test
					if (Depth < DepthBuffer[idx])
						DepthBuffer[idx] = Depth;
					else
						continue; // Don't draw pixel

					float ShadowValue = 0.0f;
					float ShadowDepth = 0.0f;
					int MapIdx = 0;

					if (DoShadows)
					{
						Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
						ShadowNdcPos.CorrectPerspective();

						ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
						ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

						ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
						ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						if (ShadowDepth > ShadowValue)
						{
							Args.IsInShadow = true;
						}
						else
						{
							Args.IsInShadow = false;
						}
					}

					// Debug depth buffer (Grayscale the pixel * depth val)
					if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
					{
						// I plan to add a way to visualize the shadow map here
						float Val = Depth;
						if (TerraPGE::DebugShadowMap)
						{
							if (ShadowDepth > ShadowValue)
							{
								Val = 0.3f;
							}
							else
							{
								Val = TerraPGE::ShadowMap[MapIdx];
							}
						}

						Args.FragColor.R = 255.0f * Val;
						Args.FragColor.G = 255.0f * Val;
						Args.FragColor.B = 255.0f * Val;
					}
					else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
					{
						// This entire else if is mainly for debugging clipping
						if (Tri.OverrideTextureColor)
						{
							Args.FragColor.R = Tri.Col.x;
							Args.FragColor.G = Tri.Col.y;
							Args.FragColor.B = Tri.Col.z;
						}
						else if (Tri.Mat.TexA.Used)
						{
							Vec3 TexturCol = Args.Tri->Mat.TexA.GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
							Args.FragColor.R = TexturCol.x;
							Args.FragColor.G = TexturCol.y;
							Args.FragColor.B = TexturCol.z;
						}
						else
						{
							Args.FragColor.R = Tri.Mat.AmbientColor.x;
							Args.FragColor.G = Tri.Mat.AmbientColor.y;
							Args.FragColor.B = Tri.Mat.AmbientColor.z;
						}
					}
					else
					{
						Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

						// Set up some shader args and call fragment shader=
						Args.FragPos = InterpolatedPos;
						Args.FragNormal = InterpolatedNormal;
						Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
						Args.BaryCoords = BaryCoords;
						Args.PixelCoords = Vec2((float)j, (float)i);
						Shader(Args);
					}

					// Set pixel in pixel buffer
					COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
					Gdi.QuickSetPixel(j, i, PixelClr);
				}
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;
					tex_w = (1.0f - t) * tex_sw + t * tex_ew;
					t += tstep;

					int idx = ContIdx(j, i, TerraPGE::sx);

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri.Points[0].x, (float)Tri.Points[0].y), Vec2((float)Tri.Points[1].x, (float)Tri.Points[1].y), Vec2((float)Tri.Points[2].x, (float)Tri.Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri.WorldSpaceVerts[0] * BaryCoords.x) + (Tri.WorldSpaceVerts[1] * BaryCoords.y) + (Tri.WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

					// World Pos -> Clipped Space
					Vec4 NdcPos = InterpolatedPos * Vp;
					// Clipped -> Ndc
					NdcPos.CorrectPerspective();

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					// Depth test
					if (Depth < DepthBuffer[idx])
						DepthBuffer[idx] = Depth;
					else
						continue;

					float ShadowValue = 0.0f;
					float ShadowDepth = 0.0f;
					int MapIdx = 0;

					if (DoShadows)
					{
						Vec4 ShadowNdcPos = InterpolatedPos * Args.Lights[0]->CalcVpMat();
						ShadowNdcPos.CorrectPerspective();

						ShadowNdcPos *= (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
						ShadowNdcPos += (Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, ShadowMapHeight - 1), ShadowMapWidth);

						ShadowValue = TerraPGE::ShadowMap[MapIdx] + ShadowMapBias;
						ShadowDepth = (((FarNear / (TerraPGE::FFAR - ShadowNdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						if (ShadowDepth > ShadowValue)
						{
							Args.IsInShadow = true;
						}
						else
						{
							Args.IsInShadow = false;
						}
					}

					// Debug depth buffer (Grayscale the pixel * depth val)
					if (TerraPGE::DebugDepthBuffer || TerraPGE::DebugShadowMap)
					{
						// I plan to add a way to visualize the shadow map here
						float Val = Depth;
						if (TerraPGE::DebugShadowMap)
						{
							if (ShadowDepth > ShadowValue)
							{
								Val = 0.3f;
							}
							else
							{
								Val = TerraPGE::ShadowMap[MapIdx];
							}
						}

						Args.FragColor.R = 255.0f * Val;
						Args.FragColor.G = 255.0f * Val;
						Args.FragColor.B = 255.0f * Val;
					}
					else if (Args.ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri.OverrideTextureColor || !TerraPGE::DoLighting)
					{
						// This entire else if is mainly for debugging clipping
						if (Tri.OverrideTextureColor)
						{
							Args.FragColor.R = Tri.Col.x;
							Args.FragColor.G = Tri.Col.y;
							Args.FragColor.B = Tri.Col.z;
						}
						else if (Tri.Mat.TexA.Used)
						{
							Vec3 TexturCol = Args.Tri->Mat.TexA.GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
							Args.FragColor.R = TexturCol.x;
							Args.FragColor.G = TexturCol.y;
							Args.FragColor.B = TexturCol.z;
						}
						else
						{
							Args.FragColor.R = Tri.Mat.AmbientColor.x;
							Args.FragColor.G = Tri.Mat.AmbientColor.y;
							Args.FragColor.B = Tri.Mat.AmbientColor.z;
						}
					}
					else
					{
						Vec3 InterpolatedNormal = ((Tri.FaceNormal * BaryCoords.x) + (Tri.FaceNormal * BaryCoords.y) + (Tri.FaceNormal * BaryCoords.z)).Normalized();

						// Set up some shader args and call fragment shader
						Args.FragPos = InterpolatedPos;
						Args.FragNormal = InterpolatedNormal;
						Args.UVW = { tex_u / tex_w, tex_v / tex_w, tex_w };
						Args.BaryCoords = BaryCoords;
						Args.PixelCoords = Vec2((float)j, (float)i);
						Shader(Args);
					}

					// Set pixel in pixel buffer
					COLORREF PixelClr = RGB(PixelRound(Args.FragColor.R), PixelRound(Args.FragColor.G), PixelRound(Args.FragColor.B));
					Gdi.QuickSetPixel(j, i, PixelClr);
				}
			}
		}
	}

	// Render process but writes to buffer only
	void __fastcall RenderTriangleDepthOnly(Triangle* Tri, float* Buffer, SIZE_T BufferWidth, SIZE_T BufferHeight, const Matrix& ViewProjMat)
	{
		float FarSubNear = TerraPGE::FFAR - TerraPGE::FNEAR;
		float FarNear = TerraPGE::FFAR * TerraPGE::FNEAR;

		int x1 = PixelRound(Tri->Points[0].x);
		int y1 = PixelRound(Tri->Points[0].y);
		int x2 = PixelRound(Tri->Points[1].x);
		int y2 = PixelRound(Tri->Points[1].y);
		int x3 = PixelRound(Tri->Points[2].x);
		int y3 = PixelRound(Tri->Points[2].y);

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;

		float dax_step = 0, dbx_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				if (ax > bx)
				{
					std::swap(ax, bx);
				}

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{ 
					t += tstep;

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);
					// Calculate our clip space position (The perspective divide is done by the * operator making these NDC coords)
					Vec4 NdcPos = InterpolatedPos * (ViewProjMat);
					NdcPos.CorrectPerspective();

					SIZE_T Idx = ContIdx(j, i, BufferWidth);

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					if (Depth < Buffer[Idx])
						Buffer[Idx] = Depth;
					else
						continue;
				}

			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				if (ax > bx)
				{
					std::swap(ax, bx);
				}

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					t += tstep;

					// Calculate the barycentric coordinates which we use for interpolation
					Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
					// Use Barycentric coords to interpolate our world position
					Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);
					// Calculate our clip space position (The perspective divide is done by the * operator making these NDC coords)
					Vec4 NdcPos = InterpolatedPos * (ViewProjMat);
					NdcPos.CorrectPerspective();

					SIZE_T Idx = ContIdx(j, i, BufferWidth);

					// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
					// this line also normalizes that value to [0, 1]
					float Depth = (((FarNear / (TerraPGE::FFAR - NdcPos.z * FarSubNear) - TerraPGE::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

					if (Depth < Buffer[Idx])
						Buffer[Idx] = Depth;
					else
						continue;
				}
			}
		}
	}

	// Render function for rendering entire meshes
	//Rendering Pipeline: Recieve call with mesh info including positions & lights sources -> Calc and apply matrices + normals -> backface culling	
	// -> Clipping + Frustum culling -> Call Draw Triangle from graphics API with shader supplied -> Shader called from triangle routine with pixel info + normals -> SetPixel
	template<typename T>
	void RenderMesh(GdiPP& Gdi, Camera& Cam, const Mesh& MeshToRender, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, LightObject** SceneLights, size_t LightCount, T&& Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
		const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		// Normal Matrix (it's weird)
		Matrix NormalMat = ObjectMatrix.QuickInversed();
		NormalMat.Transpose3x3();

		std::vector<Triangle> TrisToRender = {};
		TrisToRender.reserve(MeshToRender.Triangles.size());

		Triangle Clipped[2];

		Vec3 NormPos = Vec3(0, 0, 0);
		Vec3 NormDir = Vec3(0, 0, 0);


		for (int i = 0; i < LightCount; i++)
		{
			LightObject* Light = SceneLights[i];

			Light->CalcVpMat();
		}

		//TODO MultiThread??
		// TODO Allow vertex shaders (:
		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space / World Space
			Triangle WorldSpaceTri = Tri;
			WorldSpaceTri.ApplyMatrix(ObjectMatrix);

			for (int i = 0; i < 3; i++)
			{
				WorldSpaceTri.WorldSpaceVerts[i] = WorldSpaceTri.Points[i];
			}

			//TODO Fix this whole damn thing calc normals at mesh gen or at vertex shade time fuck the rest
			Vec3 TriNormal = Tri.FaceNormal;
			if (MeshToRender.Normals.size() == 0)
			{
				NormPos = ((Tri.Points[0] + Tri.Points[1] + Tri.Points[2]) / 3.0f);
				NormDir = (Tri.Points[1] - Tri.Points[0]).GetVec3().CrossNormalized((Tri.Points[2] - Tri.Points[0])).Normalized();

				TriNormal = (WorldSpaceTri.Points[1] - WorldSpaceTri.Points[0]).GetVec3().CrossNormalized((WorldSpaceTri.Points[2] - WorldSpaceTri.Points[0])).Normalized(); // this line and the if statement is used for culling

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= NormalMat;
				}

				WorldSpaceTri.NormalPositions[0] = NormPos;
				WorldSpaceTri.NormalPositions[1] = Tri.Points[0];
				WorldSpaceTri.NormalPositions[2] = Tri.Points[1];
				WorldSpaceTri.NormalPositions[3] = Tri.Points[2];
				WorldSpaceTri.NormDirections[0] = NormDir;
			}
			else
			{
				TriNormal *= NormalMat;
				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= NormalMat;
				}

				NormDir = WorldSpaceTri.FaceNormal;
				NormPos = WorldSpaceTri.NormalPositions[0];
			}

			WorldSpaceTri.FaceNormal = TriNormal;

			if ((TriNormal.Dot(WorldSpaceTri.Points[0] - Cam.Pos) < 0.0f) || !TerraPGE::DoCull || !MeshToRender.BackfaceCulling) // backface culling
			{
				// 3d Space -> Viewed Space
				WorldSpaceTri.ApplyMatrix(Cam.ViewMatrix);

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.ViewSpaceVerts[i] = WorldSpaceTri.Points[i];
				}

				Vec3 PlaneNormal = { 0.0f, 0.0f, 1.0f };
				int Count = WorldSpaceTri.ClipAgainstPlane(Cam.NearPlane, PlaneNormal, Clipped[0], Clipped[1], DebugClip);

				if (Count == 0)
					continue;

				for (int i = 0; i < Count; i++)
				{
					Triangle Projected = Clipped[i];

					// Viewed Space -> clip space
					Projected = Cam.ProjectTriangle(&Projected);

					// Clip Space -> NDC Space
					Projected.ApplyPerspectiveDivide();


					// Offset to viewport space (Ndc -> Screen Space)
					Projected.Scale(Vec3((float)(TerraPGE::sx * 0.5f), (float)(TerraPGE::sy * 0.5f), 1.0f));
					Projected.Translate(Vec3((float)(TerraPGE::sx * 0.5f), (float)(TerraPGE::sy * 0.5f), 0.0f));

					// Add Triangle to render list
					TrisToRender.push_back(Projected);
				}
			}
		}

		// sort faces 
//		std::sort(TrisToRender.begin(), TrisToRender.end(), [](const Triangle& t1, const Triangle& t2)
//		{
//			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
//			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;
//			return z1 > z2;
//		});

		for (const auto& Proj : TrisToRender)
		{
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
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 1:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 2:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
							break;
						}
						case 3:
						{
							NewTrisToAdd = Test.ClipAgainstPlane({ (float)sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], TerraPGE::DebugClip);
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
			for (auto& ToDraw : ListTris)
			{
				const Material* MatToUse = nullptr;
				const Texture* TexToUse = nullptr;

				if (!MeshToRender.UseSingleMat)
					MatToUse = &ToDraw.Mat;
				else
					MatToUse = &MeshToRender.Mat;

				ShaderArgs Args(&ToDraw, MatToUse, Cam.Pos, Cam.LookDir, ObjectMatrix, Cam.ViewMatrix, Cam.ProjectionMatrix, SceneLights, LightCount, SHADER_TYPE);

				// Calc lighting (only if lighting is applied at a tri level)
				if ((DoLighting) && SHADER_TYPE == ShaderTypes::SHADER_TRIANGLE)
				{
					Shader(Args);
				}

				if (!WireFrame)
				{
					if (!ShowTriLines)
					{
						if (TerraPGE::DoMultiThreading)
							RenderTriangleThreaded(DepthBuffer, Gdi, Shader, Args);
						else
							RenderTriangle(DepthBuffer, Gdi, Shader, Args);
					}
					else
					{
						if (MeshToRender.MeshName != "Ray")
						{
							Gdi.DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));

							//if (ShowNormals)
							//{
							//	Ray NormalRay = Ray(ToDraw.NormalPositions[0], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay.mesh.UseSingleMat = true;

							//	NormalRay = Ray(ToDraw.NormalPositions[1], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay = Ray(ToDraw.NormalPositions[2], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

							//	NormalRay = Ray(ToDraw.NormalPositions[3], ToDraw.NormDirections[0]);
							//	NormalRay.GenerateMesh();
							//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

							//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);
							//}
						}
						else
						{
							Gdi.DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
						}
					}
				}
				else
				{
					Gdi.DrawTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
				}
			}
		}
	}
		
	void RenderMeshDepth(GdiPP& Gdi, const Mesh& MeshToRender, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, const Vec3& LightPos, float* Buffer)
	{
		Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
		const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
		const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
		ObjectMatrix = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

		// Light Matrices (For shadow mapping)
		// TODO calc these better (And probably move to class?)
		Matrix LightProjectionMat = Matrix::CalcOrthoMatrix(-40.0f, 40.0f, -40.0f, 40.0f, TerraPGE::FNEAR, TerraPGE::FFAR);
		Matrix LightViewMatrix = Matrix::CalcViewMatrix(((Vec3(0.0f, 0.0f, 0.0f) - LightPos).Normalized()) * 50.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
		Matrix LightVP = LightViewMatrix * LightProjectionMat;

		//TODO MultiThread??
		// Project and translate object 
		for (const auto& Tri : MeshToRender.Triangles)
		{
			// 3D Space / World Space
			Triangle Proj = Tri;
			Proj.ApplyMatrix(ObjectMatrix);

			for (int i = 0; i < 3; i++)
			{
				Proj.WorldSpaceVerts[i] = Proj.Points[i];
			}

			if (TerraPGE::DoShadows)
			{
				// 3D Space -> Viewed Space -> Clipped Space
				Proj.ApplyMatrix(LightVP);
				// Clipped Space -> NDC Space
				Proj.ApplyPerspectiveDivide();

				// Offset to viewport space (Ndc -> Screen Space)
				Proj.Scale(Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 1.0f));
				Proj.Translate(Vec3((float)(TerraPGE::ShadowMapWidth * 0.5f), (float)(TerraPGE::ShadowMapHeight * 0.5f), 0.0f));

				TerraPGE::RenderTriangleDepthOnly(&Proj, TerraPGE::ShadowMap, TerraPGE::ShadowMapWidth, TerraPGE::ShadowMapHeight, LightVP);
			}
		}
	}

	template<typename T>
	void RenderRenderable(GdiPP& Gdi, Camera& Cam, Renderable& R, LightObject LightSrc, T&& Shader, const int SHADER_TYPE = SHADER_FRAGMENT)
	{
		RenderMesh(Gdi, Cam, R.mesh, R.Scalar, R.RotationRads, R.Pos, LightSrc.LightPos, LightSrc.Color, LightSrc.AmbientCoeff, LightSrc.DiffuseCoeff, LightSrc.SpecularCoeff, Shader, SHADER_TYPE);
	}

	//TerraGL (Proposed name for GdiPP)
	//TerraPGE (Proposed name for this engine)
	//Engine Pipeline (In a Loop): Check Window State -> Check Inputs -> Clear Screen -> Make Draw Calls (Rendering Pipeline) -> Draw Double Buffer
	void Run(WndCreator& Wnd, BrushPP& ClearBrush, DoTick_T DrawCallBack)
	{
		// Init Variables and grab screen dimensions
		GdiPP EngineGdi = GdiPP(Wnd.Wnd, true);
		TerraPGE::sx = Wnd.GetClientArea().Width;
		TerraPGE::sy = Wnd.GetClientArea().Height;

		TerraPGE::UpdateSystemInfo();

		delete[] TerraPGE::DepthBuffer;

		// Create depth buffer
		TerraPGE::DepthBuffer = new float[(SIZE_T)(sx * sy)];

		// Initial Client Region
		EngineGdi.UpdateClientRgn();

		// Counters
		MSG msg = { 0 };
		double ElapsedTime = 0.0f;
		double FpsCounter = 0.0f;
		int FrameCounter = 0;
		auto Start = std::chrono::system_clock::now();
		SIZE_T CurrMB = 0;

		// Init Cursor position
		SetCursorPos(sx / 2, sy / 2);
		GetCursorPos(&Tmp);
		PrevMousePos = { (float)Tmp.x, (float)Tmp.y };

		std::vector<Renderable> ToRender;
		std::vector<LightObject*> Lights;

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
					GetCursorPos(&Tmp);

					DeltaMouse.x = Tmp.x - PrevMousePos.x;
					DeltaMouse.y = -(Tmp.y - PrevMousePos.y);
					DeltaMouse.x *= Sensitivity;
					DeltaMouse.y *= Sensitivity;
				}

				if (LockCursor)
					SetCursorPos(sx / 2, sy / 2);

				GetCursorPos(&Tmp);
				PrevMousePos = { (float)Tmp.x, (float)Tmp.y };

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
			EngineGdi.Clear(GDIPP_PIXELCLEAR, ClearBrush);

			// Clear Buffers
			std::fill(DepthBuffer, DepthBuffer + sx * sy, 1.0f);
			std::fill(ShadowMap, ShadowMap + ShadowMapWidth * ShadowMapHeight, 1.0f);

			ToRender.clear();
			Lights.clear();

			// call draw code
			DrawCallBack(EngineGdi, Wnd, (float)ElapsedTime, &ToRender, &Lights);

			LightObject** LightsToRender = new LightObject*[Lights.size()];

			for (int idx = 0; idx < Lights.size(); idx++)
			{
				LightsToRender[idx] = Lights.at(idx);
			}

			// Depth Pass
			for (const auto& Rend : ToRender)
			{
				RenderMeshDepth(EngineGdi, Rend.mesh, Rend.Scalar, Rend.RotationRads, Rend.Pos, LightsToRender[0]->LightPos, ShadowMap);
			}

			for (auto& Rend : ToRender)
			{
				TerraPGE::RenderMesh(EngineGdi, Rend.Cam, Rend.mesh, Rend.Scalar, Rend.RotationRads, Rend.Pos, LightsToRender, Lights.size(), Rend.Shader, Rend.SHADER_TYPE);
			}

			if (FpsEngineCounter)
			{
#ifdef UNICODE
				// Draw FPS and some debug info
				std::wstring Str = FpsWStr + std::to_wstring(Fps) + L" Memory Usage: " + std::to_wstring(CurrMB) + L"/" + std::to_wstring(TerraPGE::MaxMemoryMB) + L" MB " + (DoMultiThreading ?  L"(MultiThreaded)" : L"");
				Wnd.SetWndTitle(Str);
				EngineGdi.DrawStringW(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
#ifndef UNICODE
				std::string Str = FpsStr + std::to_string(Fps) + " Memory Usage: " + std::to_string(CurrMB) + "/" + std::to_string(TerraPGE::MaxMemoryMB) + " MB " + (DoMultiThreading ? "(MultiThreaded)" : "");;
				Wnd.SetWndTitle(Str);
				EngineGdi.DrawStringA(20, 20, Str, RGB(255, 0, 0), TRANSPARENT);
#endif
			}

			// Draw to screen
			EngineGdi.DrawDoubleBuffer();

			// Calc elapsed time
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

				// Get current memory usage
				CurrMB = GetUsedMemory();
			}
		}

		Wnd.Destroy();

		EngineCleanup();
	}
}
