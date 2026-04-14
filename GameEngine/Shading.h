#pragma once
#define NOMINMAX
#include "Camera.h"
#include "Lighting.h"
#include <string>
#include "Materials.h"
#include <functional>
#include <cassert>

// TODO 
// this shading system is cool but performs too slow.
// move to 'functor'?


// New proposal
// System has struct of param info each info contains an ID and an index.
// then an arg with the full data struct
// 

struct ShaderUniform
{
	void* Data;
};


struct ShaderBuffers
{
	void* Data;
	uint32_t stride;
	uint32_t sz;
};


struct ShaderVarying
{
	void* Data;
};


template<typename T>
struct Slot
{
	uint32_t index;
};

class ShaderArgs
{
	std::vector<ShaderUniform> Uniforms;
	std::vector<ShaderVarying> Varyings;
	std::vector<ShaderBuffers> Buffers;

	inline void EnsureSize(std::vector<void*>& vec, uint32_t index)
	{
		if (vec.size() <= index)
			vec.resize(index + 1, nullptr);
	}


	inline void EnsureBufferSize(uint32_t index)
	{
		if (this->Buffers.size() <= index)
			this->Buffers.resize(index + 1);
	}

public:
	static __inline unsigned __int32 _AllocateUniformSlot()
	{
		static unsigned __int32 UniformCounter = 0;
		return UniformCounter++;
	}


	static __inline unsigned __int32 _AllocateVaryingSlot()
	{
		static unsigned __int32 VaryingCounter = 0;
		return VaryingCounter++;
	}


	static __inline unsigned __int32 _AllocateBufferSlot()
	{
		static unsigned __int32 BufferCounter = 0;
		return BufferCounter++;
	}


	//template<typename T>
	//inline const T* GetUniformAsCopy(Slot<T> slot)
	//{

	//	assert(ptr != nullptr && "Uniform not bound");
	//	return *(T*)ptr;
	//}


	template<typename T>
	inline const T* GetUniformAsPtr(Slot<T> slot)
	{
		const void* ptr = this->Uniforms[slot.index].Data;
		assert(ptr != nullptr && "Uniform not bound");
		return (const T*)ptr;
	}


	template<typename T>
	inline const T& GetUniformAsRef(Slot<T> slot)
	{
		const void* ptr = this->Uniforms[slot.index].Data;
		assert(ptr != nullptr && "Uniform not bound");
		return *(const T*)ptr;
	}


	template<typename T>
	inline T& GetVaryingAsRef(Slot<T> slot)
	{
		void* ptr = this->Varyings[slot.index].Data;
		assert(ptr != nullptr && "Varying not bound");
		return *(const T*)ptr;
	}


	template<typename T>
	inline T* GetVaryingAsPtr(Slot<T> slot)
	{
		void* ptr = this->Varyings[slot.index].Data;
		assert(ptr != nullptr && "Varying not bound");
		return (T*)ptr;
	}


	template<typename T>
	inline T* GetBuffer(Slot<T> slot)
	{
		return (T*)this->Buffers[slot.index].data;
	}


	// Bound data must stay alive for duration of object lifetime!
	template<typename T>
	__inline void BindUniform(Slot<T> slot, T* Data)
	{
		this->EnsureSize(this->Uniforms, slot.index);
		this->Uniforms[slot.index].Data = (void*)Data;
	}


	// Bound data must stay alive for duration of object lifetime!
	template<typename T>
	__inline void BindBuffer(Slot<T> slot, T* Data, unsigned __int32 Sz, unsigned __int32 Stride)
	{
		this->EnsureBufferSize(slot.index);

		this->Buffers[slot.index].data = (void*)Data;
		this->Buffers[slot.index].size = Sz;
		this->Buffers[slot.index].stride = Stride;
	}


