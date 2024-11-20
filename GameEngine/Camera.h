#pragma once
#include "Math.h"
#include "Mesh.h"

class Camera
{
	public:
	Camera()
	{

	}

	Camera(Vec3 Position, float AspectRatio, float Fov, float Near, float Far)
	{
		this->Pos = Position;

		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };

		this->CalcProjectionMat();
	}

	Camera(Vec3 Position, Vec3 TargetLook, Vec3 CamUp, float AspectRatio, float Fov, float Near, float Far)
	{
		this->Pos = Position;

		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->CamUp = CamUp;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->CalcProjectionMat();
		this->PointAt(TargetLook);
	}

	// Calculate the projection matrix with some screen args
	static Matrix CalcProjectionMat(float AspectRatio, float Fov, float Near, float Far)
	{
		Matrix Result = {};

		float FovRads = 1.0f / tanf(ToRad(Fov * 0.5f));

		Result.fMatrix[0][0] = AspectRatio / FovRads;
		Result.fMatrix[1][1] = -FovRads;  // THIS NEGATIVE IS FOR TERRAGL
		Result.fMatrix[2][2] = Far / (Far - Near);
		Result.fMatrix[3][2] = (-Far * Near) / (Far - Near);
		Result.fMatrix[2][3] = 1.0f;
		Result.fMatrix[3][3] = 0.0f;

		return Result;
	}

	//Calculate projection matrix of this camera
	void CalcProjectionMat()
	{
		float FovRads = abs(1.0f / tanf((ToRad(Fov * 0.5f))));

		this->ProjectionMatrix.fMatrix[0][0] = this->AspectRatio / FovRads;
		this->ProjectionMatrix.fMatrix[1][1] = -FovRads; // THIS NEGATIVE IS FOR TERRAGL
		this->ProjectionMatrix.fMatrix[2][2] = this->Far / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[3][2] = (-this->Far * this->Near) / (this->Far - this->Near);
		this->ProjectionMatrix.fMatrix[2][3] = 1.0f;
		this->ProjectionMatrix.fMatrix[3][3] = 0.0f;
	}

	Triangle ProjectTriangle(const Triangle* InTriangle, Matrix& Mat)
	{
		Triangle OutTri;

		OutTri.Points[0] = InTriangle->Points[0] * Mat;
		OutTri.Points[1] = InTriangle->Points[1] * Mat;
		OutTri.Points[2] = InTriangle->Points[2] * Mat;

		return OutTri;
	}

	Mesh ProjectMesh(Mesh InMesh)
	{
		Mesh OutMesh = Mesh();

		for (int i = 0; i < InMesh.Triangles.size(); i++)
		{
			Triangle NewTri;

			NewTri.Points[0] = InMesh.Triangles.at(i).Points[0] * this->ProjectionMatrix;
			NewTri.Points[1] = InMesh.Triangles.at(i).Points[1] * this->ProjectionMatrix;
			NewTri.Points[2] = InMesh.Triangles.at(i).Points[2] * this->ProjectionMatrix;

			OutMesh.Triangles.push_back(NewTri);
		}

		return OutMesh;
	}

	Triangle ProjectTriangle(const Triangle* InTriangle)
	{
		Triangle OutTri = *InTriangle;

		OutTri.Points[0] = InTriangle->Points[0] * this->ProjectionMatrix;
		OutTri.Points[1] = InTriangle->Points[1] * this->ProjectionMatrix;
		OutTri.Points[2] = InTriangle->Points[2] * this->ProjectionMatrix;

		return OutTri;
	}

	void ProjectTriangle(const Triangle* InTriangle, Triangle& OutTri)
	{
		OutTri.Points[0] = InTriangle->Points[0] * this->ProjectionMatrix;
		OutTri.Points[1] = InTriangle->Points[1] * this->ProjectionMatrix;
		OutTri.Points[2] = InTriangle->Points[2] * this->ProjectionMatrix;
	}

	Triangle TriangleProjected(const Triangle* InTriangle)
	{
		return
		{
			InTriangle->Points[0] * this->ProjectionMatrix,
			InTriangle->Points[1] * this->ProjectionMatrix,
			InTriangle->Points[2] * this->ProjectionMatrix
		};
	}

	static Matrix PointAt(const Vec3& CamPos, const Vec3& Target, const Vec3& Up)
	{
		Vec3 NewForward = (Target - CamPos).Normalized();

		Vec3 NewUp = (Up - (NewForward * Up.Dot(NewForward))).Normalized();

		Vec3 NewRight = NewUp.Cross(NewForward);

		Matrix DimensioningAndTrans;
		DimensioningAndTrans.fMatrix[0][0] = NewRight.x;	    DimensioningAndTrans.fMatrix[0][1] = NewRight.y;	    DimensioningAndTrans.fMatrix[0][2] = NewRight.z;      DimensioningAndTrans.fMatrix[0][3] = 0.0f;
		DimensioningAndTrans.fMatrix[1][0] = NewUp.x;		    DimensioningAndTrans.fMatrix[1][1] = NewUp.y;		    DimensioningAndTrans.fMatrix[1][2] = NewUp.z;         DimensioningAndTrans.fMatrix[1][3] = 0.0f;
		DimensioningAndTrans.fMatrix[2][0] = NewForward.x;		DimensioningAndTrans.fMatrix[2][1] = NewForward.y;		DimensioningAndTrans.fMatrix[2][2] = NewForward.z;    DimensioningAndTrans.fMatrix[2][3] = 0.0f;
		DimensioningAndTrans.fMatrix[3][0] = CamPos.x;			DimensioningAndTrans.fMatrix[3][1] = CamPos.y;	    	DimensioningAndTrans.fMatrix[3][2] = CamPos.z;        DimensioningAndTrans.fMatrix[3][3] = 1.0f;

		return DimensioningAndTrans;
	}

	void __inline __fastcall PointAt(const Vec3& Target)
	{
		Vec3 NewForward = (Target - this->Pos).Normalized();

		Vec3 NewUp = (this->CamUp - (NewForward * this->CamUp.Dot(NewForward))).Normalized();

		Vec3 NewRight = NewUp.Cross(NewForward);

		this->ViewMatrix.fMatrix[0][0] = NewRight.x;	    this->ViewMatrix.fMatrix[0][1] = NewRight.y;	    this->ViewMatrix.fMatrix[0][2] = NewRight.z;      this->ViewMatrix.fMatrix[0][3] = 0.0f;
		this->ViewMatrix.fMatrix[1][0] = NewUp.x;		    this->ViewMatrix.fMatrix[1][1] = NewUp.y;		    this->ViewMatrix.fMatrix[1][2] = NewUp.z;         this->ViewMatrix.fMatrix[1][3] = 0.0f;
		this->ViewMatrix.fMatrix[2][0] = NewForward.x;		this->ViewMatrix.fMatrix[2][1] = NewForward.y;		this->ViewMatrix.fMatrix[2][2] = NewForward.z;    this->ViewMatrix.fMatrix[2][3] = 0.0f;
		this->ViewMatrix.fMatrix[3][0] = this->Pos.x;		this->ViewMatrix.fMatrix[3][1] = this->Pos.y;	    this->ViewMatrix.fMatrix[3][2] = this->Pos.z;     this->ViewMatrix.fMatrix[3][3] = 1.0f;
	}

	void __inline __fastcall CalcCamViewMatrix(const Vec3& Target)
	{
		//this->ViewMatrix = this->PointAt(this->Pos, Target, this->CamUp).QuickInversed();
		this->PointAt(Target);
		this->ViewMatrix.QuickInverse();
	}

	Vec3 GetNewVelocity(const Vec3& Direction)
	{
		return Direction * this->Velocity;
	}

	Vec3 GetNewVelocity()
	{
		return this->LookDir * this->Velocity;
	}

	Vec3 Pos = Vec3(0, 0, 0);
	Vec3 ViewAngles = Vec3(0, 0, 0);
	Vec3 InitialLook = Vec3(0, 0, 1);
	Vec3 LookDir = Vec3(0, 0, 0);
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0.1f };
	float Velocity = 8.0f;

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};
	Matrix CamRotation = {};
	float Fov = 0.f;
	float AspectRatio = 0.f;
	float Near = 0.f;
	float Far = 0.f;
};