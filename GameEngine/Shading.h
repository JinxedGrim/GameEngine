#pragma once
#include "Camera.h"
#include "Lighting.h"
#include <string>
#include "Materials.h"
#include <functional>
#include <cassert>

enum class ShaderTypes
{
	SHADER_TRIANGLE = 0,
	SHADER_FRAGMENT = 1,
};

struct ShaderData
{
	void* Data = nullptr;
	__int32 DataSz = 0;
	bool FreeOnDelete = false;
	std::string Name = "";
	size_t StoredHash = 0;

	ShaderData()
	{
		this->Data = nullptr;
		this->DataSz = 0;
		this->FreeOnDelete = false;
		this->StoredHash = 0;
	}

	ShaderData(const ShaderData* Data)
	{
		this->Data = Data->Data;
		this->DataSz = Data->DataSz;
		this->Name = Data->Name;
		this->StoredHash = Data->StoredHash;
		this->FreeOnDelete = false;
	}

	static __inline size_t GetHash(const std::string_view& Name)
	{
		return ShaderData::hash_fn(Name);
	}

	static const std::hash<std::string_view> hash_fn;
};

const std::hash<std::string_view> ShaderData::hash_fn;

// ShaderName class
class ShaderName
{
	size_t hash;

	public:
	ShaderName(const char* name)
		: hash(ShaderData::hash_fn(std::string_view(name))) {}

	// Getter for the hash value
	size_t getHash() const 
	{
		return hash;
	}

	constexpr operator unsigned long long() const 
	{
		return hash;
	}
};


const ShaderName TPGE_SHDR_TYPE = "TpgeShdrType";
const ShaderName TPGE_SHDR_CAMERA_POS = "TpgeShdrCamPos";
const ShaderName TPGE_SHDR_CAMERA_LDIR = "TpgeShdrCamLdir";
const ShaderName TPGE_SHDR_CAMERA_VIEW_MATRIX = "TpgeShdrCamViewMat";
const ShaderName TPGE_SHDR_CAMERA_PROJ_MATRIX = "TpgeShdrCamProjMat";
const ShaderName TPGE_SHDR_OBJ_MATRIX = "TpgeShdrObjMat";
const ShaderName TPGE_SHDR_TRI = "TpgeShdrTri";
const ShaderName TPGE_SHDR_LIGHT_COUNT = "TpgeShdrLightCount";
const ShaderName TPGE_SHDR_LIGHT_OBJECTS = "TpgeShdrLightObjects";
const ShaderName TPGE_SHDR_FRAG_NORMAL = "TpgeShdrFragNormal";
const ShaderName TPGE_SHDR_FRAG_COLOR = "TpgeShdrFragColor";
const ShaderName TPGE_SHDR_FRAG_POS = "TpgeShdrFragPos";
const ShaderName TPGE_SHDR_IS_IN_SHADOW = "TpgeShdrIsInShadow";
const ShaderName TPGE_SHDR_DEBUG_SHADOWS = "TpgeShdrDebugShadows";
const ShaderName TPGE_SHDR_FRAG_BARY_COORDS = "TpgeShdrFragBaryCoords";
const ShaderName TPGE_SHDR_PIXEL_COORDS = "TpgeShdrPixelCoords";
const ShaderName TPGE_SHDR_TEX_UVW = "TpgeShdrTexUvwPixelCoords";

// Class for holding shader data you can add / get variables from it by using string name
class ShaderArgs
{
	std::vector<ShaderData*> Payload = {};
	int Parameters = 0;

	public:

	ShaderArgs()
	{

	}

	ShaderArgs(const ShaderArgs* B)
	{
		Payload.reserve(B->Payload.size());

		// Iterate through the hash table
		for (ShaderData* BData : B->Payload)
		{
			ShaderData* AData = DEBUG_NEW ShaderData(BData); // copy constructor  
			AData->FreeOnDelete = false; // Set this because we are a child of the original args and it will do cleanup / the user will

			this->Payload.push_back(AData);
		}

		this->Parameters = this->Payload.size();
	}


