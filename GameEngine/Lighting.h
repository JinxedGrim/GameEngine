#pragma once
#include "Math.h"

enum class LightTypes
{
	PointLight = 0,
	DirectionalLight = 1,
	SpotLight = 2,
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
		this->LightMesh.Materials.at(0)->AmbientColor = LightColor;
		this->LightMesh.Materials.at(0)->DiffuseColor = LightColor;
		this->LightMesh.Materials.at(0)->SpecularColor = LightColor;

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

	LightTypes Type = LightTypes::DirectionalLight;

	Vec3 LightDir = Vec3(0, 0, -1);
	Vec3 LightPos = Vec3(0, 0, 0);
	Vec3 UpVector = Vec3(0, 1, 0);
	Vec3 Color = Vec3(253, 251, 211);
	Mesh LightMesh = Mesh();
	bool Render = false;

	float AmbientCoeff = 0.1f;  // Lowest level of light possible (only the objects mat ambient color)
	float SpecularCoeff = 0.5f; // How much the lights color will combine with the objects specular color
	float DiffuseCoeff = 0.25f; // How much the light will combine with the objects diffuse mat color

	float Far = 0.1f;
	float Near = 150.0f;
};

class PointLight : public LightObject
{

};

class DirectionalLight : public LightObject
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
		this->Type = LightTypes::DirectionalLight;

	}
	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float Left, float Right, float Bottom, float Top, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->Left = Left;
		this->Right = Right;
		this->Top = Bottom;
		this->Bottom = Top;
		this->CenterPoint = CenterPoint;
		this->Type = LightTypes::DirectionalLight;

	}

	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Mesh LightMesh, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff, LightMesh)
	{
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->CenterPoint = CenterPoint;
		this->Type = LightTypes::DirectionalLight;

	}

	DirectionalLight(Vec3 Pos, Vec3 LightDir, Vec3 LightColor, float AmbientCoeff, float DiffuseCoeff, float SpecularCoeff, Vec3 CenterPoint = Vec3()) : LightObject(Pos, LightDir, LightColor, AmbientCoeff, DiffuseCoeff, SpecularCoeff)
	{
		this->Left = -40.0f;
		this->Right = 40.0f;
		this->Top = 40.0f;
		this->Bottom = -40.0f;
		this->CenterPoint = CenterPoint;
		this->Type = LightTypes::DirectionalLight;
	}

	__inline Matrix CalcVpMat() override
	{
		Matrix LightProjectionMat = Matrix::CalcOrthoMatrix(Left, Right, Bottom, Top, Far, Near);
		Matrix LightViewMatrix = Matrix::CalcViewMatrix(((this->CenterPoint - this->LightPos).Normalized()) * 50.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
		return LightViewMatrix * LightProjectionMat;
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

	__inline Matrix CalcVpMat() override
	{

	}
};