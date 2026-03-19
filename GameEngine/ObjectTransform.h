#pragma once
#include "Math.h"

class ObjectTransform
{
private:
	Vec3 LocalPosition = Vec3(1.0f, 5.0f, 1.0f);
	Vec3 LocalScale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 LocalEulerRotation = Vec3(0.0f, 0.0f, 0.0f);

	bool IsWorldMatrixDirty = false;
	bool IsNormalMatrixDirty = false;
	bool IsInverseWorldDirty = false;

	void RecalculateLocalMatrix()
	{
		this->Local = Matrix::CreateScalarMatrix(LocalScale) * Matrix::CreateRotationMatrix(LocalEulerRotation) * Matrix::CreateTranslationMatrix(LocalPosition);
		this->MarkDirty();
	}

	void RecalculateLocalVec3()
	{
		return;
		//Local.Decompose(this->LocalScale, this->LocalEulerRotation, this->LocalPosition);
	}

	void MarkDirty()
	{
		if (IsWorldMatrixDirty)
			return;

		IsWorldMatrixDirty = true;
		IsNormalMatrixDirty = true;
		IsInverseWorldDirty = true;

		for (auto* c : Children)
			c->MarkDirty();
	}

	Matrix World = Matrix::CreateIdentity();
	Matrix Normal = Matrix::CreateIdentity();
	Matrix Local = Matrix::CreateIdentity();
	Matrix InverseWorld = Matrix::CreateIdentity();
public:

	ObjectTransform()
	{
		LocalPosition = Vec3(1.0f, 5.0f, 1.0f);
		LocalScale = Vec3(1.0f, 1.0f, 1.0f);
		LocalEulerRotation = Vec3(0.0f, 0.0f, 0.0f);

		this->MarkDirty();
	}


	ObjectTransform(const Vec3& Pos, const Vec3& scale = Vec3(1.0f, 1.0f, 1.0f), const Vec3& euler = Vec3(0.0f, 0.0f, 0.0f)) : LocalPosition(Pos), LocalScale(scale), LocalEulerRotation(euler)
	{
		RecalculateLocalMatrix();
		this->MarkDirty();
	}


	Vec3 GetWorldRight()
	{
		return GetWorldMatrix().GetRight().Normalized();
	}

	Vec3 GetWorldUp()
	{
		return GetWorldMatrix().GetUp().Normalized();
	}


	Vec3 GetWorldForward() 
	{
		return this->GetWorldMatrix().GetForward();
	}


	Vec3 GetLocalForward() const
	{
		Vec3 f = this->Local.GetForward();

		if (f.Magnitude() > 1e-6f)
			return f.Normalized();

		Vec3 FallbackForward = Vec3::EulerToDirection(this->GetLocalEulerAngles());

		return FallbackForward.Normalized();
	}
	

	Matrix* _GetWorldMatrixPtr()
	{
		this->UpdateTransformChain();
		return &this->World;
	}


	Matrix* _GetLocalMatrixPtr()
	{
		return &this->Local;
	}


	Matrix GetWorldMatrix()
	{
		this->UpdateTransformChain();

		return this->World;
	}


	Matrix GetInverseWorldMatrix()
	{
		UpdateTransformChain();

		if (IsInverseWorldDirty)
		{
			InverseWorld = World.Inversed();
			IsInverseWorldDirty = false;
		}


		if (IsNormalMatrixDirty)
		{
			Matrix Normalt = InverseWorld;
			Normalt.Transpose();
			Normal = Normalt;
			IsNormalMatrixDirty = false;
		}

		return InverseWorld;
	}


	Matrix GetNormalMatrix()
	{
		UpdateTransformChain();

		if (IsNormalMatrixDirty)
		{
			Normal = World.Inversed();
			Normal.Transpose();
			IsNormalMatrixDirty = false;
		}

		return Normal;
	}


	Vec3 GetWorldPosition()
	{
		Matrix world = this->GetWorldMatrix(); // walks full parent chain
		return world.GetTranslation();
	}


	Vec3 GetWorldRotation()
	{
		Matrix m = this->GetWorldMatrix();
		return m.ExtractEuler();
	}


	void SetWorldPosition(const Vec3& pos)
	{
		if (this->Parent)
		{
			Matrix invParent = Parent->GetWorldMatrix().Inversed();
			this->LocalPosition = pos * invParent;
		}
		else
			this->LocalPosition = pos;

		RecalculateLocalMatrix();
	}


	void SetWorldEulerAngles(const Vec3& r) 
	{ 
		if (this->Parent)
		{
			Matrix invParent = Parent->GetWorldMatrix().Inversed();
			this->LocalEulerRotation = r * invParent;
		}
		else
			this->LocalEulerRotation = r;

		RecalculateLocalMatrix(); 
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

		Matrix invParent = Parent ? Parent->GetWorldMatrix().Inversed() : Matrix::CreateIdentity();
		Matrix newLocal = invParent * worldBefore;

		this->SetLocalMatrix(&newLocal);
	}


	void AddChild(ObjectTransform* child)
	{
		child->SetParent(this);
	}


	// take const ref instead somehow
	void SetLocalMatrix(Matrix* m)
	{
		// TODO make a decomposition step
		LocalScale = m->ExtractScale();
		LocalPosition = m->GetTranslation();
		LocalEulerRotation = m->ExtractEuler();
		//m.Decompose(LocalScale, LocalEulerRotation, LocalPosition);
		RecalculateLocalMatrix();
	}


	void UpdateTransformChain()
	{
		if (Parent)
			this->Parent->UpdateTransformChain();

		if (!this->IsWorldMatrixDirty)
			return;

		if (Parent)
			this->World = Parent->World * Local;
		else
			this->World = Local;

		this->IsWorldMatrixDirty = false;
		this->IsInverseWorldDirty = true;
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
