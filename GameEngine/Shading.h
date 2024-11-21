#pragma once
#include "Camera.h"
#include "Lighting.h"
#include <string>
#include "Materials.h"

enum class ShaderTypes
{
	SHADER_TRIANGLE = 0,
	SHADER_FRAGMENT = 1,
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
	bool DebugShadows = false;
	bool DebugShadowmap = false;

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

namespace EngineShaders
{
	//const auto DefaultVertexShader = [](ShaderArgs& Args)
	//{

	//};

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

		ShadowMult = 1.0f - (0.5f * (int)Args.IsInShadow);

		Args.FragColor.R = std::clamp<float>((AmbientCol.x + DiffuseCol.x + SpecularClr.x) * ShadowMult, 0.0f, 255.0f);
		Args.FragColor.G = std::clamp<float>(((AmbientCol.y + DiffuseCol.y + SpecularClr.y) * ShadowMult) - ((255.0f * Args.DebugShadows) * (int)Args.IsInShadow), 0.0f, 255.0f);
		Args.FragColor.B = std::clamp<float>(((AmbientCol.z + DiffuseCol.z + SpecularClr.z) * ShadowMult) - ((255.0f * Args.DebugShadows) * (int)Args.IsInShadow), 0.0f, 255.0f);
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

		if (Args.Tri->Material->HasUsableTexture())
		{
			TexturCol = Args.Tri->Material->Textures.at(0)->GetPixelColor(Args.UVW.u, 1.0f - Args.UVW.v).GetRGB();
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

		Vec3 AmbientCol = (TexturCol)*Light->AmbientCoeff;
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

		if (Args.Tri->Material->HasUsableTexture())
		{
			TexturCol = Args.Tri->Material->Textures.at(0)->GetPixelColor(Args.UVW.u, 1.0f - Args.UVW.v).GetRGB();
			Args.FragColor.R = std::clamp<float>(TexturCol.x, 0.0f, 255.0f);
			Args.FragColor.G = std::clamp<float>(TexturCol.y, 0.0f, 255.0f);
			Args.FragColor.B = std::clamp<float>(TexturCol.z, 0.0f, 255.0f);
			Args.FragColor.A = 255.0f;
		}
		else
		{
			Args.FragColor.R = 255.0f;
			Args.FragColor.G = 255.0f;
			Args.FragColor.B = 255.0f;
			Args.FragColor.A = 255.0f;
		}
	};
}