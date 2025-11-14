#pragma once
#include "Math.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#define _NULL_MATERIAL_VALUES 255.0f, 0.0f, 255.0f
#define NULL_MATERIAL_COLOR Color(_NULL_MATERIAL_VALUES)
#define NULL_MATERIAL_COLOR_VEC3 Vec3(_NULL_MATERIAL_VALUES)

#define SoftUnlitMatAmbient Vec3(0.15f * 255.0f, 0.15f * 255.0f, 0.15f * 255.0f)
#define SoftUnlitMatDiffuse Vec3(0.2f * 255.0f, 0.2f * 255.0f, 0.2f * 255.0f)
#define SoftUnlitMatSpecular Vec3(0.2f * 255.0f, 0.2f * 255.0f, 0.2f * 255.0f)

class Material
{
public:
	Vec3 AmbientColor = Vec3(255.0f * 0.15f, 0, 0);
	Vec3 DiffuseColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	Vec3 SpecularColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	Vec3 EmissiveColor = Vec3(0.0f, 0.0f, 0.0f);

	//Core Blinn Phong
	float Shininess = 32.0f;
	float EmissiveStrength = 0.0f;
	float Reflectivity = 0.0f;     // For mirror/reflection intensity
	float Opacity = 1.0f;          // For transparency or alpha blending
	float RefractiveIndex = 1.0f;  // For refraction (if you add that later)

	// Core PBR params
	float Metallic = 0.0f;          // 0 = dielectric, 1 = metal
	float Roughness = 0.5f;         // Microfacet roughness
	float AO = 1.0f;                // Ambient occlusion multiplier

	Texture* DiffuseMap = nullptr;
	Texture* SpecularMap = nullptr;
	Texture* NormalMap = nullptr;
	Texture* EmissiveMap = nullptr;
	Texture* RoughnessMap = nullptr;
	Texture* MetallicMap = nullptr;
	Texture* AOMap = nullptr;
	Texture* HeightMap = nullptr; // (for parallax/displacement)

	// add more properties TODO
	// ALSO TEXTURE MAPS (::


	std::vector<Texture*> Textures = {};
	std::string MaterialName = "NullMat";

	static inline std::vector<Material*> LoadedMaterials = {};


	static Material* FindMaterial(std::string MtlName)
	{
		for (Material* M : LoadedMaterials)
		{
			if (M->MaterialName == MtlName)
			{
				return M;
			}
		}

		return nullptr;
	}


	static Material* LoadMaterial(std::string MtlFn, std::string MtlName)
	{
		Material* Mat = FindMaterial(MtlName);

		if (Mat != nullptr)
		{
			return Mat;
		}
		else
		{
			Mat = DEBUG_NEW Material();
		}

		std::ifstream mtlFile(MtlFn);
		if (!mtlFile.is_open())
		{
			Mat = DEBUG_NEW Material();
			return nullptr;
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
					Mat->MaterialName = name;

					Mat->EmissiveStrength = 0.0f;

					while (std::getline(mtlFile, line))
					{
						std::stringstream ssProp(line);
						std::string propKeyword;
						ssProp >> propKeyword;

						if (propKeyword == "Ka")
						{
							ssProp >> Mat->AmbientColor.x >> Mat->AmbientColor.y >> Mat->AmbientColor.z;
							Mat->AmbientColor *= 255.0f;
						}
						else if (propKeyword == "Kd")
						{
							ssProp >> Mat->DiffuseColor.x >> Mat->DiffuseColor.y >> Mat->DiffuseColor.z;
							Mat->DiffuseColor *= 255.0f;
						}
						else if (propKeyword == "Ks")
						{
							ssProp >> Mat->SpecularColor.x >> Mat->SpecularColor.y >> Mat->SpecularColor.z;
							Mat->SpecularColor *= 255.0f;
						}
						else if (propKeyword == "Ns")
						{
							ssProp >> Mat->Shininess;
						}
						else if (propKeyword == "map_Ka")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
								Mat->Textures.push_back(DEBUG_NEW Texture(textureFilePath));
							}
						}
						else if (propKeyword == "map_Kd")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
								Mat->Textures.push_back(DEBUG_NEW Texture(textureFilePath));
							}
						}

						if (line.find("newmtl") != std::string::npos)
							break;
					}

					break;
				}
			}
		}

		LoadedMaterials.push_back(Mat);
		mtlFile.close();

		return Mat;
	}


	static Material* CreateMaterial(Vec3 AmbientColor, Vec3 DiffuseColor, Vec3 SpecularColor, float Shininess, std::string MtlName="")
	{
		Material* Mat = FindMaterial(MtlName);

		if (Mat != nullptr)
		{
			return Mat;
		}
		else
		{
			Mat = DEBUG_NEW Material();
		}

		Mat->AmbientColor = AmbientColor;
		Mat->DiffuseColor = DiffuseColor;
		Mat->SpecularColor = SpecularColor;
		Mat->Shininess = Shininess;

		LoadedMaterials.push_back(Mat);

		return Mat;
	}


	static Material* GetNullMaterial()
	{
		Material* RetMat = FindMaterial("NullMat");
		if (RetMat == nullptr)
		{
			RetMat = Material::CreateMaterial(NULL_MATERIAL_COLOR_VEC3, NULL_MATERIAL_COLOR_VEC3, NULL_MATERIAL_COLOR_VEC3, 96.0f, "NullMat");

			if (RetMat == nullptr)
			{
				throw;
			}

			RetMat->EmissiveColor = NULL_MATERIAL_COLOR_VEC3;
			RetMat->EmissiveStrength = 1.0f;
		}

		return RetMat;
	}


	int GetLoadedMatsSize()
	{
		return this->LoadedMaterials.size();
	}


	bool HasUsableTexture()
	{
		if (this->Textures.size() > 0 && this->Textures.at(0)->Used)
		{
			return true;
		}

		return false;
	}


	void Delete()
	{
		auto it = std::find(LoadedMaterials.begin(), LoadedMaterials.end(), this);

		if (it != LoadedMaterials.end())
		{
			this->LoadedMaterials.erase(it);
		}

		for (Texture* T : Textures)
		{
			T->Delete();
		}

		delete this;
	}

protected:

	Material()
	{
		this->AmbientColor = Vec3(255.0f * 0.15f, 0, 0);
		this->DiffuseColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
		this->SpecularColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
		this->EmissiveColor = Vec3(0.0f, 0.0f, 0.0f);

		//Core Blinn Phong
		this->Shininess = 32.0f;
		this->EmissiveStrength = 0.0f;
		this->Reflectivity = 0.0f;     // For mirror/reflection intensity
		this->Opacity = 1.0f;          // For transparency or alpha blending
		this->RefractiveIndex = 1.0f;  // For refraction (if you add that later)

		// Core PBR params
		this->Metallic = 0.0f;          // 0 = dielectric, 1 = metal
		this->Roughness = 0.5f;         // Microfacet roughness
		this->AO = 1.0f;                // Ambient occlusion multiplier

		this->DiffuseMap = nullptr;
		this->SpecularMap = nullptr;
		this->NormalMap = nullptr;
		this->EmissiveMap = nullptr;
		this->RoughnessMap = nullptr;
		this->MetallicMap = nullptr;
		this->AOMap = nullptr;
		this->HeightMap = nullptr; // (for parallax/displacement)
	}

	~Material()
	{

	}
};