	// Bound data must stay alive for duration of object lifetime!
	template<typename T>
	__inline void BindVarying(Slot<T> slot, T* Data)
	{
		this->EnsureSize(this->Varyings, slot.index);
		this->Varyings[slot.index].Data = (void*)Data;
	}
};

#define DEFINE_UNIFORM(name, type) static Slot<type> name { ShaderArgs::_AllocateUniformSlot() }
#define DEFINE_VARYING(name, type) static Slot<type> name { ShaderArgs::_AllocateVaryingSlot() }
#define DEFINE_BUFFER(name, type) static Slot<type> name { ShaderArgs::_AllocateBufferSlot() }


DEFINE_UNIFORM(TPGE_SHDR_TYPE_, int);
DEFINE_UNIFORM(TPGE_SHDR_CAMERA_POS_, Vec3);
DEFINE_UNIFORM(TPGE_SHDR_CAMERA_LDIR_, Vec3);
DEFINE_UNIFORM(TPGE_SHDR_CAMERA_VIEW_MATRIX_, Matrix);
DEFINE_UNIFORM(TPGE_SHDR_CAMERA_PROJ_MATRIX_, Matrix);
DEFINE_UNIFORM(TPGE_SHDR_OBJ_MATRIX_, Matrix);
DEFINE_UNIFORM(TPGE_SHDR_TRI_, Triangle);
DEFINE_UNIFORM(TPGE_SHDR_LIGHT_COUNT_, size_t);
DEFINE_UNIFORM(TPGE_SHDR_LIGHT_OBJECTS_, LightObject**);
DEFINE_UNIFORM(TPGE_SHDR_DEBUG_SHADOWS_, bool);

DEFINE_VARYING(TPGE_SHDR_FRAG_NORMAL_, Vec3);
DEFINE_VARYING(TPGE_SHDR_FRAG_COLOR_, Color);
DEFINE_VARYING(TPGE_SHDR_FRAG_POS_, Vec3);
DEFINE_VARYING(TPGE_SHDR_IS_IN_SHADOW_, bool);
DEFINE_VARYING(TPGE_SHDR_FRAG_BARY_COORDS_, Vec3);
DEFINE_VARYING(TPGE_SHDR_PIXEL_COORDS_, Vec2);
DEFINE_VARYING(TPGE_SHDR_TEX_UVW_, TextureCoords);


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
class ShaderArgss
{
	std::vector<ShaderData*> Payload = {};
	int Parameters = 0;

	public:

	ShaderArgss()
	{

	}

	ShaderArgss(const ShaderArgss* B)
	{
		Payload.reserve(B->Payload.size());

		// Iterate through the hash table
		for (const ShaderData* BData : B->Payload)
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


	void __inline PrepareFragmentShader()
	{
		this->AddShaderDataByValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(0, 0, 0), 0);
		this->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_POS, Vec3(), sizeof(void*));
		this->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, Vec3(), sizeof(void*));
		this->AddShaderDataByValue<TextureCoords>(TPGE_SHDR_TEX_UVW, TextureCoords(), sizeof(void*));
		this->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, Vec3(), sizeof(void*));
		this->AddShaderDataByValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2(), sizeof(void*));
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


	~ShaderArgss()
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

