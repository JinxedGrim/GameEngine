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
		this->Transform = ObjectTransform(Position, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));

		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };

		this->CalcProjectionMat();
	}

	Camera(Vec3 Position, Vec3 TargetLook, Vec3 CamUp, float AspectRatio, float Fov, float Near, float Far)
	{
		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->CamUp = CamUp;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->CalcProjectionMat();
		this->PointAt(Position, TargetLook, CamUp);
		this->Transform = ObjectTransform(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));

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
		this->ProjectionMatrix = Matrix::CalcPerspectiveMatrix(this->Fov, this->AspectRatio, this->Near, this->Far);
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


	void PointAt(const Vec3& Target)
	{
		this->ViewMatrix = this->PointAt(this->Transform.GetLocalPosition(), Target, this->CamUp);
	}


	void LookAt(const Vec3& Target)
	{
		Vec3 forward = (Target - Transform.GetWorldPosition()).Normalized();
		Vec3 right = CamUp.Cross(forward).Normalized();
		Vec3 up = forward.Cross(right);

		// Build view matrix
		ViewMatrix = Matrix::CreateIdentity();
		ViewMatrix.fMatrix[0][0] = right.x;   ViewMatrix.fMatrix[0][1] = right.y;   ViewMatrix.fMatrix[0][2] = right.z;
		ViewMatrix.fMatrix[1][0] = up.x;      ViewMatrix.fMatrix[1][1] = up.y;      ViewMatrix.fMatrix[1][2] = up.z;
		ViewMatrix.fMatrix[2][0] = forward.x; ViewMatrix.fMatrix[2][1] = forward.y; ViewMatrix.fMatrix[2][2] = forward.z;
		ViewMatrix.fMatrix[3][0] = Transform.GetWorldPosition().x;
		ViewMatrix.fMatrix[3][1] = Transform.GetWorldPosition().y;
		ViewMatrix.fMatrix[3][2] = Transform.GetWorldPosition().z;
	}


	void __inline __fastcall CalcCamViewMatrix()
	{
		//this->ViewMatrix = this->PointAt(this->Pos, Target, this->CamUp).QuickInversed();
		this->Transform.WalkTransformChain();  // make sure world matrix is up to date
		this->ViewMatrix = this->Transform.World.QuickInversed();
	}


	Vec3 GetNewVelocity(const Vec3& Direction)
	{
		return Direction * this->Velocity;
	}


	Vec3 GetLookDirection() const
	{
		// The forward vector is usually the Z-axis in local space
		// If your convention is +Z forward:
		Vec3 forward(
			Transform.World.fMatrix[2][0],
			Transform.World.fMatrix[2][1],
			Transform.World.fMatrix[2][2]
		);

		return forward.Normalized();
	}


	Vec3 GetForward() const
	{
		// Assuming row-major and SRT order (Model = Parent * Local)
		Matrix world = Transform.World; // already walked
		return Vec3(world.fMatrix[2][0], world.fMatrix[2][1], world.fMatrix[2][2]).Normalized();
	}

	Vec3 GetNewVelocity() const
	{
		return GetForward() * Velocity;
	}

	
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0.1f };
	float Velocity = 8.0f;

	ObjectTransform Transform = ObjectTransform(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));;

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};
	float Fov = 0.f;
	float AspectRatio = 0.f;
	float Near = 0.f;
	Vec3 InitialLook = Vec3(0.0f, 0.0f, 1.0f);
	float Far = 0.f;
};