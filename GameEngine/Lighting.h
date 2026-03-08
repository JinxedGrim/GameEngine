#pragma once
#include "Math.h"
#include "GameObject.h"

//TODO
// 1. Finish other lighting types
// 2. add getters/setters to maintain vp mat (IsVpDirty)

enum class LightTypes
{
	PointLight = 0,
	DirectionalLight = 1,
	SpotLight = 2,
};


class LightObject : public GameObject
{
public:

	float ShadowMapBias = 0.0000005f;

	LightObject() = delete;

	bool IsVpDirty = false;


//	static Vec3 DirToEuler(const Vec3& d_raw)
//	{
//		Vec3 d = d_raw.Normalized();
//		float pitch = asinf(d.y);
//		float yaw = atan2f(d.x, d.z);
//		float roll = 0.0f;
//		return Vec3(pitch, yaw, roll);
//	}


	virtual Vec3 GetLightDirection()
	{
		// Default: forward vector from transform
		// Modify depending on your forward axis
		return this->Transform.GetWorldForward().Normalized();
	}


	LightObject(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, float Near = 0.1f, float Far = 100.0f) : GameObject(Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Pos)
	{
		this->Color = LightColor;

		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;

		this->LightMesh = CubeMesh;
		
		// TODO Fix this
		//this->LightMesh.Materials.at(0)->AmbientColor = LightColor;
		//this->LightMesh.Materials.at(0)->DiffuseColor = LightColor;
		//this->LightMesh.Materials.at(0)->SpecularColor = LightColor;

		this->Far = Far;
		this->Near = Near;

		this->Render = false;
		this->IsVpDirty = true;
	}


	LightObject(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, float Near = 0.1f, float Far = 150.0f) : GameObject(Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), Pos)
	{
		this->Color = LightColor;

		this->AmbientCoeff = AmbientCoeff;
		this->DiffuseCoeff = DiffuseCoeff;
		this->SpecularCoeff = SpecularCoeff;

		this->LightMesh = LightMesh;

		this->Far = Far;
		this->Near = Near;
		this->Render = false;
	}


	virtual void CalcVpMats() = 0;


	virtual int SelectVpMat(Vec3 FragPos) = 0;


	virtual Vec4 CalcNdc(Vec4 InterpolatedPos) = 0;


	virtual __inline bool SampleShadowMap(float* ShadowMap, __int64 W, __int64 H, Vec3 WorldPos, float* ShadowDepth, float* ShadowMapDepth) = 0;


	LightTypes Type = LightTypes::DirectionalLight;


	Vec3 UpVector = Vec3(0, 1, 0);
	Vec3 Color = Vec3(253, 251, 211);

	Matrix VpMatrices[6] = { Matrix().CreateIdentity() };

	Mesh LightMesh = Mesh();

	bool Render = false;
	bool CastsShadows = false;

	float AmbientCoeff = 0.1f;  // Lowest level of light possible (only the objects mat ambient color)
	float SpecularCoeff = 0.5f; // How much the lights color will combine with the objects specular color
	float DiffuseCoeff = 0.25f; // How much the light will combine with the objects diffuse mat color


	__inline void SetAmbientCoeff(const float& NewAmb)
	{
		this->AmbientCoeff = NewAmb;
	}


	__inline void SetDiffuseCoeff(const float& NewDiff)
	{
		this->DiffuseCoeff = NewDiff;
	}


	__inline void SetSpecularCoeff(const float& NewSpec)
	{
		this->SpecularCoeff = NewSpec;
	}


	__inline void SetFar(const float& NewFar)
	{
		this->Far = NewFar;
		this->IsVpDirty = true;
	}


	__inline void SetNear(const float& NewNear)
	{
		this->Near = NewNear;
		this->IsVpDirty = true;
	}

	
	float Far = 100.0f;
	float Near = 0.1f;
};


class DirectionalLight : public LightObject
{
	float _lightDistane = 0.0f;

public:
	DirectionalLight() : LightObject(Vec3(), Vec3(), Vec3(255, 255, 255), 0.15f, 0.15f, 0.15f)
	{
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		Vec3 CenterPoint = Vec3();
		this->LightDirection = Vec3(0, 0, 0);
		this->Type = LightTypes::DirectionalLight;
		this->IsVpDirty = true;
		this->CalcVpMats();
	}


