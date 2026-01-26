#pragma once
#include "Math.h"
#include "GameObject.h"
#include "RayCaster.h"

namespace TerraPGE::Physics
{
    struct PhysicsParams
    {
        bool DoPhysics = true;
        bool DoGravity = true;
        bool ApplyFriction = true;

        // No Floor Found
        float FallbackHeight = 0.0f;
    };

    const static Vec3 GRAVITY_ACCELERATION_VECTOR = Vec3(0, -9.81, 0);  // m/s²

    //float GetFloorHeight(const Vec3& pos, Collider* self, const std::vector<Collider*>& World)
    //{
    //    Ray r(pos, Vec3(0, -1, 0));
    //    RaycastHit hit;

    //    if (Raycast(r, world, hit))
    //        return hit.point.y;
    //
    //    return -INFINITY; // no floor
    //}

    inline float KineticFrictionAverage(float mu1, float mu2)
    {
        return (mu1 + mu2) * 0.5f;
    }

    // The kinetic friction average function is an approximation to this
    inline float KineticFrictionMultiply(float mu1, float mu2)
    {
        return mu1 * mu2;
    }

    __inline Vec3 IntegrateGravity(float dt)
    {
        return GRAVITY_ACCELERATION_VECTOR * dt;
    }

    __inline Vec3 IntegrateVelocity(const Vec3& Velocity, float dt)
    {
        return Velocity * dt;
    }

    __inline Vec3 IntegrateFriction(float mu_k, float mu_k2, float mass, float dt, Vec3 Velocity, Vec3 NormalVector = Vec3(0, 1, 0))
    {
        // Get total kinetic
        float KineticFriction = KineticFrictionAverage(mu_k, mu_k2);


        // Standard physics calculation mu_k * F_n
        float NormalForce = fabs(GRAVITY_ACCELERATION_VECTOR.Dot(NormalVector)) * mass;
        float FrictionalForce = NormalForce * KineticFriction;

        // Friction direction is opposite to velocity
        //Vec3 velDir = Velocity;
        //float speed = velDir.Magnitude();

        //// normalize by magnitude for direction
        //velDir = velDir / speed;

        //// object nearly stopped
        //if (speed < 0.001f)
        //    return Vec3(0, 0, 0);

        Vec3 TangentialVelocity = Velocity - NormalVector * Velocity.Dot(NormalVector);
        float speed = TangentialVelocity.Magnitude();

        if (speed < 0.001f)
            return -TangentialVelocity;

        Vec3 velDir = TangentialVelocity / speed;

        // F = ma -> a = F/m
        Vec3 FrictionalAcceleration = -velDir * (FrictionalForce / mass);
        Vec3 DeltaVelocity = FrictionalAcceleration * dt;

        if (DeltaVelocity.Magnitude() > speed)
        {
            return -TangentialVelocity;
        }

        // Integrate the acceleration -> Gives us dv
        return DeltaVelocity;
    }

    void Integrate(Collider* collider, float dt, GameObject* Floor, const RaycastHit* FloorHit, bool ApplyGravity = true)
    {
        if (!collider->PhysicsEnabled)
            return;

        // Apply gravity only if not grounded
        if (ApplyGravity && !collider->body.IsGrounded)
            collider->body.Velocity += IntegrateGravity(dt);
        
        if (collider->body.IsGrounded)
            collider->body.Velocity += IntegrateFriction(collider->body.KineticFriction, Floor->collider.body.KineticFriction, collider->body.mass, dt, collider->body.Velocity);

        // Integrate position (all axes)
        Vec3 deltaWorld = IntegrateVelocity(collider->body.Velocity, dt);

        collider->Transform->SetLocalEulerAngles(
            collider->Transform->GetLocalEulerAngles() + collider->body.AngularVelocity * dt
        );

        collider->Transform->SetLocalPosition(collider->Transform->GetLocalPosition() + deltaWorld);

        if (collider->type == ColliderType::None)
            return;

        // Ground collision
        if (Floor && collider->TestCollision(&Floor->collider))
        {
            Vec3 worldPos = collider->Transform->GetWorldPosition();
            worldPos.y = FloorHit->point.y;
            collider->Transform->SetLocalPosition(worldPos);

            if (collider->body.Velocity.y < 0.0f)
            {
                // calculate bounce
                float v_n = collider->body.Velocity.Dot(FloorHit->normal);
                collider->body.Velocity.y = -collider->body.Velocity.y * collider->body.restitution;
                if (v_n >= 0) return;
                Vec3 j =  FloorHit->normal * (-(1 + collider->body.restitution) * v_n) / (1.0f / collider->body.mass);
                collider->body.Velocity += j / collider->body.mass;  // p += j; v = p/m

                if (std::abs(collider->body.Velocity.y) < 0.05f) // small threshold
                {
                    collider->body.Velocity.y = 0.0f;
                    collider->body.IsGrounded = true;
                }
            }
        }
        else
        {
            collider->body.IsGrounded = false;
        }
    }
}