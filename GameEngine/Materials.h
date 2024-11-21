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

	static Material* FindMaterial(std::string Name)
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

	static Material* LoadMaterial(std::string MtlFn, std::string MtlName)
	{
		Material* Mat = FindMaterial(MtlName);

		if (Mat != nullptr)
		{
			return Mat;
		}
		else
		{
			Mat = new Material();
		}

		std::ifstream mtlFile(MtlFn);
		if (!mtlFile.is_open())
		{
			Mat = new Material();
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
								Mat->Textures.push_back(new Texture(textureFilePath));
							}
						}
						else if (propKeyword == "map_Kd")
						{
							std::string textureFilePath;
							ssProp >> textureFilePath;
							if (textureFilePath != "")
							{
								Mat->Textures.push_back(new Texture(textureFilePath));
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

	int GetLoadedMatsSize()
	{
		return this->LoadedMaterials.size();
	}

	Vec3 AmbientColor = Vec3(255.0f * 0.15f, 0, 0);
	Vec3 DiffuseColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	Vec3 SpecularColor = Vec3(255.0f * 0.25f, 255.0f * 0.25f, 255.0f * 0.25f);
	float Shininess = 32.0f;

	std::vector<Texture*> Textures = {};

	std::string MaterialName = "DefaultMat";


	//TODO FIND A WAY TO NOT ALLOW ALLOCATION WITHOUT NEW 
	Material()
	{
		Material* RetMat = FindMaterial("DefaultMat");
		if (RetMat == nullptr)
		{
			Material* m = new Material(NULL_TEXTURE_COLOR_VEC3, NULL_TEXTURE_COLOR_VEC3, NULL_TEXTURE_COLOR_VEC3, 96.0f, "DefaultMat");
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

		LoadedMaterials.push_back(this);
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

	bool HasUsableTexture()
	{
		if (this->Textures.size() > 0 && this->Textures.at(0)->Used)
		{
			return true;
		}

		return false;
	}

	protected:
	~Material()
	{

	}
};