	DirectionalLight(Vec3 LightDir, float LightDistance, Vec3 LightColor, float Left, float Right, float Bottom, float Top, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Vec3 CenterPoint = Vec3()) : LightObject(CenterPoint, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->Left = Left;
		this->Right = Right;
		this->Top = Bottom;
		this->Bottom = Top;
		this->CenterPoint = CenterPoint;
		this->LightDirection = LightDir;
		this->Type = LightTypes::DirectionalLight;
		this->IsVpDirty = true;
		this->CalcVpMats();
	}


	DirectionalLight(Vec3 LightDir, float LightDistance, Vec3 LightColor, float Left, float Right, float Bottom, float Top, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(CenterPoint, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->IsVpDirty = true;
		this->Left = Left;
		this->Right = Right;
		this->Top = Bottom;
		this->Bottom = Top;
		this->CenterPoint = CenterPoint;
		this->LightDirection = LightDir;
		this->Type = LightTypes::DirectionalLight;
		this->CalcVpMats();
	}


	DirectionalLight(Vec3 LightDir, float LightDistance, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(CenterPoint, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->IsVpDirty = true;
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->CenterPoint = CenterPoint;
		this->LightDirection = LightDir;
		this->Type = LightTypes::DirectionalLight;
		this->CalcVpMats();
	}


	DirectionalLight(Vec3 LightDir, float LightDistance, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Vec3 CenterPoint = Vec3(0.0f, 0.0f, 0.0f)) : LightObject(Vec3(0, 0, 0), LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->IsVpDirty = true;
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->_lightDistane = LightDistance;
		this->CenterPoint = CenterPoint;
		this->LightDirection = LightDir;
		this->Type = LightTypes::DirectionalLight;
		this->CalcVpMats();
	}


	__inline void CalcVpMats() override
	{
		Vec3 lightPos = this->CenterPoint - this->LightDirection.Normalized() * this->_lightDistane;
		Matrix LightViewMatrix = Matrix::CalcViewMatrix(lightPos, CenterPoint, Vec3(0, 1, 0));
		Matrix LightProjectionMat = Matrix::CalcOrthoMatrix(Left, Right, Bottom, Top, Near, Far);

		this->VpMatrices[0] = LightViewMatrix * LightProjectionMat;
		this->IsVpDirty = false;
	}


	virtual Vec3 GetLightDirection() override
	{
		return this->LightDirection;
	}


	Matrix GetViewMatrix()
	{
		Vec3 lightPos = this->CenterPoint - this->LightDirection.Normalized() * this->_lightDistane;
		return Matrix::CalcViewMatrix(lightPos, CenterPoint, Vec3(0, 1, 0));
	}


	Matrix GetProjectionMatrix()
	{
		return Matrix::CalcOrthoMatrix(Left, Right, Bottom, Top, Near, Far);
	}


	__inline bool SampleShadowMap(float* ShadowMap, __int64 W, __int64 H, Vec3 WorldPos, float* ShadowDepth, float* ShadowMapDepth) override
	{
		this->CalcVpMats();

		// Transform interpolated world position to light clip-space
		Vec4 LightClip = Vec4(WorldPos, 1.0f) * this->VpMatrices[0];

		// Divide by W, map to shadow map
		Vec3 ShadowUV;

		float invW = 1.0f / LightClip.w;

		ShadowUV.x = LightClip.x * invW;
		ShadowUV.y = LightClip.y * invW;
		ShadowUV.z = LightClip.z * invW;

		// NDC x,y -> [-1,1] -> [0,1]
		ShadowUV.x = ShadowUV.x * 0.5f + 0.5f;
		ShadowUV.y = ShadowUV.y * 0.5f + 0.5f; // flip Y

		if ((ShadowUV.x < 0 || ShadowUV.x > 1) || ((ShadowUV.y < 0 || ShadowUV.y > 1)))
			return false;

		int ShadowX = int(ShadowUV.x * W);
		int ShadowY = int(ShadowUV.y * H);

		// 5. Sample shadow map
		int MapIdx = ContIdx(ShadowX, ShadowY, W);

		// TODO BAD INTERP     Core::ShadowMap[MapIdx]
		*ShadowMapDepth = ShadowMap[MapIdx];
		*ShadowDepth = ShadowUV.z + ShadowMapBias;

		return *ShadowDepth > *ShadowMapDepth;
	}


	__inline int SelectVpMat(Vec3 FragPos) override
	{
		return 0;
	}

	Vec4 CalcNdc(Vec4 InterpolatedPos) override
	{
		int idx = this->SelectVpMat(InterpolatedPos);

	    InterpolatedPos *= this->VpMatrices[idx];
		InterpolatedPos.CorrectPerspective();

		return InterpolatedPos;
	}

private:
	float Left = -40.0f;
	float Right = 40.0f;
	float Top = 40.0f;
	float Bottom = -40.0f;
	Vec3 LightDirection = Vec3();
	Vec3 CenterPoint = Vec3();

};


class PointLight : public LightObject
{
	float ConstantAttenuation = 0.0f;
	float LinearAttenuation = 0.0f;
	float QuadraticAttenuation = 0.0f;

public:
	PointLight() : LightObject(Vec3(), Vec3(), Vec3(255, 255, 255), 0.15f, 0.15f, 0.15f)
	{
		this->ConstantAttenuation = 0.0f;
		this->LinearAttenuation = 0.0f;
		this->QuadraticAttenuation = 0.0f;
		this->LightMesh = Sphere(0.5f, 20, 20);
		this->Type = LightTypes::PointLight;
	}

	PointLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float ConstantAttenuation, float LinearAttenuation, float QuadraticAttenuation, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->ConstantAttenuation = ConstantAttenuation;
		this->LinearAttenuation = LinearAttenuation;
		this->QuadraticAttenuation = QuadraticAttenuation;
		this->LightMesh = Sphere(0.5f, 20, 20);
		this->Type = LightTypes::PointLight;
	}

	float Attenuate(float Dist)
	{
		return 1.0f / (this->ConstantAttenuation + this->LinearAttenuation * Dist + this->QuadraticAttenuation * Dist * Dist);
	}


	__inline void CalcVpMats() override
	{
		Matrix Persp = Matrix::CalcPerspectiveMatrix(90.0f, 1.0f, this->Far, this->Near);

		Vec3 LightPos = this->Transform.GetLocalPosition();

		this->VpMatrices[0] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(1, 0, 0), Vec3(0, -1, 0)) * Persp;  // + X
		this->VpMatrices[1] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(-1, 0, 0), Vec3(0, -1, 0)) * Persp; // - X
		this->VpMatrices[2] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(0, 1, 0), Vec3(0, 0, 1)) * Persp;   // + Y
		this->VpMatrices[3] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(0, -1, 0), Vec3(0, 0, -1)) * Persp; // - Y
		this->VpMatrices[4] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(0, 0, 1), Vec3(0, -1, 0)) * Persp;  // + Z
		this->VpMatrices[5] = Matrix::CalcViewMatrix(LightPos,  LightPos + Vec3(0, 0, -1), Vec3(0, -1, 0)) * Persp; // - Z
	}

	__inline int SelectVpMat(Vec3 FragPos) override
	{
		Vec3 CardDir = this->Transform.GetLocalPosition().GetSignedCardinalDirection(FragPos);

		int axis = CardDir.GetDominantAxis();

		// Map axis to VpMatrix index
		// +X: 0, -X: 1, +Y: 2, -Y: 3, +Z: 4, -Z: 5
		if (CardDir.data[axis] < 0)
			return axis * 2 + 1; // -X/Y/Z
		else
			return axis * 2 + 0; // +X/Y/Z
	}

	Vec4 CalcNdc(Vec4 InterpolatedPos) override
	{
		int idx = this->SelectVpMat(InterpolatedPos);

		return InterpolatedPos * this->VpMatrices[idx];
	}

	__inline bool SampleShadowMap(float* ShadowMap, __int64 W, __int64 H, Vec3 WorldPos, float* ShadowDepth, float* ShadowMapDepth) override
	{
		return false;
	}
};


class SpotLight : public LightObject
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
		this->Type = LightTypes::SpotLight;
	}

	//SpotLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, float CutoffAngle, float ConstantAttenuation = 0.0f, float LinearAttenuation = 0.0f, float QuadraticAttenuation = 0.0f) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	//{
	//	this->CutoffAngle = CutoffAngle;
	//	this->ConstantAttenuation = ConstantAttenuation;
	//	this->LinearAttenuation = LinearAttenuation;
	//	this->QuadraticAttenuation = QuadraticAttenuation;
	//}

	__inline void CalcVpMat()
	{

	}

	__inline bool SampleShadowMap(float* ShadowMap, __int64 W, __int64 H, Vec3 WorldPos, float* ShadowDepth, float* ShadowMapDepth) override
	{
		return false;
	}
};