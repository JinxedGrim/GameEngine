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
#define SoftUnlitMatDiffuse Vec3(0.6f * 255.0f, 0.6f * 255.0f, 0.6f * 255.0f)
#define SoftUnlitMatSpecular Vec3(0.04f * 255.0f, 0.04f * 255.0f, 0.04f * 255.0f)

class Material
{
public:
	Vec3 AmbientColor = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 DiffuseColor = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 SpecularColor = Vec3(0.0f, 0.0f, 0.0f);
	Vec3 EmissiveColor = Vec3(_NULL_MATERIAL_VALUES);

	//Core Blinn Phong
	float Shininess = 32.0f;
	float EmissiveStrength = 1.0f;
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


	static Material* LoadMaterialFile(std::string MtlFn, std::string MtlName, const std::string& Prefix = "Assets\\")
	{
		Material* Mat = FindMaterial(MtlName);

		if (Mat != nullptr)
		{
			return Mat;
		}
		else
		{
			Mat = DEBUG_NEW Material(MtlName);
		}

		std::ifstream mtlFile(Prefix + MtlFn);
		if (!mtlFile.is_open())
		{
			std::cout << "Failed to load: " << Prefix + MtlFn << std::endl;
			delete Mat;
			Mat = GetNullMaterial();
			return Mat;
		}

		std::string line;
		std::cout << "Loading Material: " << Prefix + MtlFn << std::endl;
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
#ifdef _DEBUG
								std::cout << "Loading Texture (ka): " << textureFilePath << std::endl;
#endif
								Mat->Textures.push_back(Texture::Create(textureFilePath));
							}
						}
						else if (propKeyword == "map_Kd")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
#ifdef _DEBUG
								std::cout << "Loading Texture (kd): " << textureFilePath << std::endl;
#endif
								Mat->Textures.push_back(Texture::Create(textureFilePath));
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

		if (Mat == nullptr)
		{
			Mat = DEBUG_NEW Material(AmbientColor, DiffuseColor, SpecularColor, Shininess, MtlName);
		}

		return Mat;
	}


	// Need to copy the mat instead of returning it somehow or block modifications to this? 
	// But honestly if a user is grabbing the null mat they can change it if they want i guess
	// Not sure on the correct solution TODO
	static Material* GetNullMaterial()
	{
		Material* RetMat = FindMaterial("NullMat");
		if (RetMat == nullptr)
		{
			RetMat = DEBUG_NEW Material(Vec3(0,0,0), Vec3(0, 0, 0), Vec3(0, 0, 0), 0.0f, "NullMat");

			if (RetMat == nullptr)
			{
				throw;
			}

			RetMat->EmissiveColor = NULL_MATERIAL_COLOR_VEC3;
			RetMat->EmissiveStrength = 1.0f;
		}

		return RetMat;
	}


	size_t GetLoadedMatsSize()
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

private:

	Material(const std::string& Uri)
	{
		this->AmbientColor = Vec3(0.0f, 0.0f, 0.0f);
		this->DiffuseColor = Vec3(0.0f, 0.0f, 0.0f);
		this->SpecularColor = Vec3(0.0f, 0.0f, 0.0f);
		this->EmissiveColor = Vec3(_NULL_MATERIAL_VALUES);

		//Core Blinn Phong
		this->Shininess = 32.0f;
		this->EmissiveStrength = 1.0f;
		this->Reflectivity = 0.0f;     // For mirror/reflection intensity TODO
		this->Opacity = 1.0f;          // For transparency or alpha blending
		this->RefractiveIndex = 1.0f;  // For refraction (TODO)

		// Core PBR params
		this->Metallic = 0.0f;          // 0 = dielectric, 1 = metal
		this->Roughness = 0.5f;         // Microfacet roughness
		this->AO = 1.0f;                // Ambient occlusion multiplier

		this->MaterialName = Uri;

		this->DiffuseMap = nullptr;
		this->SpecularMap = nullptr;
		this->NormalMap = nullptr;
		this->EmissiveMap = nullptr;
		this->RoughnessMap = nullptr;
		this->MetallicMap = nullptr;
		this->AOMap = nullptr;
		this->HeightMap = nullptr; // (for parallax/displacement)
	}

	Material(Vec3 AmbientColor, Vec3 DiffuseColor, Vec3 SpecularColor, float Shininess, const std::string& Uri)
	{
		this->AmbientColor = AmbientColor;
		this->DiffuseColor = DiffuseColor;
		this->SpecularColor = SpecularColor;
		this->Shininess = Shininess;

		this->EmissiveColor = Vec3(0.0f, 0.0f, 0.0f);
		this->EmissiveStrength = 0.0f;

		this->MaterialName = Uri;

		LoadedMaterials.push_back(this);
	}

	~Material()
	{

	}
};