	const size_t __inline FindHash(const std::string_view& Name)
	{
		return ShaderData::GetHash(Name);


		//const auto &it = this->Hashes.find(Name);
		//if (it != this->Hashes.end())
		//{
		//	return it->second;
		//}
		//else
		//{
		//	return 0;
		//}
	}


	__inline ShaderData* FindDataPtr(const size_t& Hash)
	{
		for (ShaderData* Data : this->Payload)
		{
			if (Data->StoredHash == Hash)
			{
				return Data;
			}
		}

		return nullptr;
	}


	template<typename T>
	T FindShaderResourcePtr(const ShaderName& Hash)
	{
		ShaderData* ShdrData = this->FindDataPtr(Hash);

		if (ShdrData == nullptr)
		{
			throw;
		}

		return (T)ShdrData->Data;
	}


	template<typename T>
	T FindShaderResourceValue(const size_t& Hash)
	{
		const ShaderData* ShdrData = this->FindDataPtr(Hash);

		if (ShdrData == nullptr)
		{
			if constexpr (std::is_fundamental<T>::value)
			{
				return NULL;
			}
			else
			{
				return T();
			}
		}
		
		if (ShdrData->Data == nullptr)
		{
			throw;
		}

		return *(T*)ShdrData->Data;
	}


	ShaderData* FindShaderResourceEdit(const size_t& Hash)
	{
		ShaderData* ShadrData = this->FindDataPtr(Hash);

		return ShadrData;
	}


	void AddShaderDataPtr(size_t Hash, void* Data, __int32 Sz = sizeof(void*), bool FreeOnDelete = false)
	{
		ShaderData* ShadrData = this->FindDataPtr(Hash);

		if (ShadrData == nullptr)
		{
			ShadrData = new ShaderData();
		}

		ShadrData->StoredHash = Hash;
		ShadrData->DataSz = Sz;
		ShadrData->Data = Data;
		ShadrData->FreeOnDelete = FreeOnDelete;

		this->Payload.push_back(ShadrData);

		Parameters++;
	}


	// Primitives work all around here 
	// for a class it must have a ctor with the sig ctor(*B) as this function will attempt to instantiate a copy of the object
	template <typename T>
	void AddShaderDataByValue(const size_t& Hash, const T& Data, const __int32& Sz = sizeof(T))
	{
		ShaderData* ShadrData = this->FindDataPtr(Hash);

		if (ShadrData == nullptr)
		{
			ShadrData = DEBUG_NEW ShaderData();
		}

		ShadrData->StoredHash = Hash;
		ShadrData->DataSz = Sz;


		if constexpr (std::is_fundamental<T>::value)
		{
			ShadrData->Data = DEBUG_NEW T;
			*(T*)(ShadrData->Data) = Data;
		}
		else
		{
			ShadrData->Data = DEBUG_NEW T();

			assert(ShadrData != nullptr);

			*(T*)(ShadrData->Data) = Data;
		}

		ShadrData->FreeOnDelete = true;

		this->Payload.push_back(ShadrData);

		Parameters++;
	}


	void EditShaderData(const size_t& Hash, void* Data, const __int32& Sz = sizeof(void*))
	{
		ShaderData* ShadrData = this->FindDataPtr(Hash);

		if (ShadrData != nullptr)
		{
			ShadrData->DataSz = Sz;
			ShadrData->Data = Data;
		}
		else
		{
			throw;
		}
	}


	template<typename T>
	void EditShaderDataValue(const size_t& Hash, const T& Data)
	{
		ShaderData* ShadrData = this->FindShaderResourceEdit(Hash);

		if (ShadrData != nullptr && ShadrData->Data != nullptr)
		{
			*((T*)ShadrData->Data) = Data;
		}
		else
		{
			throw;	
		}
	}


