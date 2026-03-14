#pragma once
#include "Math.h"
#include "RayCaster.h"
#include "Collider.h"

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


    __inline Vec3 IntegrateRestitution(const Vec3& Velocity, const Vec3&  CollisionNormal, const float& Restitution, float Dt)
    {
        Vec3 VelocityDelta = Vec3(0.0f,  0.0f, 0.0f);
        float vn = Velocity.Dot(CollisionNormal);

        if (vn < 0)
        {
            VelocityDelta -= (CollisionNormal * (1.0f + Restitution) * vn);
        }

        return VelocityDelta;
    }


    void Integrate(Collider* collider, float dt, Collider* Floor, const RaycastHit* FloorHit, bool ApplyGravity = true)
    {
        // if velocity ==  0.0f sleep the obj

        if (!collider->PhysicsEnabled)
            return;

        // Apply gravity only if not grounded
        if (ApplyGravity && !collider->body.IsGrounded)
            collider->body.Velocity += IntegrateGravity(dt);
        
        if (collider->body.IsGrounded)
            collider->body.Velocity += IntegrateFriction(collider->body.KineticFriction, Floor->body.KineticFriction, collider->body.mass, dt, collider->body.Velocity);

        if (collider->type == ColliderType::None)
            return;

        Vec3 PredictedPosition = collider->GetPosition() + IntegrateVelocity(collider->body.Velocity, dt);

        collider->body.IsGrounded = false;

        // Ground collision
        if (Floor && collider->TestCollision(Floor))
        {
            PredictedPosition.y = FloorHit->point.y;
            collider->body.Velocity.y = 0.0f;

            if (collider->body.Velocity.y < 0.0f)
            {
                Vec3 RestitutionVelocity = IntegrateRestitution(collider->body.Velocity, FloorHit->normal, collider->body.restitution, dt);

                collider->body.Velocity += RestitutionVelocity;

                if (std::abs(collider->body.Velocity.y) < 0.05f) // small threshold
                {
                    collider->body.Velocity.y = 0.0f;
                    collider->body.IsGrounded = true;
                }
            }
        }

        //set y
        collider->SetEulerAngles(
            collider->GetEulerAngles() + collider->body.AngularVelocity * dt
        );

        // Integrate position (all axes)
        collider->SetPosition(PredictedPosition);
    }
}