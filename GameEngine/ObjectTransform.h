#pragma once
#include "Math.h"

class ObjectTransform
{
private:
	Vec3 LocalPosition = Vec3(1.0f, 5.0f, 1.0f);
	Vec3 LocalScale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 LocalEulerRotation = Vec3(0.0f, 0.0f, 0.0f);
	bool IsTransformDirty = false;

	void RecalculateLocalMatrix()
	{
		this->Local = Matrix::CreateScalarMatrix(LocalScale) * Matrix::CreateRotationMatrix(LocalEulerRotation) * Matrix::CreateTranslationMatrix(LocalPosition);
	}

	void RecalculateLocalVec3()
	{
		return;
		//Local.Decompose(this->LocalScale, this->LocalEulerRotation, this->LocalPosition);
	}

public:
	Matrix World = Matrix::CreateIdentity();
	Matrix Normal = Matrix::CreateIdentity();
	Matrix Local = Matrix::CreateIdentity();

	ObjectTransform()
	{
		LocalPosition = Vec3(1.0f, 5.0f, 1.0f);
		LocalScale = Vec3(1.0f, 1.0f, 1.0f);
		LocalEulerRotation = Vec3(0.0f, 0.0f, 0.0f);
	}


	Vec3 GetWorldForward() 
	{
		this->WalkTransformChain();
		Matrix m = this->GetWorldMatrix();
		return m.GetForward();
	}


	Vec3 GetLocalForward() const
	{
		Vec3 f = this->Local.GetForward();

		if (f.Magnitude() > 1e-6f)
			return f.Normalized();

		Vec3 FallbackForward = Vec3::EulerToDirection(this->GetLocalEulerAngles());

		return FallbackForward.Normalized();
	}


	ObjectTransform(const Vec3& Pos, const Vec3& scale = Vec3(1.0f, 1.0f, 1.0f), const Vec3& euler = Vec3(0.0f, 0.0f, 0.0f)) : LocalPosition(Pos), LocalScale(scale), LocalEulerRotation(euler)
	{
		RecalculateLocalMatrix();
	}


	Matrix GetWorldMatrix() const
	{
		Matrix m = Local;
		for (const ObjectTransform* p = Parent; p; p = p->Parent)
			m = p->Local * m;
		return m;
	}


	void SetWorldPosition(const Vec3& pos)
	{
		if (this->Parent)
		{
			Matrix invParent = Parent->GetWorldMatrix().InverseSRT();
			this->LocalPosition = pos * invParent;
		}
		else
			this->LocalPosition = pos;

		RecalculateLocalMatrix();
	}


	Vec3 GetWorldPosition() const
	{
		Matrix world = GetWorldMatrix(); // walks full parent chain
		return world.GetTranslation();
	}


	Vec3 GetWorldRotation() const
	{
		Matrix m = this->GetWorldMatrix();
		return m.ExtractEuler();
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
		RecalculateLocalMatrix();
	}


	void AddChild(ObjectTransform* child)
	{
		child->SetParent(this);
	}


	void WalkTransformChain()
	{
		this->World = this->Parent ? this->Parent->World * this->Local : this->Local;
		for (auto* c : Children) c->WalkTransformChain();
	}

	Matrix* _GetWorldMatrixPtr()
	{
		this->WalkTransformChain();
		return &this->World;
	}

	Matrix* _GetLocalMatrixPtr()
	{
		return &this->Local;
	}


	const Vec3& GetLocalPosition() const { return LocalPosition; }
	const Vec3& GetLocalScale() const { return LocalScale; }
	const Vec3& GetLocalEulerAngles() const { return LocalEulerRotation; }
	const Vec3& GetLocalLookDirection() const { return this->GetLocalForward(); }
	const Vec3& GetLookDirection() { return this->GetWorldForward(); }

	void SetLocalPosition(const Vec3& pos) { LocalPosition = pos; RecalculateLocalMatrix(); }
	void SetLocalScale(const Vec3& s) { LocalScale = s; RecalculateLocalMatrix(); }
	void SetLocalEulerAngles(const Vec3& r) { LocalEulerRotation = r; RecalculateLocalMatrix(); }
	//void SetLocalEulerAnglesFromDirection(const Vec3& LookDirection) { LocalEulerRotation = DirToEuler(LookDirection); RecalculateLocalMatrix(); }

public:
	ObjectTransform* Parent = nullptr;
	std::vector<ObjectTransform*> Children;
};
