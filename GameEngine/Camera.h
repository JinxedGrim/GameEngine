#pragma once
#include "Math.h"
#include "GameObject.h"

//GameObject(Vec3(1.0f, 1.0f, 1.0f), DirToEuler(LightDir)

class Camera: public GameObject
{
	public:
	Camera() = delete;

	Camera(Vec3 Position, float AspectRatio, float Fov, float Near, float Far) : GameObject(Position)
	{
		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };

		this->CalcProjectionMat();
	}

	Camera(Vec3 Position, Vec3 TargetLookPos, Vec3 CamUp, float AspectRatio, float Fov, float Near, float Far) : GameObject(Position)
	{
		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->CamUp = CamUp;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->Transform.PointAt(TargetLookPos, CamUp);
		this->CalcProjectionMat();
	}


	//Calculate projection matrix of this camera
	Matrix CalcProjectionMat()
	{
		return Matrix::CalcPerspectiveMatrix(this->Fov, this->AspectRatio, this->Near, this->Far);
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


	Matrix __inline __fastcall CalcCamViewMatrix()
	{
		return this->Transform.CalculateViewMatrix();
	}


	Vec3 GetNewVelocity(const Vec3& Direction)
	{
		return Direction * this->Velocity;
	}


	Vec3 GetLookDirection()
	{
		return this->GetForward().Normalized();
	}


	Vec3 GetForward()
	{
		// Assuming row-major and SRT order (Model = Parent * Local)
		Matrix world = Transform.GetWorldMatrix(); // already walked
		return Transform.GetWorldForward();
	}

	Vec3 GetNewVelocity()
	{
		return GetForward().Normalized() * Velocity;
	}

	
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0.1f };
	float Velocity = 8.0f;

	Matrix ProjectionMatrix = {};
	float Fov = 90.0f;
	float AspectRatio = 0.f;
	float Near = 0.1f;
	float Far = 50.f;
};