	bool DoesArgExist(const size_t& Hash)
	{
		const ShaderData* Data = this->FindDataPtr(Hash);

		if (Data == nullptr)
		{
			return false;
		}

		return true;
	}


	void Delete()
	{
		delete this;
		//for (const auto& pair : this->Payload)
		//{
		//	ShaderData* Data = pair.second;

		//	if (Data->Data != nullptr && Data->FreeOnDelete)
		//	{
		//		delete Data->Data;
		//	}

		//	delete Data;
		//}

		//this->Payload.clear();
	}


	~ShaderArgs()
	{
		for (ShaderData* Data : this->Payload)
		{
			if (Data->Data != nullptr && Data->FreeOnDelete)
			{
				delete Data->Data;
			}

			delete Data;
		}

		this->Payload.clear();
	}
};

namespace EngineShaders
{
	//const auto DefaultVertexShader = [](ShaderArgs* Args)
	//{

	//};

	const auto WHACK_SHADER = [](ShaderArgs* Args)
	{
		Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		Material* Mat = Tri->Material;

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 LDir = (Light->LightPos - ((Tri->Points[0] + Tri->Points[1] + Tri->Points[2]) / 3.0f)).Normalized();

		float Li = std::max<float>(0.0f, Tri->FaceNormal.Dot(LDir));

		Vec3 AmbientCol = Mat->AmbientColor * Light->AmbientCoeff;
		Vec3 DiffuseCol = Mat->DiffuseColor * Li;

		Tri->Col = (AmbientCol + DiffuseCol) * Light->Color;
	};

	const auto Shader_Phong_LOW_LOD = [](ShaderArgs* Args)
	{
		Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		Material* Mat = Tri->Material;

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 LDir = (Light->LightPos - ((Tri->Points[0] + Tri->Points[1] + Tri->Points[2]) / 3.0f)).Normalized();

		float Li = std::max<float>(0.0f, Tri->FaceNormal.Dot(LDir));

		Vec3 AmbientCol = (Mat->AmbientColor * Light->AmbientCoeff);
		Vec3 DiffuseCol = ((Light->Color * Li) + (Mat->DiffuseColor * Li)) * Light->DiffuseCoeff;

		Tri->Col.x = std::clamp<float>((AmbientCol.x + DiffuseCol.x), 0.0f, 255.0f);
		Tri->Col.y = std::clamp<float>((AmbientCol.y + DiffuseCol.y), 0.0f, 255.0f);
		Tri->Col.z = std::clamp<float>((AmbientCol.z + DiffuseCol.z), 0.0f, 255.0f);
	};

