#pragma once
#include "Math.h"

class ObjectTransform
{
private:
	Vec3 LocalPosition = Vec3(1.0f, 5.0f, 1.0f);
	Vec3 LocalScale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 LocalEulerRotation;

	void RecalculateLocalMatrix()
	{
		this->Local = Matrix::CreateScalarMatrix(LocalScale) * Matrix::CreateRotationMatrix(LocalEulerRotation) * Matrix::CreateTranslationMatrix(LocalPosition);
	}

	void RecalculateLocalVec3()
	{
		Local.Decompose(this->LocalScale, this->LocalEulerRotation, this->LocalPosition);
	}

public:
	Matrix World = Matrix::CreateIdentity();
	Matrix Normal = Matrix::CreateIdentity();
	Matrix Local = Matrix::CreateIdentity();

	ObjectTransform() = delete;

	static Vec3 DirToEuler(const Vec3& d_raw)
	{
		Vec3 d = d_raw.Normalized();
		// clamp to avoid asin domain errors
		float safeY = std::clamp(-d.y, -0.999999f, 0.999999f);
		float pitch = asinf(safeY);
		float yaw = atan2f(d.x, d.z);
		float roll = 0.0f;
		return Vec3(ToDegree(pitch), ToDegree(yaw), ToDegree(roll)).AngleNormalized();

	}


	Vec3 GetWorldForward() 
	{
		this->WalkTransformChain();
		const Matrix& m = (World != Matrix::CreateIdentity()) ? World : Local;
		Vec3 forward(m.fMatrix[2][0], m.fMatrix[2][1], m.fMatrix[2][2]);
		return forward.Normalized();
	}

	Vec3 GetLocalForward() const
	{
		// Extract forward from local matrix (row-major row 2)
		Vec3 f(Local.fMatrix[2][0], Local.fMatrix[2][1], Local.fMatrix[2][2]);

		if (f.Magnitude() > 0.00001f)
			return f.Normalized();

		// ---- FALLBACK: compute from Euler ----
		const float pitch = LocalEulerRotation.x;
		const float yaw = LocalEulerRotation.y;

		float cp = cosf(pitch);
		float sp = sinf(pitch);
		float cy = cosf(yaw);
		float sy = sinf(yaw);

		Vec3 fallbackForward(
			sy * cp,   // x
			-sp,       // y   (note the minus — matches your DirToEuler convention)
			cy * cp    // z
		);

		return fallbackForward.Normalized();
	}


	ObjectTransform(const Vec3& Pos, const Vec3& scale = Vec3(1.0f, 1.0f, 1.0f), const Vec3& euler = Vec3(0.0f, 0.0f, 0.0f)) : LocalPosition(Pos), LocalScale(scale), LocalEulerRotation(euler)
	{
		RecalculateLocalMatrix();
	}


