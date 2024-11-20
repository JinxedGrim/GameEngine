#pragma once
#include "Math.h"
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

class Material
{
	public:

	static inline std::vector<Material*> LoadedMaterials = {};

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
