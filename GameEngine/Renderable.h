#pragma once
#include "EngineCore.h"
#include "Collider.h"

struct ObjectTransform
{
	Matrix Model;
	Matrix Normal;
};

namespace TerraPGE
{
	class Renderable
	{
	public:
		Renderable() = delete;

		Renderable(Mesh* mesh, Camera* Cam, const Vec3& Scalar, const Vec3& RotationRads, const Vec3& Pos, const std::function<void(ShaderArgs*)> Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT)
		{
			this->Cam = Cam;
			this->Pos = Pos;
			this->mesh = mesh;
			this->Scalar = Scalar;
			this->RotationRads = RotationRads;
			this->Shader = Shader;
			this->SHADER_TYPE = SHADER_TYPE;
			this->collider.PhysicsEnabled = false;
			UpdateTransform();
		}


		void UpdateTransform()
		{
			const Matrix ObjectMatrix = Matrix::CreateScalarMatrix(Scalar); // Scalar Matrix
			const Matrix RotM = Matrix::CreateRotationMatrix(RotationRads); // Rotation Matrix
			const Matrix TransMat = Matrix::CreateTranslationMatrix(Pos); // Translation Matrix
			this->Transform.Model = ((ObjectMatrix * RotM) * TransMat); // Matrices are applied in SRT order 

			this->collider.UpdatedRigidBody(this->Pos);
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
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::AABB, (void*)&Params);
			this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
		}


		void AddSphereCollider(Vec3 Offset, float Radius, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
		{
			this->collider.PhysicsEnabled = true;
			SphereColliderParams sphereParams;
			sphereParams.Offset = Offset;
			sphereParams.radius = Radius;
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Sphere, (void*)&sphereParams);
			this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
		}

		void AddSphereCollider(const SphereColliderParams& Params, float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
		{
			this->collider.PhysicsEnabled = true;
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Sphere, (void*)&Params);
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
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::OBB, (void*)&Params);
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
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::Capsule, (void*)&Params);
			this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
		}


		void AddBodyWithoutCollider(float MassInKg, float Restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f))
		{
			this->collider.PhysicsEnabled = true;
			collider = Collider(RigidBody(MassInKg, Restitution, InitialVelocity), ColliderType::None, nullptr);
			this->collider.AddRigidBody(MassInKg, Restitution, InitialVelocity);
		}


		void UpdatePostPhysics()
		{
			this->Pos = this->collider.body.Position;
		}


		Mesh* mesh = nullptr;
		ObjectTransform Transform;
		Vec3 Pos = Vec3();
		Vec3 Scalar = Vec3();
		Vec3 RotationRads = Vec3();
		Camera* Cam;
		Collider collider;

		std::function<void(ShaderArgs*)> Shader = EngineShaders::Shader_Frag_Phong_Shadows;
		ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT;
	};
}