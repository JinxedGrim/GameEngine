#pragma once
#include "Math.h"
#include "GameObject.h"
#include "RayCaster.h"

//GameObject(Vec3(1.0f, 1.0f, 1.0f), DirToEuler(LightDir)

enum class CameraStyles
{
	FirstPerson = 0,
	Orthographic = 1
};

class Camera: public GameObject
{
private:
	Vec3 CamUp = Vec3(0, 1, 0);
	Vec3 NearPlane = { 0, 0, 0.1f };

	Matrix ProjectionMatrix = {};
	Matrix ViewMatrix = {};

	bool IsViewDirty = true;
	bool IsProjectionDirty = true;
	bool ForceViewMatrix = false;
	bool ForceProjectionMatrix = false;
	// Projection Data
	float Fov = 90.0f;
	float AspectRatio = 0.f;
	float Near = 0.1f;
	float Far = 50.f;

	// For orthographic projection
	float Top = 0.0f;
	float Bottom = 0.0f;
	float Left = 0.0f;
	float Right = 0.0f;

	Vec3 CenterPoint = Vec3(0.0f, 0.0f, 0.0f);

	CameraStyles _cameraStyle = CameraStyles::FirstPerson;

	void __inline __fastcall CalcCamViewMatrix(const Vec3& TargetPos)
	{
		this->ViewMatrix = Matrix::CalcViewMatrix(this->Transform.GetWorldPosition(), TargetPos, this->CamUp);
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

		this->_cameraStyle = CameraStyles::FirstPerson;

		this->Transform.SetLocalPosition(Position);

		this->IsViewDirty = true;
		this->IsProjectionDirty = true;

		this->GetViewMatrix();
		this->GetProjectionMatrix();
	}

	Camera(float Top, float Bottom, float Left, float Right, float AspectRatio, float Fov, float Near, float Far) : GameObject(Vec3(0, 0, 0))
	{
		this->AspectRatio = AspectRatio;
		this->Fov = Fov;
		this->Near = Near;
		this->Far = Far;
		this->NearPlane = { 0.0f, 0.0f, Near };
		this->CamUp = { 0.0f, 1.0f, 0.0f };

		this->Top = Top;
		this->Bottom = Bottom;
		this->Left = Left;
		this->Right = Right;

		this->_cameraStyle = CameraStyles::FirstPerson;

		this->IsViewDirty = true;
		this->IsProjectionDirty = true;

		this->GetViewMatrix();
		this->GetProjectionMatrix();
	}


	void SetViewMatrix(const Matrix& ViewMat)
	{
		this->ViewMatrix = ViewMat;
	}


	void SetProjectionMatrix(const Matrix& Projection)
	{
		this->ProjectionMatrix = Projection;
		this->IsProjectionDirty = false;
		this->IsViewDirty = false;
	}


	void SetLeft(float left)
	{
		this->Left = left;
		this->IsViewDirty = true;
	}


	void SetTop(float top)
	{
		this->IsViewDirty = true;
		this->Top = top;
	}


	void SetRight(float right)
	{
		this->Right = right;
		this->IsViewDirty = true;

	}


	void SetBottom(float bottom)
	{
		this->Bottom = bottom;
		this->IsViewDirty = true;

	}


	void ChangeStyle(CameraStyles NewStyle)
	{
		this->_cameraStyle = NewStyle;
		this->IsProjectionDirty = true;
		this->IsViewDirty = true;
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
			switch (this->_cameraStyle)
			{
			case CameraStyles::FirstPerson:
				this->ViewMatrix = this->Transform.Local.CalcInverseView(this->CamUp);
				break;
			case CameraStyles::Orthographic:
				this->ViewMatrix = Matrix::CalcViewMatrix(((this->CenterPoint - this->Transform.GetLocalPosition()).Normalized()) * 50.0f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0, 1, 0));
				break;
			default:
				throw;
				break;
			}
			IsViewDirty = false;
		}

		return this->ViewMatrix;
	}


	Matrix GetProjectionMatrix()
	{
		if (this->IsProjectionDirty)
		{
			switch (this->_cameraStyle)
			{
			case CameraStyles::FirstPerson:
				this->ProjectionMatrix = Matrix::CalcPerspectiveMatrix(this->Fov, this->AspectRatio, this->Near, this->Far);
				break;
			case CameraStyles::Orthographic:
				this->ProjectionMatrix = Matrix::CalcOrthoMatrix(this->Left, this->Right, this->Bottom, this->Top, this->Near, this->Far);
				break;
			}

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


	Ray ScreenPointToRay(int sx, int sy, int ScreenWidth, int ScreenHeight)
	{
		float NdcX = (2.0f * sx / ScreenWidth) - 1.0f;
		float NdcY = 1.0f - (2.0f * sy / ScreenHeight);

		Vec4 NearClip = Vec4(NdcX, NdcY, 0.0f, 1.0f);
		Vec4 FarClip = Vec4(NdcX, NdcY, 1.0f, 1.0f);

		Vec4 ViewSpaceNearClip = NearClip * this->GetProjectionMatrix().Inversed();
		Vec4 ViewSpaceFarClip = NearClip * this->GetProjectionMatrix().Inversed();
		ViewSpaceNearClip.CorrectPerspective();
		ViewSpaceFarClip.CorrectPerspective();

		Vec4 WorldSpaceNearClip = ViewSpaceNearClip * this->GetViewMatrix().Inversed();
		Vec4 WorldSpaceFarClip = ViewSpaceFarClip * this->GetViewMatrix().Inversed();

		Ray Out = Ray(WorldSpaceNearClip, (WorldSpaceFarClip.xyz() - WorldSpaceNearClip.xyz()).Normalized());

		return Out;
	}


	Matrix3x3 GetRotationMatrix()
	{
		return this->Transform.GetWorldMatrix().GetBasis3x3();
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