	const auto Shader_Phong = [](ShaderArgs* Args)
	{
		Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		Material* Mat = Tri->Material;
		Vec3* LookDir = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_CAMERA_LDIR);

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - ((Tri->Points[0] + Tri->Points[1] + Tri->Points[2]) / 3.0f)).Normalized();
		float Li = Tri->FaceNormal.Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);
		Vec3 RDir = LDir - Tri->FaceNormal * 2.0f * Li;

		float SpecularIntensity = pow(std::max<float>(0.0f, RDir.Dot(*LookDir)), Mat->Shininess);

		Vec3 AmbientCol = (Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		Tri->Col.x = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		Tri->Col.y = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		Tri->Col.z = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
	};

	const auto Shader_Material = [](ShaderArgs* Args)
	{
		Triangle* Tri = Args->FindShaderResourceValue<Triangle*>(TPGE_SHDR_TRI);
		Material* Mat = Tri->Material;

		Tri->Col = Mat->AmbientColor;
	};

	const auto Shader_Frag_Phong = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		const Material* Mat = Tri->Material;
		const Vec3* CamLookDir = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_CAMERA_LDIR);
		Vec3* FragPos = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_POS);
		Vec3* FragNormal = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_NORMAL);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 LDir = Light->LightPos.GetDirectionToVector(*FragPos);
		float Li = FragNormal->Dot(LDir);

		float Intensity = std::max<float>(0.0f, Li);
		Vec3 RDir = (-LDir).GetReflectection(*FragNormal);

		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(*CamLookDir)), Mat->Shininess);

		Vec3 AmbientCol = (Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		FragColor->R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		FragColor->G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		FragColor->B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
		FragColor->A = 255.0f;
	};

	const auto Shader_Frag_Phong_Shadows = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		const Material* Mat = Tri->Material;
		const Vec3* CamLookDir = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_CAMERA_LDIR);
		Vec3* FragPos = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_POS);
		Vec3* FragNormal = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_NORMAL);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);
		bool IsInShadow = Args->FindShaderResourceValue<bool>(TPGE_SHDR_IS_IN_SHADOW);
		//bool DebugShadows = Args->FindShaderResourceValue<bool>(TPGE_SHDR_DEBUG_SHADOWS);

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 LDir = Light->LightPos.GetDirectionToVector(*FragPos);
		float Li = FragNormal->Dot(LDir);

		// Calculate the reflection direction using the formula: R = I - 2 * (I dot N) * N
		float Intensity = std::max<float>(0.0f, Li);
		//Vec3 RDir = (LDir - Args->FragNormal * 2.0f * Li).Normalized();
		Vec3 RDir = (-LDir).GetReflectection(*FragNormal);

		// Calculate the specular intensity
		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(*CamLookDir)), Mat->Shininess);

		Vec3 AmbientCol = (Mat->AmbientColor) * Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

		float ShadowMult = 1.0f;

		ShadowMult = 1.0f - (0.5f * (int)IsInShadow);

		FragColor->R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x) * ShadowMult, 0.0f, 255.0f);
		FragColor->G = std::clamp<float>(((AmbientCol.y + DiffuseCol.y + SpecularClr.y) * ShadowMult) /*((255.0f * (int)DebugShadows) * (int)IsInShadow)*/, 0.0f, 255.0f);
		FragColor->B = std::clamp<float>(((AmbientCol.z + DiffuseCol.z + SpecularClr.z) * ShadowMult) /*((255.0f * (int)DebugShadows) * (int)IsInShadow)*/, 0.0f, 255.0f);
		FragColor->A = 255.0f;
	};

	const auto Shader_Gradient = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		const Material* Mat = Tri->Material;
		const Vec3* CamLookDir = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_CAMERA_LDIR);
		const Vec3* FragPos = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_POS);
		const Vec3* FragNormal = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_NORMAL);
		const Vec3* BaryCoords = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_BARY_COORDS);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - *FragPos).Normalized();
		float Li = FragNormal->Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);

		FragColor->R = std::clamp<float>(((255.0f * BaryCoords->x) * Intensity), 0.0f, 255.0f);
		FragColor->G = std::clamp<float>(((255.0f * BaryCoords->y) * Intensity), 0.0f, 255.0f);
		FragColor->B = std::clamp<float>(((255.0f * BaryCoords->z) * Intensity), 0.0f, 255.0f);
		FragColor->A = 255.0f;
	};

	const auto Shader_Gradient_Centroid = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		const Vec3* FragPos = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_POS);
		const Vec3* FragNormal = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_NORMAL);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);
		Vec2* PixelCoords = Args->FindShaderResourcePtr<Vec2*>(TPGE_SHDR_PIXEL_COORDS);
		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 Centroid = ((Tri->Points[0] + Tri->Points[1] + Tri->Points[2]) / 3.0f);
		Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(*PixelCoords, Vec2(Centroid.x, Centroid.y), Vec2(Tri->Points[1].x, Tri->Points[1].y), Vec2(Tri->Points[2].x, Tri->Points[2].y));
		Vec3 BaryCoords2 = CalculateBarycentricCoordinatesScreenSpace(*PixelCoords, Vec2(Tri->Points[0].x, Tri->Points[0].y), Vec2(Tri->Points[1].x, Tri->Points[1].y), Vec2(Centroid.x, Centroid.y));

		float Intensity = 1.0f;
		Vec3 LDir = (Light->LightPos - *FragPos).Normalized();
		float Li = FragNormal->Dot(LDir);

		Intensity = std::max<float>(0.0f, Li);

		FragColor->R = std::clamp<float>(((255.0f * BaryCoords2.x) * Intensity), 0.0f, 255.0f);
		FragColor->G = std::clamp<float>(((255.0f * BaryCoords.y) * Intensity), 0.0f, 255.0f);
		FragColor->B = std::clamp<float>(((255.0f * BaryCoords.z) * Intensity), 0.0f, 255.0f);
		FragColor->A = 255.0f;
	};

	const auto Shader_Tex_Phong = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		const size_t LightCount = Args->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		const Vec3* FragPos = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_POS);
		const Vec3* FragNormal = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_NORMAL);
		const Vec3* BaryCoords = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_FRAG_BARY_COORDS);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);
		Vec2* PixelCoords = Args->FindShaderResourcePtr<Vec2*>(TPGE_SHDR_PIXEL_COORDS);
		const Vec3* CamLookDir = Args->FindShaderResourcePtr<Vec3*>(TPGE_SHDR_CAMERA_LDIR);
		TextureCoords* UVW = Args->FindShaderResourcePtr<TextureCoords*>(TPGE_SHDR_TEX_UVW);
		const Material* Mat = Tri->Material;

		LightObject* Light = nullptr;
		if (LightCount <= 0)
		{
			return;
		}

		Light = *Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);

		Vec3 TexturCol = Vec3();

		if (Tri->Material->HasUsableTexture())
		{
			TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(UVW->u, 1.0f - UVW->v).GetRGB();
			FragColor->R = std::clamp<float>(TexturCol.x, 0.0f, 255.0f);
			FragColor->G = std::clamp<float>(TexturCol.y, 0.0f, 255.0f);
			FragColor->B = std::clamp<float>(TexturCol.z, 0.0f, 255.0f);
			FragColor->A = 255.0f;
		}
		else
		{
			Shader_Frag_Phong(Args);
		}

		Vec3 LDir = Light->LightPos.GetDirectionToVector(*FragPos);
		float Li = FragNormal->Dot(LDir);

		// Calculate the reflection direction using the formula: R = I - 2 * (I dot N) * N
		float Intensity = std::max<float>(0.0f, Li);
		//Vec3 RDir = (LDir - Args->FragNormal * 2.0f * Li).Normalized();
		Vec3 RDir = (-LDir).GetReflectection(*FragNormal);

		float SpecularIntensity = pow(std::max<float>(0.0f, (-RDir).Dot(*CamLookDir)), Mat->Shininess);

		Vec3 AmbientCol = (TexturCol)*Light->AmbientCoeff;
		Vec3 DiffuseCol = ((Light->Color * Intensity) + (TexturCol * Intensity)) * Light->DiffuseCoeff;
		Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (TexturCol * Light->SpecularCoeff)) * SpecularIntensity;

		FragColor->R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
		FragColor->G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
		FragColor->B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
		FragColor->A = 255.0f;
	};

	const auto Shader_Texture_Only = [](ShaderArgs* Args)
	{
		const Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		TextureCoords* UVW = Args->FindShaderResourcePtr<TextureCoords*>(TPGE_SHDR_TEX_UVW);
		Color* FragColor = Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

		Vec3 TexturCol = Vec3();

		if (Tri->Material->HasUsableTexture())
		{
			TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(UVW->u, 1.0f - UVW->v).GetRGB();
			FragColor->R = std::clamp<float>(TexturCol.x, 0.0f, 255.0f);
			FragColor->G = std::clamp<float>(TexturCol.y, 0.0f, 255.0f);
			FragColor->B = std::clamp<float>(TexturCol.z, 0.0f, 255.0f);
			FragColor->A = 255.0f;
		}
		else
		{
			FragColor->R = 255.0f;
			FragColor->G = 255.0f;
			FragColor->B = 255.0f;
			FragColor->A = 255.0f;
		}
	};
}