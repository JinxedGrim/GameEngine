#pragma once
#include "EngineCore.h"
#include "Collider.h"


namespace TerraPGE
{
	class Renderable
	{
	public:
		Renderable() = delete;

		Renderable(Mesh* mesh, Camera* Cam, const Vec3& Scalar, const Vec3& EulerRotatiom, const Vec3& Pos, const std::function<void(ShaderArgs*)> Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
		{
			this->Transform = ObjectTransform(Pos, Scalar, EulerRotatiom);
			this->mesh = mesh;
			this->Shader = Shader;
			this->SHADER_TYPE = SHADER_TYPE;
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


		Mesh* mesh = nullptr;
		ObjectTransform Transform = ObjectTransform(Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f));
		Collider collider;

		std::function<void(ShaderArgs*)> Shader = EngineShaders::Shader_Frag_Phong_Shadows;
		ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT;
	};
}