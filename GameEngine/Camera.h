#pragma once
#include "Math.h"
#include "GameObject.h"

//GameObject(Vec3(1.0f, 1.0f, 1.0f), DirToEuler(LightDir)

class Camera: public GameObject
{
private:
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0.1f };

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};

	bool IsViewDirty = true;
	bool IsProjectionDirty = true;

	float Fov = 90.0f;
	float AspectRatio = 0.f;
	float Near = 0.1f;
	float Far = 50.f;


	void __inline __fastcall CalcCamViewMatrix(const Vec3& TargetPos)
	{
		this->ViewMatrix = Matrix::CalcViewMatrix(this->Transform.GetWorldPosition(), TargetPos, this->CamUp);
	}


	void __inline __fastcall CalcCamLookMatrix(const Vec3& Dir)
	{
		this->ViewMatrix = Matrix::CalcLookAtMatrix(this->Transform.GetWorldPosition(), Dir, this->CamUp);
	}

public:

	float Velocity = 8.0f;

	Camera() = delete;


	Camera(Vec3 Position, float AspectRatio, float Fov, float Near, float Far) : GameObject(Position)
	{
		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->CamUp = {0.0f, 1.0f, 0.0f};

		this->SetLocalPosition(Position);

		this->IsViewDirty = true;
		this->IsProjectionDirty = true;

		this->GetViewMatrix();
		this->GetProjectionMatrix();
	}


	void SetFov(float Fov)
	{
		this->Fov = Fov;
		this->IsProjectionDirty = true;
	}


	void SetAspectRatio(float AspectRatio)
	{
		this->AspectRatio = AspectRatio;
		this->IsProjectionDirty = true;
	}


	void SetFar(float Far)
	{
		this->Far = Far;
		this->IsProjectionDirty = true;
	}


	void SetNear(float Near)
	{
		this->Near = Near;
		this->NearPlane = Vec3(0, 0, Near);
		this->IsProjectionDirty = true;
	}


	float GetFov() const
	{
		return this->Fov;
	}


	float GetAspectRatio() const 
	{
		return this->AspectRatio;
	}


	float GetFar() const
	{
		return this->Far;
	}


	float GetNear() const 
	{
		return this->Near;	
	}



	Vec3 GetNearPLane() const
	{
		return this->NearPlane;
	}


	Matrix GetViewMatrix()
	{
		if (IsViewDirty)
		{
			this->ViewMatrix = this->Transform.GetWorldMatrix().QuickInversed();
			IsViewDirty = false;
		}

		return this->ViewMatrix;
	}


	Matrix GetProjectionMatrix()
	{
		if (this->IsProjectionDirty)
		{
			this->ProjectionMatrix = Matrix::CalcPerspectiveMatrix(this->Fov, this->AspectRatio, this->Near, this->Far);
			IsProjectionDirty = false;
		}

		return this->ProjectionMatrix;
	}


	Vec3 GetNewVelocity(const Vec3& Direction) const
	{
		return Direction * this->Velocity;
	}


	Vec3 GetLookDirection() const
	{
		return Vec3::EulerToDirection(this->Transform.GetWorldRotation());
		//return this->GetForward().Normalized();
	}


	Vec3 GetLocalViewAngles()
	{
		return this->Transform.GetLocalEulerAngles();
	}


	Vec3 GetLocalPosition()
	{
		return this->Transform.GetLocalPosition();
	}

	Vec3 GetWorldPosition()
	{
		return this->Transform.GetWorldMatrix().GetTranslation();
	}


	void SetLocalViewAngles(const Vec3& Angles) 
	{
		this->Transform.SetLocalEulerAngles(Angles);
		this->IsViewDirty = true;
	}


	Vec3 GetWorldViewAngles()
	{
		return this->Transform.GetWorldMatrix().ExtractEuler();
	}


	void SetLocalPosition(const Vec3& Pos)
	{
		this->Transform.SetLocalPosition(Pos);
		this->IsViewDirty = true;
	}


	//Vec3 GetForward()
	//{
	//	Matrix world = Transform.GetWorldMatrix(); // already walked
	//	return Transform.GetWorldForward();
	//}


	Vec3 GetNewVelocity()
	{
		return GetLookDirection().Normalized() * Velocity;
	}


	Matrix* _GetViewMatrixPtr()
	{
		if (IsViewDirty)
		{
			this->ViewMatrix = this->Transform.GetWorldMatrix().QuickInversed();
			IsViewDirty = false;
		}

		return &(this->ViewMatrix);
	}


	Matrix* _GetProjectionMatrixPtr()
	{
		if (this->IsProjectionDirty)
		{
			this->ProjectionMatrix = Matrix::CalcPerspectiveMatrix(this->Fov, this->AspectRatio, this->Near, this->Far);
			IsProjectionDirty = false;
		}

		return &(this->ProjectionMatrix);
	}
};