//struct BaseShaderArgs
//{
//	ShaderTypes ShaderType = ShaderTypes::SHADER_TRIANGLE;
//	LightTypes LightType = LightTypes::DirectionalLight;
//
//	// Triangle Info (ALL SHADERS)
//	Triangle* Tri = nullptr;
//	const Material* Mat = nullptr;
//
//	//Camera Info (ALL SHADERS)
//	Matrix ModelMat;00
//	Matrix ViewMat;
//	Matrix ProjectionMat;
//	Vec3 CamPos = Vec3(0, 0, 0);
//	Vec3 CamLookDir = Vec3(0, 0, 0);
//
//	// Light Info (ALL SHADERS)
//	LightObject** Lights;
//	size_t LightCount;
//
//	// Fragment Info (SHADER_TYPE == SHADER_FRAGMENT)
//	Vec3* FragPos = nullptr;
//	Vec3 FragNormal = Vec3(0.0f, 0.0f, 0.0f);
//	Vec3 BaryCoords = Vec3(0, 0, 0);
//	Color FragColor = Color(0.0f, 0.0f, 0.0f);
//	Vec2 PixelCoords = Vec2(0, 0);
//	TextureCoords UVW = { 0.0f, 0.0f, 0.0f };
//
//	// ShadowInfo
//	Vec3 LightSpacePos;
//	bool IsInShadow = false;
//	bool DebugShadows = false;
//	bool DebugShadowmap = false;
//
//	BaseShaderArgs()
//	{
//
//	}
//
//	BaseShaderArgs(Triangle* Tri, const Material* Mat, const Vec3& CamPos, const Vec3& CamLookDir, const Matrix& ModelMatrix, const Matrix& ViewMatrix, const Matrix& ProjMatrix, LightObject** Lights, const size_t LightCount, const ShaderTypes SHADER_TYPE)
//	{
//		this->Tri = Tri;
//		this->Mat = Mat;
//		this->CamPos = CamPos;
//		this->CamLookDir = CamLookDir;
//		this->Lights = Lights;
//		this->LightCount = LightCount;
//		this->ShaderType = SHADER_TYPE;
//		this->ModelMat = ModelMatrix;
//		this->ViewMat = ViewMatrix;
//		this->ProjectionMat = ProjMatrix;
//	}
//};
//};
//
//template<typename ArgsT>
//struct Shader
//{
//	inline void operator()(ArgsT* args) const
//	{
//
//	}
//};
//
//
//



namespace TerraPGE
{
	namespace EngineShaders
	{
		namespace DebugShaders
		{
			const auto Shader_Gradient = [](ShaderArgs* Args)
			{
				const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
				const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));
				const Material* Mat = Tri->Material;
				const Vec3* CamLookDir = Args->GetUniformAsPtr<Vec3>(TPGE_SHDR_CAMERA_LDIR_);
				LightObject** Lights = *Args->GetUniformAsPtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);