	Matrix GetLocalRotationMatrix() const
	{
		Matrix R = Matrix::CreateIdentity();
		// Copy just the rotation part of Local (upper-left 3x3)
		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				R.fMatrix[row][col] = Local.fMatrix[row][col];
			}
		}

		R.fMatrix[0][3] = R.fMatrix[1][3] = R.fMatrix[2][3] = 0.0f;
		R.fMatrix[3][0] = R.fMatrix[3][1] = R.fMatrix[3][2] = 0.0f;

		return R;
	}


	Matrix GetWorldMatrix() const
	{
		Matrix m = Local;
		for (const ObjectTransform* p = Parent; p; p = p->Parent)
			m = p->Local * m;
		return m;
	}

	//static Matrix PointAt(const Vec3& CamPos, const Vec3& Target, const Vec3& Up)
	//{
	//	Vec3 NewForward = (Target - CamPos).Normalized();
	//	Vec3 NewUp = (Up - (NewForward * Up.Dot(NewForward))).Normalized();
	//	Vec3 NewRight = NewUp.Cross(NewForward);
	//	Matrix DimensioningAndTrans;
	//	DimensioningAndTrans.fMatrix[0][0] = NewRight.x;	    DimensioningAndTrans.fMatrix[0][1] = NewRight.y;	    DimensioningAndTrans.fMatrix[0][2] = NewRight.z;      DimensioningAndTrans.fMatrix[0][3] = 0.0f;
	//	DimensioningAndTrans.fMatrix[1][0] = NewUp.x;		    DimensioningAndTrans.fMatrix[1][1] = NewUp.y;		    DimensioningAndTrans.fMatrix[1][2] = NewUp.z;         DimensioningAndTrans.fMatrix[1][3] = 0.0f;
	//	DimensioningAndTrans.fMatrix[2][0] = NewForward.x;		DimensioningAndTrans.fMatrix[2][1] = NewForward.y;		DimensioningAndTrans.fMatrix[2][2] = NewForward.z;    DimensioningAndTrans.fMatrix[2][3] = 0.0f;
	//	DimensioningAndTrans.fMatrix[3][0] = CamPos.x;			DimensioningAndTrans.fMatrix[3][1] = CamPos.y;	    	DimensioningAndTrans.fMatrix[3][2] = CamPos.z;        DimensioningAndTrans.fMatrix[3][3] = 1.0f;
	//	return DimensioningAndTrans;
	//}

	Vec3 GetUpVector()
	{
		Matrix world = this->GetWorldMatrix();
		return Vec3(world._21, world._22, world._23).Normalized();
	}

	Matrix CalculateViewMatrix()
	{
		Matrix WorldMat = this->GetWorldMatrix();

		WorldMat = WorldMat.GetRTMat();

		WorldMat.QuickInverse();
		return WorldMat;
	}


	void SetWorldPosition(const Vec3& pos)
	{
		if (this->Parent)
		{
			Matrix invParent = Parent->GetWorldMatrix().InverseSRT();
			this->LocalPosition = Vec3((invParent * Matrix::CreateTranslationMatrix(pos)).fMatrix[3][0], (invParent * Matrix::CreateTranslationMatrix(pos)).fMatrix[3][1], (invParent * Matrix::CreateTranslationMatrix(pos)).fMatrix[3][2]);
		}
		else
			this->LocalPosition = pos;

		RecalculateLocalMatrix();
	}


	Vec3 GetWorldPosition() const
	{
		Matrix world = GetWorldMatrix(); // walks full parent chain
		return Vec3(world.fMatrix[3][0], world.fMatrix[3][1], world.fMatrix[3][2]);
	}


	void SetParent(ObjectTransform* NewParent)
	{
		// Remove from old parent
		if (Parent)
		{
			auto& otherChildren = Parent->Children;
			otherChildren.erase(std::remove(otherChildren.begin(), otherChildren.end(), this), otherChildren.end());
		}

		Matrix worldBefore = GetWorldMatrix();

		Parent = NewParent;
		if (Parent) Parent->Children.push_back(this);

		Matrix invParent = Parent ? Parent->GetWorldMatrix().InverseSRT() : Matrix::CreateIdentity();
		Local = invParent * worldBefore;
		Local.Decompose(LocalScale, LocalEulerRotation, LocalPosition);
		RecalculateLocalMatrix();

	}


	void AddChild(ObjectTransform* child)
	{
		child->SetParent(this);
	}


	void PointAt(const Vec3& Target, const Vec3& Up = Vec3(0.0f, 1.0f, 0.0f))
	{
		// Make sure world is up-to-date
		this->WalkTransformChain();

		Vec3 forward = (Target - this->GetWorldPosition()).Normalized();;
		Vec3 upProj = (Up - forward * Up.Dot(forward)).Normalized();
		Vec3 right = upProj.Cross(forward);

		Matrix R = Matrix::CreateMatrixFromRigthForwardUp(right, upProj, forward, LocalPosition);

		// Build Local matrix
		Local = R;

		// --- Step 4: update LocalPosition/Scale/Rotation vectors ---
		this->RecalculateLocalVec3();

		this->WalkTransformChain();

	}


	void WalkTransformChain()
	{
		this->World = this->Parent ? this->Parent->World * this->Local : this->Local;
		for (auto* c : Children) c->WalkTransformChain();
	}


	const Vec3& GetLocalPosition() const { return LocalPosition; }
	const Vec3& GetLocalScale() const { return LocalScale; }
	const Vec3& GetLocalEulerAngles() const { return LocalEulerRotation; }
	const Vec3& GetLocalLookDirection() const { return this->GetLocalForward(); }
	const Vec3& GetLookDirection() { return this->GetWorldForward(); }

	void SetLocalPosition(const Vec3& pos) { LocalPosition = pos; RecalculateLocalMatrix(); }
	void SetLocalScale(const Vec3& s) { LocalScale = s; RecalculateLocalMatrix(); }
	void SetLocalEulerAngles(const Vec3& r) { LocalEulerRotation = r; RecalculateLocalMatrix(); }
	void SetLocalEulerAnglesFromDirection(const Vec3& LookDirection) { LocalEulerRotation = DirToEuler(LookDirection); RecalculateLocalMatrix(); }

public:
	ObjectTransform* Parent = nullptr;
	std::vector<ObjectTransform*> Children;
};
