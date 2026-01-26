#pragma once
#include "Collider.h"

class GameObject
{
public:
	GameObject() = delete;


	GameObject(const Vec3& Scalar, const Vec3& EulerRotatiom, const Vec3& Pos, const bool PhysicsEnabled = false)
	{
		this->Transform = ObjectTransform(Pos, Scalar, EulerRotatiom);
		this->collider.PhysicsEnabled = false;
	}

	GameObject(const Vec3& Pos, const bool PhysicsEnabled = false)
	{
		this->Transform = ObjectTransform(Pos, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));
		this->collider.PhysicsEnabled = false;
	}


	void AddAABBCollider(Vec3 Offset, Vec3 HalfExtents, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		AABBColliderParams aabbParams;
		aabbParams.offset = Offset;
		aabbParams.halfExtents = HalfExtents;
		AddAABBCollider(aabbParams, MassInKg, Restitution, InitialVelocity);
	}


	void AddAABBCollider(const AABBColliderParams& Params, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		this->collider.type = ColliderType::AABB;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::AABB, &this->Transform, (void*)&Params);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}


	void AddSphereCollider(Vec3 Offset, float Radius, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		SphereColliderParams sphereParams;
		sphereParams.Offset = Offset;
		sphereParams.radius = Radius;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Sphere, &this->Transform, (void*)&sphereParams);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}


	void AddSphereCollider(const SphereColliderParams& Params, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Sphere, &this->Transform, (void*)&Params);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}


	void AddOBBCollider(Vec3 Offset, Vec3 HalfExtents, const Matrix3x3& Orientation, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		OBBColliderParams obbParams;
		obbParams.offset = Offset;
		obbParams.halfExtents = HalfExtents;
		obbParams.orientation = Orientation;
		AddOBBCollider(obbParams, MassInKg, Restitution, InitialVelocity);
	}


	void AddOBBCollider(const OBBColliderParams& Params, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::OBB, &this->Transform, (void*)&Params);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}


	void AddCapsuleCollider(Vec3 Offset, float Radius, float HalfHeight, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		CapsuleColliderParams capParams;
		capParams.offset = Offset;
		capParams.radius = Radius;
		capParams.halfHeight = HalfHeight;
		AddCapsuleCollider(capParams, MassInKg, Restitution, InitialVelocity);
	}


	void AddCapsuleCollider(const CapsuleColliderParams& Params, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Capsule, &this->Transform, (void*)&Params);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}


	void AddBodyWithoutCollider(float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
	{
		this->collider.PhysicsEnabled = true;
		collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::None, &this->Transform, nullptr);
		this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
	}

	ObjectTransform Transform = ObjectTransform(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));
	Collider collider;
};