				// Varyings
				const Vec3* FragNormal = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_NORMAL_);
				const Vec3* BaryCoords = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS_);
				Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);
				LightObject* Light = nullptr;

				if (LightCount <= 0)
				{
					return;
				}

				Light = *Lights;

				Vec3 LDir = Light->GetLightDirection();
				float Li = FragNormal->Dot(LDir);

				float Intensity = std::max<float>(0.0f, Li);

				FragColor->R = std::clamp<float>(((255.0f * BaryCoords->x) * Intensity), 0.0f, 255.0f);
				FragColor->G = std::clamp<float>(((255.0f * BaryCoords->y) * Intensity), 0.0f, 255.0f);
				FragColor->B = std::clamp<float>(((255.0f * BaryCoords->z) * Intensity), 0.0f, 255.0f);
				FragColor->A = 255.0f;			
			};

				
			const auto Shader_Normal = [](ShaderArgs* Args)
			{
				const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
				const Vec3* FragNormal = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_NORMAL_);
				Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

				Vec3 n = FragNormal->Normalized();

				FragColor->R = std::clamp<float>((n.x * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
				FragColor->G = std::clamp<float>((n.y * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
				FragColor->B = std::clamp<float>((n.z * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
				FragColor->A = 255.0f;
			};

			const auto Shader_WireFrame = [](ShaderArgs* Args)
			{
				const Vec3* BaryCoords = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS_);
				Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

				//TODO:
				//add these to shader system
				float WireThickness = 0.01;
				Vec3 WireColor = Vec3(255, 255, 255);

				// Basic idea: 
				// each bary coord represents distance from an edge
				// so if edge distance < thickness of the wire frame 
				// we set the pixel to the wire frame color

				float Dist = std::min({ BaryCoords->x, BaryCoords->y, BaryCoords->z });

				if (Dist < WireThickness)
				{
					*FragColor = WireColor;
				}
				else
				{
					*FragColor = Vec3(0, 0, 0);
				}
			};


			const auto Shader_Gradient_Centroid = [](ShaderArgs* Args)
			{
				const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));
				LightObject** Lights = *Args->GetUniformAsPtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);

				const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
				const Vec3* FragPos = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_POS_);
				const Vec3* FragNormal = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_NORMAL_);
				Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);
				const Vec2* PixelCoords = Args->GetVaryingAsPtr<Vec2>(TPGE_SHDR_PIXEL_COORDS_);
				LightObject* Light = nullptr;
				
				if (LightCount <= 0)
				{
					return;
				}

				Light = *Lights;

				Vec3 Centroid = ((Tri->Points.Points[0] + Tri->Points.Points[1] + Tri->Points.Points[2]) / 3.0f);
				Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(*PixelCoords, Vec2(Centroid.x, Centroid.y), Vec2(Tri->Points.Points[1].x, Tri->Points.Points[1].y), Vec2(Tri->Points.Points[2].x, Tri->Points.Points[2].y));
				Vec3 BaryCoords2 = CalculateBarycentricCoordinatesScreenSpace(*PixelCoords, Vec2(Tri->Points.Points[0].x, Tri->Points.Points[0].y), Vec2(Tri->Points.Points[1].x, Tri->Points.Points[1].y), Vec2(Centroid.x, Centroid.y));

				float Intensity = 1.0f;
				Vec3 LDir = (Light->Transform.GetLocalPosition() - *FragPos).Normalized();
				float Li = FragNormal->Dot(LDir);

				Intensity = std::max<float>(0.0f, Li);

				FragColor->R = std::clamp<float>(((255.0f * BaryCoords2.x) * Intensity), 0.0f, 255.0f);
				FragColor->G = std::clamp<float>(((255.0f * BaryCoords.y) * Intensity), 0.0f, 255.0f);
				FragColor->B = std::clamp<float>(((255.0f * BaryCoords.z) * Intensity), 0.0f, 255.0f);
				FragColor->A = 255.0f;
			};

		}


		const auto WHACK_SHADER = [](ShaderArgs* Args)
		{
			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			Material* Mat = Tri->Material;
			LightObject** Lights = *Args->GetUniformAsPtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);
			const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));

			LightObject* Light = nullptr;
			if (LightCount <= 0)
			{
				return;
			}

			Light = *Lights;

			Vec3 LDir = (Light->Transform.GetLocalPosition() - ((Tri->Points.Points[0] + Tri->Points.Points[1] + Tri->Points.Points[2]) / 3.0f)).Normalized();

			float Li = std::max<float>(0.0f, Tri->FaceNormal.Dot(LDir));

			Vec3 AmbientCol = Mat->AmbientColor * Light->AmbientCoeff;
			Vec3 DiffuseCol = Mat->DiffuseColor * Li;

			Vec3 Col = (AmbientCol + DiffuseCol) * Light->Color;

			FragColor->R = Col.x;
			FragColor->G = Col.y;
			FragColor->B = Col.z;
		};


		static __inline void Shader_Phong_LOW_LOD(ShaderArgs* Args)
		{
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));
			Material* Mat = Tri->Material;
			LightObject** Lights = *Args->GetUniformAsPtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);

			LightObject* Light = nullptr;
			if (LightCount <= 0)
			{
				return;
			}

			Light = *Lights;

			Vec3 LDir = (Light->Transform.GetLocalPosition() - ((Tri->Points.Points[0] + Tri->Points.Points[1] + Tri->Points.Points[2]) / 3.0f)).Normalized();

			float Li = std::max<float>(0.0f, Tri->FaceNormal.Dot(LDir));

			Vec3 AmbientCol = (Mat->AmbientColor * Light->AmbientCoeff);
			Vec3 DiffuseCol = ((Light->Color * Li) + (Mat->DiffuseColor * Li)) * Light->DiffuseCoeff;

			FragColor->R = std::clamp<float>((AmbientCol.x + DiffuseCol.x), 0.0f, 255.0f);
			FragColor->G = std::clamp<float>((AmbientCol.y + DiffuseCol.y), 0.0f, 255.0f);
			FragColor->B = std::clamp<float>((AmbientCol.z + DiffuseCol.z), 0.0f, 255.0f);
		};


		static __inline void Shader_Phong(ShaderArgs* Args)
		{
			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));
			Material* Mat = Tri->Material;
			const Vec3* LookDir = Args->GetUniformAsPtr<Vec3>(TPGE_SHDR_CAMERA_LDIR_);
			LightObject** Lights = Args->GetUniformAsRef<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

			LightObject* Light = nullptr;
			if (LightCount <= 0)
			{
				return;
			}

			Light = *Lights;

			float Intensity = 1.0f;
			Vec3 LDir = (Light->Transform.GetLocalPosition() - ((Tri->Points.Points[0] + Tri->Points.Points[1] + Tri->Points.Points[2]) / 3.0f)).Normalized();
			float Li = Tri->FaceNormal.Dot(LDir);

			Intensity = std::max<float>(0.0f, Li);
			Vec3 RDir = LDir - Tri->FaceNormal * 2.0f * Li;

			float SpecularIntensity = pow(std::max<float>(0.0f, RDir.Dot(*LookDir)), Mat->Shininess);

			Vec3 AmbientCol = (Mat->AmbientColor) * Light->AmbientCoeff;
			Vec3 DiffuseCol = ((Light->Color * Intensity) + (Mat->DiffuseColor * Intensity)) * Light->DiffuseCoeff;
			Vec3 SpecularClr = ((Light->Color * Light->SpecularCoeff) + (Mat->SpecularColor * Light->SpecularCoeff)) * SpecularIntensity;

			FragColor->R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x), 0.0f, 255.0f);
			FragColor->G = std::clamp<float>((AmbientCol.y + DiffuseCol.y + SpecularClr.y), 0.0f, 255.0f);
			FragColor->B = std::clamp<float>((AmbientCol.z + DiffuseCol.z + SpecularClr.z), 0.0f, 255.0f);
		};


		// Blinn Phong with extra stuff
		static __inline void  DefaultShader(ShaderArgs* Args)
		{
			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			const size_t LightCount = *(Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_));
			const Material* Mat = Tri->Material;
			const Vec3* LookDir = Args->GetUniformAsPtr<Vec3>(TPGE_SHDR_CAMERA_LDIR_);
			const Vec3* CamPos = Args->GetUniformAsPtr<Vec3>(TPGE_SHDR_CAMERA_POS_);

			Vec3* FragPos = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_POS_);
			Vec3* FragNormal = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_NORMAL_);
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);
			bool IsInShadow = Args->GetVaryingAsPtr<bool>(TPGE_SHDR_IS_IN_SHADOW_);
			float shadowMul = 1.0f - IsInShadow * (1.0f - 0.5f);
			TextureCoords* UVW = Args->GetVaryingAsPtr<TextureCoords>(TPGE_SHDR_TEX_UVW_);
			FragNormal->Normalize();
			
			Vec3 TexturCol = Vec3(1.0f, 1.0f, 1.0f);
			Vec3 TexColor = Vec3(1.0f, 1.0f, 1.0f); // default white (no effect)
			TexColor *= (Tri->Material->HasUsableTexture()) ? Tri->Material->Textures.at(0)->GetPixelColor(UVW->u, 1.0f - UVW->v).GetRGB() / 255.0f : Vec3(1.0f, 1.0f, 1.0f);
				
			Vec3 BaseDiffuse = (Mat->DiffuseColor / 255.0f) * TexColor;

			LightObject* Light = nullptr;
			Vec3 FinalColor = Vec3();

			FragColor->A = 255.0f;
			FragColor->R = 0.0f;
			FragColor->G = 0.0f;
			FragColor->B = 0.0f;

			if (LightCount <= 0)
			{
				return;
			}

			LightObject** Lights = Args->GetUniformAsRef<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);

			for (int i = 0; i < LightCount; i++)
			{
				Light = Lights[i];

				Vec3 LightDir = -Light->GetLightDirection().Normalized();

				// Blinn specular math
				//https://en.wikipedia.org/wiki/Blinn–Phong_reflection_model
				Vec3 V = (*CamPos - *FragPos).Normalized();
				Vec3 H = (LightDir + V).Normalized();
				float Hdot = FragNormal->Dot(H);
				float AO = 0.7f + 0.3f * FragNormal->y;

				// Fresnel effect
				// with schlick approximation
				//https://en.wikipedia.org/wiki/Fresnel_equations
				//https://en.wikipedia.org/wiki/Schlick%27s_approximation
				float NdotV = std::max(0.0f, FragNormal->Dot(V));
				Vec3 F0 = Mat->SpecularColor / 255.0f; // or ~0.04 for non-metals
				Vec3 Fresnel = F0 + (Vec3(1.0f, 1.0f, 1.0f) - F0) * pow(1.0f - NdotV, 5.0f); // Schlick

				float Li = FragNormal->Dot(LightDir); // Light intensity
				float Intensity = std::max<float>(0.0f, Li);
				float SpecularIntensity = pow(std::max(0.0f, Hdot), Mat->Shininess);

				// Standard phong:
				//https://en.wikipedia.org/wiki/Phong_reflection_model
				Vec3 AmbientCol = (Mat->AmbientColor / 255.0f) * Light->AmbientCoeff * AO;
				Vec3 DiffuseCol = ((((Light->Color / 255.0f) * Light->DiffuseCoeff) * BaseDiffuse) * Intensity);
				Vec3 SpecularClr = ((((Light->Color / 255.0f) * Light->SpecularCoeff) * (Mat->SpecularColor / 255.0f)) * SpecularIntensity);

				DiffuseCol *= (Vec3(1.0f, 1.0f, 1.0f) - Fresnel);
				SpecularClr *= Fresnel;

				FinalColor += AmbientCol + ((DiffuseCol + SpecularClr) * shadowMul);

				// for point light
				//Vec3 LDir = Light->Transform.Ge
				//LocalPosition().GetDirectionToVector(*FragPos);
			}

			FinalColor = FinalColor;
				
			float glow = Mat->EmissiveStrength; /* * pow(1.0 - FragNormal->Dot(*CamLookDir), 4.0f)*/;
			Vec3 EmissiveCol = (Mat->EmissiveColor / 255.0f) * glow;

			FinalColor = (FinalColor + EmissiveCol);

			FragColor->R = FinalColor.x;
			FragColor->G = FinalColor.y;
			FragColor->B = FinalColor.z;
		};


		static __inline void  Shader_Frag_Phong_Shadows(ShaderArgs* Args)
		{
			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			const size_t LightCount = *Args->GetUniformAsPtr<size_t>(TPGE_SHDR_LIGHT_COUNT_);
			const Material* Mat = Tri->Material;
			const Vec3* CamLookDir = Args->GetUniformAsPtr<Vec3>(TPGE_SHDR_CAMERA_LDIR_);

			Vec3* FragPos = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_POS_);
			Vec3* FragNormal = Args->GetVaryingAsPtr<Vec3>(TPGE_SHDR_FRAG_NORMAL_);
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);
			bool IsInShadow = Args->GetVaryingAsPtr<bool>(TPGE_SHDR_IS_IN_SHADOW_);
			float ShadowMult = 1.0f - IsInShadow * (1.0f - 0.5f);
			TextureCoords* UVW = Args->GetVaryingAsPtr<TextureCoords>(TPGE_SHDR_TEX_UVW_);

			LightObject** Lights = Args->GetUniformAsRef<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS_);
			//bool DebugShadows = Args->FindShaderResourceValue<bool>(TPGE_SHDR_DEBUG_SHADOWS);
			// Shadow intensity multiplier
			//float ShadowMult = 1.0f - (0.5f * (int)IsInShadow);


			Vec3 AmbientSum(0.0f, 0.0f, 0.0f);
			Vec3 DiffuseSum(0.0f, 0.0f, 0.0f);
			Vec3 SpecularSum(0.0f, 0.0f, 0.0f);

			if (Lights && LightCount > 0)
			{
				for (size_t i = 0; i < LightCount; ++i)
				{
					LightObject* Light = Lights[i];
					if (!Light) continue;

					Vec3 LDir;
					float Atten = 1.0f;

					if (Light->Type == LightTypes::DirectionalLight)
					{
						// For directional light, direction is constant (points *toward* surface)
						LDir = (-Light->GetLightDirection()).Normalized();
					}
					else if (Light->Type == LightTypes::PointLight)
					{
						// Point light direction is from fragment to light
						LDir = Light->Transform.GetLocalPosition().GetDirectionToVector(*FragPos);
						Atten = static_cast<PointLight*>(Light)->Attenuate(Light->Transform.GetLocalPosition().Distance(*FragPos));
					}
					else
					{
						continue; // Unsupported light type
					}

					// Light direction and attenuation
					float NdotL = std::max(0.0f, FragNormal->Dot(LDir));
					float Li = FragNormal->Dot(LDir);

					// Reflection direction for specular
					Vec3 RDir = (-LDir).GetReflectection(FragNormal->Normalized());
					float SpecularIntensity = pow(std::max(0.0f, (-RDir).Dot(*CamLookDir)), Mat->Shininess);

					// Accumulate each component
					AmbientSum += (((Mat->AmbientColor) * (Light->Color / 255.0f)) * Light->AmbientCoeff) * Atten;
					DiffuseSum += ((Mat->DiffuseColor) * (Light->Color / 255.0f)) * Light->DiffuseCoeff * NdotL * Atten;
					SpecularSum += ((Mat->SpecularColor) * (Light->Color / 255.0f)) * Light->SpecularCoeff * SpecularIntensity * Atten;
				}
			}

			float glow = Mat->EmissiveStrength * pow(1.0 - FragNormal->Dot(*CamLookDir), 4.0f);
			Vec3 EmissiveCol = Mat->EmissiveColor * glow;

			Vec3 FinalColor = ((AmbientSum + DiffuseSum + SpecularSum) * ShadowMult) + EmissiveCol;

			FragColor->R = std::clamp(FinalColor.x, 0.0f, 255.0f);
			FragColor->G = std::clamp(FinalColor.y, 0.0f, 255.0f);
			FragColor->B = std::clamp(FinalColor.z, 0.0f, 255.0f);
			FragColor->A = 255.0f;
		};


		
		static __inline void Shader_Texture_Only(ShaderArgs* Args)
			{
				const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
				TextureCoords* UVW = Args->GetVaryingAsPtr<TextureCoords>(TPGE_SHDR_TEX_UVW_);
				Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

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
					FragColor->G = 0.0f;
					FragColor->B = 255.0f;
					FragColor->A = 255.0f;
				}
			};


		static __inline void Shader_Sample_Texture(ShaderArgs* Args)
		{
			const Triangle* Tri = Args->GetUniformAsPtr<Triangle>(TPGE_SHDR_TRI_);
			TextureCoords* UVW = Args->GetVaryingAsPtr<TextureCoords>(TPGE_SHDR_TEX_UVW_);
			Color* FragColor = Args->GetVaryingAsPtr<Color>(TPGE_SHDR_FRAG_COLOR_);

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
}