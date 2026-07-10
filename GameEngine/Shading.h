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

enum class ShaderTypes
{
	SHADER_TRIANGLE = 0,
	SHADER_FRAGMENT = 1,
};

struct ShaderUniform
{
	const void* Data = nullptr;
};


struct ShaderBuffers
{
	void* Data = nullptr;
	uint32_t stride = 0;
	uint32_t sz = 0;
};


struct ShaderVarying
{
	void* Data = nullptr;
};


struct OwnedPtr
{
	void* Ptr = nullptr;
	void (*Delete)() = nullptr;
};


template<typename T>
struct Slot
{
	uint32_t index = 0;
};


class ShaderArgs
{
	inline static std::vector<ShaderUniform> Uniforms;
	std::vector<ShaderVarying> Varyings;
	std::vector<ShaderBuffers> Buffers;
	std::vector<OwnedPtr> DeleteList;
	


	inline static uint32_t UniformCounter = 0;
	inline static uint32_t VaryingCounter = 0;
	inline static uint32_t BufferCounter = 0;

	template<typename T>
	inline void EnsureSize(std::vector<T>& vec, uint32_t index)
	{
		if (vec.size() <= index)
			vec.resize(index + 1);
	}


	inline void EnsureBufferSize(uint32_t index)
	{
		if (this->Buffers.size() <= index)
			this->Buffers.resize(index + 1);
	}

public:

	template<typename T>
	static __inline uint32_t& GetTypedUniformCounter()
	{
		static uint32_t counter = 0;
		return counter;
	}

	template<typename T>
	static __inline uint32_t& GetTypedVaryingCounter()
	{
		static uint32_t counter = 0;
		return counter;
	}

	template<typename T>
	static __inline uint32_t& GetTypedBufferCounter()
	{
		static uint32_t counter = 0;
		return counter;
	}

	template<typename T>
	static __inline Slot<T> _AllocateUniformSlot()
	{
		(GetTypedUniformCounter<T>())++;
		return { UniformCounter++ };
	}

	template<typename T>
	static __inline Slot<T> _AllocateVaryingSlot()
	{
		GetTypedVaryingCounter<T>()++;
		return { VaryingCounter++ };
	}

	template<typename T>
	static __inline Slot<T> _AllocateBufferSlot()
	{
		GetTypedBufferCounter<T>()++;
		return { BufferCounter++ };
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
	__inline void BindUniform(Slot<T> slot, const T* Data)
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
	void BindVarying(Slot<T> slot, T* Data)
	{
		this->EnsureSize(this->Varyings, slot.index);
		this->Varyings[slot.index].Data = (void*)Data;
	}


	template<typename T>
	__inline void BindOwnedUniform(Slot<T> slot, const T* Data, void(*Deleter)() = nullptr)
	{
		this->EnsureSize(this->Uniforms, slot.index);
		this->Uniforms[slot.index].Data = (void*)Data;
		OwnedPtr op = { (void*)Data, Deleter };
		this->DeleteList.push_back(op);
	}


	template<typename T>
	__inline void BindOwnedBuffer(Slot<T> slot, T* Data, unsigned __int32 Sz, unsigned __int32 Stride, void(*Deleter)() = nullptr)
	{
		this->EnsureBufferSize(slot.index);

		this->Buffers[slot.index].data = (void*)Data;
		this->Buffers[slot.index].size = Sz;
		this->Buffers[slot.index].stride = Stride;

		OwnedPtr op = { Data, Deleter };
		this->DeleteList.push_back(op);
	}


	template<typename T>
	__inline void BindOwnedVarying(Slot<T> slot, T* Data, void(*Deleter)() = nullptr)
	{
		this->EnsureSize(this->Varyings, slot.index);
		this->Varyings[slot.index].Data = (void*)Data;

		OwnedPtr op = { Data, Deleter };
		this->DeleteList.push_back(op);
	}


	__inline void DeleteData()
	{
		for (OwnedPtr& ptr : this->DeleteList)
		{
			if (ptr.Delete != nullptr)
				ptr.Delete();
			else
				delete ptr.Ptr;
		}

		this->DeleteList.clear();
	}

	~ShaderArgs()
	{
		this->DeleteData();
	}
};

#define DEFINE_UNIFORM(name, type) const inline static Slot<type> name { ShaderArgs::_AllocateUniformSlot<type>() }
#define DEFINE_VARYING(name, type) const inline static Slot<type> name { ShaderArgs::_AllocateVaryingSlot<type>() }
#define DEFINE_BUFFER(name, type) const inline static Slot<type> name { ShaderArgs::_AllocateBufferSlot<type>() }


enum SHADER_MODES
{
	CPU_SHADER = 0,
	GPU_SHADER = 1
};

class BaseShader
{
public:

	SHADER_MODES Mode = SHADER_MODES::CPU_SHADER;
	virtual __inline void Run(ShaderArgs* Args) = 0;
};

namespace TerraPGE
{
	DEFINE_UNIFORM(TPGE_SHDR_TYPE_, ShaderTypes);
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