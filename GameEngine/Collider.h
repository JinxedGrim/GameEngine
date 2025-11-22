#pragma once
#include "Math.h"
#include "ObjectTransform.h"
#include "Mesh.h"

#define ICE_KINETIC_FRICTION 0.03
#define STEEL_KINETIC_FRICTION 0.15
#define ALUMINUM_KINETIC_FRICTION 0.25
#define COPPER_KINETIC_FRICTION 0.25
#define WOOD_KINETIC_FRICTION 0.4
#define RUBBER_KINETIC_FRICTION 0.6
#define GLASS_KINETIC_FRICTION 0.4
#define LEATHER_KINETIC_FRICTION 0.6
#define CONCRETE_KINETIC_FRICTION 0.6
#define SANDPAPER_KINETIC_FRICTION 0.6
#define PLASTIC_KINETIC_FRICTION 0.35
#define RUBBER_SOFT_KINETIC_FRICTION 0.9

#define ICE_STATIC_FRICTION 0.05
#define STEEL_STATIC_FRICTION 0.2
#define ALUMINUM_STATIC_FRICTION 0.3
#define COPPER_STATIC_FRICTION 0.3
#define WOOD_STATIC_FRICTION 0.5
#define RUBBER_STATIC_FRICTION 0.7
#define GLASS_STATIC_FRICTION 0.5
#define LEATHER_STATIC_FRICTION 0.7
#define CONCRETE_STATIC_FRICTION 0.7
#define SANDPAPER_STATIC_FRICTION 0.65
#define PLASTIC_STATIC_FRICTION 0.4
#define RUBBER_SOFT_STATIC_FRICTION 1.0

class RigidBody
{
public:
	bool IsGrounded = false;
	float mass = 5.0;   // kg
	float restitution = 0.1f;  // bounce factor [0–1]
    float KineticFriction = WOOD_KINETIC_FRICTION;
    float StaticFriction = WOOD_STATIC_FRICTION;
    float Drag = 0.0f;

	Vec3 Velocity = Vec3(0.0f, 0.0f, 0.0f);
    Vec3 AngularVelocity = Vec3(0.0f, 0.0f, 0.0f);

	RigidBody()
	{
		this->IsGrounded = false;
		this->mass = 1.0;   // kg
		this->restitution = 0.1f;  // bounce factor [0–1]
		this->Velocity = Vec3(0.0f, 0.0f, 0.0f);
	}

	RigidBody(float mass, float restitution, Vec3 InitialVelocity = Vec3(0.0f, 0.0f, 0.0f), bool IsGrounded = false)
	{
		this->IsGrounded = IsGrounded;
		this->mass = mass;   // kg
		this->restitution = restitution;  // bounce factor [0–1]
		this->Velocity = InitialVelocity;
	}
};

enum class ColliderType
{
	None,
	Sphere,
	AABB,
	OBB,
	Capsule,
};

struct SphereColliderParams
{
	float radius;
	Vec3 Offset;
};

struct AABBColliderParams
{
	Vec3 halfExtents;
	Vec3 offset;
};

struct OBBColliderParams
{
	Vec3 halfExtents;
	Matrix3x3 orientation;   // Local rotation basis (3x3)
	Vec3 offset;
};

struct CapsuleColliderParams
{
	float radius;
	float halfHeight;        // distance from center to sphere center end
    Vec3 axis;     // normalized capsule axis (direction)
	Vec3 offset;
};

class Collider
{
public:
	ColliderType type;
	RigidBody body; // maybe just a pointer to the renderable (or mesh) rigidbody
    ObjectTransform* Transform = nullptr;
	bool PhysicsEnabled = false;
	// TODO Breaak these up to seperate classes use polymorphism prolly
	SphereColliderParams  _sphereParams;
	AABBColliderParams    _AABBParams;
	OBBColliderParams     _OBBParams;
	CapsuleColliderParams _capsuleParams;

	Collider()
	{
		this->PhysicsEnabled = false;
	}

	Collider(RigidBody body, ColliderType Type, ObjectTransform* Transform, void* Params = nullptr)
	{
		this->type = Type;
		this->body = body;
        this->Transform = Transform;

		if (Params == nullptr)
			return;

		switch (type)
		{
		case ColliderType::Sphere:
			_sphereParams = *((SphereColliderParams*)Params);
			break;

		case ColliderType::AABB:
			_AABBParams = *((AABBColliderParams*)Params);
			break;

		case ColliderType::OBB:
			_OBBParams = *((OBBColliderParams*)Params);
			break;

		case ColliderType::Capsule:
			_capsuleParams = *((CapsuleColliderParams*)Params);
			break;

		case ColliderType::None:
		default:
			break;
		}
	}


	void AddRigidBody(float MassInKg, float Restitution, Vec3 InitalVelocity)
	{
		this->PhysicsEnabled = true;

        this->body = RigidBody(MassInKg, Restitution, InitalVelocity);
	}


	bool TestCollision(const Collider* other)  const
	{
        switch (this->type)
        {
        case ColliderType::Sphere:   return this->TestSphere(other);
        case ColliderType::AABB:     return this->TestAABB(other);
        case ColliderType::OBB:      return this->TestOBB(other);
        case ColliderType::Capsule:  return this->TestCapsule(other);
        default: return false;
        }
	}


    static bool TestCollision(const Collider* A, const Collider* B) 
    {
        switch (A->type)
        {
        case ColliderType::Sphere:   return A->TestSphere(B);
        case ColliderType::AABB:     return A->TestAABB(B);
        case ColliderType::OBB:      return A->TestOBB(B);
        case ColliderType::Capsule:  return A->TestCapsule(B);
        default: return false;
        }
    }
    

	Vec3 GetClosestPoint(const Vec3& point)  const
	{

	}


    static AABBColliderParams CalculateAABB(const std::vector<Triangle>& triangles)
    {
        Vec3 minP(FLT_MAX, FLT_MAX, FLT_MAX);
        Vec3 maxP(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        for (const auto& tri : triangles)
        {
            for (int i = 0; i < 3; i++)
            {
                const Vec3 v = tri.Points[i];
                minP.x = std::min(minP.x, v.x);
                minP.y = std::min(minP.y, v.y);
                minP.z = std::min(minP.z, v.z);

                maxP.x = std::max(maxP.x, v.x);
                maxP.y = std::max(maxP.y, v.y);
                maxP.z = std::max(maxP.z, v.z);
            }
        }

        AABBColliderParams result;
        result.halfExtents = (maxP - minP) * 0.5f;
        result.offset = (minP + maxP) * 0.5f;
        return result;
    }


    static SphereColliderParams CalculateBoundingSphere(const std::vector<Triangle>& triangles)
    {
        Vec3 centroid(0.0f, 0.0f, 0.0f);
        size_t count = 0;

        for (const auto& tri : triangles)
            for (int i = 0; i < 3; i++)
            {
                centroid += tri.Points[i];
                count++;
            }
        centroid = centroid / static_cast<float>(count);

        float radius = 0.0f;
        for (const auto& tri : triangles)
            for (int i = 0; i < 3; i++)
                radius = std::max(radius, (tri.Points[i] - centroid).xyz().Magnitude());

        SphereColliderParams result;
        result.Offset = centroid;
        result.radius = radius;
        return result;
    }


    //static OBBColliderParams CalculateOBB(const std::vector<Triangle>& triangles)
    //{
    //    // 1. Compute covariance matrix
    //    Vec3 mean(0.0f, 0.0f, 0.0f);
    //    size_t count = 0;
    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            mean += tri.Points[i];
    //            count++;
    //        }
    //    mean = mean / float(count);

    //    float cov[3][3] = { 0 };
    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            Vec3 p = tri.Points[i] - mean;
    //            cov[0][0] += p.x * p.x; cov[0][1] += p.x * p.y; cov[0][2] += p.x * p.z;
    //            cov[1][0] += p.y * p.x; cov[1][1] += p.y * p.y; cov[1][2] += p.y * p.z;
    //            cov[2][0] += p.z * p.x; cov[2][1] += p.z * p.y; cov[2][2] += p.z * p.z;
    //        }

    //    // 2. Divide by count
    //    for (int r = 0; r < 3; r++)
    //        for (int c = 0; c < 3; c++)
    //            cov[r][c] /= float(count);

    //    // 3. Eigenvectors of covariance = principal axes (for simplicity, assume you have a function)
    //    Matrix3x3 axes = Eigenvectors(cov); // TODO: implement or use library

    //    // 4. Project points onto axes to get min/max along each
    //    Vec3 minP(FLT_MAX, FLT_MAX, FLT_MAX);
    //    Vec3 maxP(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            Vec3 pLocal(
    //                (tri.Points[i] - mean).xyz().Dot(axes[0]),
    //                (tri.Points[i] - mean).xyz().Dot(axes[1]),
    //                (tri.Points[i] - mean).xyz().Dot(axes[2])
    //            );

    //            minP.x = std::min(minP.x, pLocal.x); maxP.x = std::max(maxP.x, pLocal.x);
    //            minP.y = std::min(minP.y, pLocal.y); maxP.y = std::max(maxP.y, pLocal.y);
    //            minP.z = std::min(minP.z, pLocal.z); maxP.z = std::max(maxP.z, pLocal.z);
    //        }

    //    OBBColliderParams result;
    //    result.offset = mean + axes[0] * ((minP.x + maxP.x) * 0.5f)     
    //        + axes[1] * ((minP.y + maxP.y) * 0.5f)
    //        + axes[2] * ((minP.z + maxP.z) * 0.5f);
    //    result.halfExtents = (maxP - minP) * 0.5f;
    //    result.orientation = axes;
    //    return result;
    //}


    //static CapsuleColliderParams CalculateCapsule(const std::vector<Triangle>& triangles)
    //{
    //    // 1. Compute covariance matrix for PCA
    //    Vec3 mean(0.0f, 0.0f, 0.0f);
    //    size_t count = 0;
    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            mean += tri.Points[i];
    //            count++;
    //        }
    //    mean = mean / float(count);

    //    float cov[3][3] = { 0 };
    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            Vec3 p = tri.Points[i] - mean;
    //            cov[0][0] += p.x * p.x; cov[0][1] += p.x * p.y; cov[0][2] += p.x * p.z;
    //            cov[1][0] += p.y * p.x; cov[1][1] += p.y * p.y; cov[1][2] += p.y * p.z;
    //            cov[2][0] += p.z * p.x; cov[2][1] += p.z * p.y; cov[2][2] += p.z * p.z;
    //        }

    //    for (int r = 0; r < 3; r++)
    //        for (int c = 0; c < 3; c++)
    //            cov[r][c] /= float(count);

    //    // 2. Compute eigenvectors -> choose eigenvector with largest eigenvalue as axis
    //    Matrix3x3 axes = Eigenvectors(cov);
    //    Vec3 axis = axes[0]; // assume axes[0] corresponds to largest eigenvalue

    //    // 3. Project points onto axis to get height
    //    float minProj = FLT_MAX, maxProj = -FLT_MAX;
    //    float radiusSq = 0.0f;
    //    for (const auto& tri : triangles)
    //        for (int i = 0; i < 3; i++)
    //        {
    //            Vec3 d = tri.Points[i] - mean;
    //            float proj = d.Dot(axis);
    //            minProj = std::min(minProj, proj);
    //            maxProj = std::max(maxProj, proj);

    //            // distance to axis for radius
    //            Vec3 closestPoint = mean + axis * proj;
    //            radiusSq = std::max(radiusSq, (tri.Points[i] - closestPoint).xyz().LengthSquared());
    //        }

    //    CapsuleColliderParams result;
    //    result.axis = axis;
    //    result.offset = mean + axis * ((minProj + maxProj) * 0.5f);
    //    result.halfHeight = (maxProj - minProj) * 0.5f;
    //    result.radius = sqrtf(radiusSq);
    //    return result;
    //}


    static inline Vec3 CapsuleSegmentA(const Vec3& p, float halfH)
    {
        return Vec3(p.x, p.y + halfH, p.z);
    }


    static inline Vec3 CapsuleSegmentB(const Vec3& p, float halfH)
    {
        return Vec3(p.x, p.y - halfH, p.z);
    }


    static inline float DistSqSegmentToPoint(const Vec3& a, const Vec3& b, const Vec3& p)
    {
        Vec3 ab = b - a;
        float t = (p - a).Dot(ab) / ab.LengthSquared();
        t = Clamp(t, 0.0f, 1.0f);
        Vec3 closest = a + ab * t;
        return (p - closest).LengthSquared();
    }


    static inline float DistSqSegmentToSegment(const Vec3& p1, const Vec3& q1, const Vec3& p2, const Vec3& q2)
    {
        // Cheap version: check endpoints only (good enough for early iterations)
        return std::min(
            std::min((p1 - p2).LengthSquared(), (p1 - q2).LengthSquared()),
            std::min((q1 - p2).LengthSquared(), (q1 - q2).LengthSquared())
        );
    }


    static inline float DistSqPointAABB(const Vec3& p, const Vec3& bPos, const Vec3& e)
    {
        float dx = 0.0f;
        if (p.x < bPos.x - e.x) dx = bPos.x - e.x - p.x;
        else if (p.x > bPos.x + e.x) dx = p.x - (bPos.x + e.x);

        float dy = 0.0f;
        if (p.y < bPos.y - e.y) dy = bPos.y - e.y - p.y;
        else if (p.y > bPos.y + e.y) dy = p.y - (bPos.y + e.y);

        float dz = 0.0f;
        if (p.z < bPos.z - e.z) dz = bPos.z - e.z - p.z;
        else if (p.z > bPos.z + e.z) dz = p.z - (bPos.z + e.z);

        return dx * dx + dy * dy + dz * dz;
    }


    static inline float DistSqSegmentToAABB(const Vec3& a, const Vec3& b, const Vec3& boxPos, const Vec3& halfExt)
    {
        Vec3 d = b - a;

        // Start with both segment endpoints
        float t0 = 0.0f;
        float t1 = 1.0f;

        // Liang-Barsky style: clip against AABB slabs to get segment interval inside the box
        auto clip = [&](float denom, float numer)
            {
                if (fabs(denom) < 1e-6f)
                {
                    // Parallel to slab
                    return numer <= 0;
                }
                float t = numer / denom;
                if (denom > 0)
                {
                    if (t > t1) return false;
                    if (t > t0) t0 = t;
                }
                else
                {
                    if (t < t0) return false;
                    if (t < t1) t1 = t;
                }
                return true;
            };

        if (!clip(d.x, (boxPos.x - halfExt.x) - a.x)) return DistSqPointAABB(a, boxPos, halfExt);
        if (!clip(-d.x, a.x - (boxPos.x + halfExt.x))) return DistSqPointAABB(a, boxPos, halfExt);

        if (!clip(d.y, (boxPos.y - halfExt.y) - a.y)) return DistSqPointAABB(a, boxPos, halfExt);
        if (!clip(-d.y, a.y - (boxPos.y + halfExt.y))) return DistSqPointAABB(a, boxPos, halfExt);

        if (!clip(d.z, (boxPos.z - halfExt.z) - a.z)) return DistSqPointAABB(a, boxPos, halfExt);
        if (!clip(-d.z, a.z - (boxPos.z + halfExt.z))) return DistSqPointAABB(a, boxPos, halfExt);

        return 0.0f;
    }


    static inline bool OBBvCapsule(const Vec3& obbPos, const Vec3& halfExt, const Matrix3x3& axes, const Vec3& capPos, float capRad, float halfH)
    {
        Vec3 a = CapsuleSegmentA(capPos, halfH);
        Vec3 b = CapsuleSegmentB(capPos, halfH);

        Vec3 aLocal(
            (a - obbPos).Dot(axes[0]),
            (a - obbPos).Dot(axes[1]),
            (a - obbPos).Dot(axes[2])
        );

        Vec3 bLocal(
            (b - obbPos).Dot(axes[0]),
            (b - obbPos).Dot(axes[1]),
            (b - obbPos).Dot(axes[2])
        );

        float distSq = DistSqSegmentToAABB(aLocal, bLocal, Vec3(0, 0, 0), halfExt);

        return distSq <= capRad * capRad;
    }


    static inline bool SphereVSphere(const Vec3& p1, float r1, const Vec3& p2, float r2)
    {
        float distSq = (p1 - p2).LengthSquared();
        float radSum = r1 + r2;
        return distSq <= radSum * radSum;
    }


    static inline bool SphereVOBB(const Vec3& spherePos, float radius, const Vec3& obbPos, const Vec3& halfExt, const Matrix3x3& axes)
    {
        Vec3 d = spherePos - obbPos;

        Vec3 local(
            d.Dot(axes[0]),
            d.Dot(axes[1]),
            d.Dot(axes[2])
        );

        float cx = Clamp(local.x, -halfExt.x, halfExt.x);
        float cy = Clamp(local.y, -halfExt.y, halfExt.y);
        float cz = Clamp(local.z, -halfExt.z, halfExt.z);

        Vec3 closest = axes[0] * cx + axes[1] * cy + axes[2] * cz + obbPos;

        return (closest - spherePos).LengthSquared() <= radius * radius;
    }


    static inline bool SphereVAABB(const Vec3& spherePos, float radius, const Vec3& boxPos, const Vec3& halfExtents)
    {
        float cx = Clamp(spherePos.x, boxPos.x - halfExtents.x, boxPos.x + halfExtents.x);
        float cy = Clamp(spherePos.y, boxPos.y - halfExtents.y, boxPos.y + halfExtents.y);
        float cz = Clamp(spherePos.z, boxPos.z - halfExtents.z, boxPos.z + halfExtents.z);

        Vec3 closest = Vec3(cx, cy, cz);
        return (spherePos - closest).LengthSquared() <= radius * radius;
    }


    static inline bool SphereVCapsule(const Vec3& sp, float radius, const Vec3& cp, float capRad, float halfH)
    {
        Vec3 a = CapsuleSegmentA(cp, halfH);
        Vec3 b = CapsuleSegmentB(cp, halfH);

        float distSq = DistSqSegmentToPoint(a, b, sp);
        float r = radius + capRad;

        return distSq <= r * r;
    }


    static inline bool OBBvOBB(const Matrix3x3& aAxes, const Vec3& aPos, const Vec3& aExt, const Matrix3x3& bAxes, const Vec3& bPos, const Vec3& bExt)
    {
        // Vector from A to B
        Vec3 T = bPos - aPos;

        // Test A's 3 axes
        for (int i = 0; i < 3; i++)
        {
            // Project T onto axis A_i
            float dist = std::abs(T.Dot(aAxes[i]));

            // Radius of A projected onto its own axis = its extent on that axis
            float ra = aExt[i];

            // Radius of B projected onto A's axis
            float rb =
                std::abs(bExt.x * aAxes[i].Dot(bAxes[0])) +
                std::abs(bExt.y * aAxes[i].Dot(bAxes[1])) +
                std::abs(bExt.z * aAxes[i].Dot(bAxes[2]));

            if (dist > ra + rb)
                return false;
        }
        return true;
    }


    static inline bool AABBvAABB(const Vec3& p1, const Vec3& e1, const Vec3& p2, const Vec3& e2)
    {
        if (fabs(p1.x - p2.x) > (e1.x + e2.x)) return false;
        if (fabs(p1.y - p2.y) > (e1.y + e2.y)) return false;
        if (fabs(p1.z - p2.z) > (e1.z + e2.z)) return false;
        return true;
    }


    static inline bool AABBvCapsule(const Vec3& boxPos, const Vec3& halfExt, const Vec3& capPos, float capRad, float halfH)
    {
        Vec3 a = CapsuleSegmentA(capPos, halfH);
        Vec3 b = CapsuleSegmentB(capPos, halfH);

        float distSq = DistSqSegmentToAABB(a, b, boxPos, halfExt);
        float r = capRad;

        return distSq <= r * r;
    }


    static inline bool AABBvOBB(const Vec3& aPos, const Vec3& aExt, const Vec3& bPos, const Vec3& bExt, const Matrix3x3& bAxes)
    {
        // AABB's axes (world aligned)
        Vec3 aAxes[3] = {
            Vec3(1,0,0),
            Vec3(0,1,0),
            Vec3(0,0,1)
        };

        // Rotation matrix between boxes
        float R[3][3];
        float AbsR[3][3];

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                R[i][j] = aAxes[i].Dot(bAxes[j]);
                AbsR[i][j] = fabs(R[i][j]) + 1e-6f;
            }
        }

        // Translation into AABB basis
        Vec3 t = bPos - aPos;
        float T[3] = {
            t.Dot(aAxes[0]),
            t.Dot(aAxes[1]),
            t.Dot(aAxes[2])
        };

        // Test AABB axes
        for (int i = 0; i < 3; i++)
        {
            float ra = aExt[i];
            float rb = bExt[0] * AbsR[i][0] + bExt[1] * AbsR[i][1] + bExt[2] * AbsR[i][2];
            if (fabs(T[i]) > ra + rb) return false;
        }

        // Test OBB axes
        for (int j = 0; j < 3; j++)
        {
            float ra = aExt[0] * AbsR[0][j] + aExt[1] * AbsR[1][j] + aExt[2] * AbsR[2][j];
            float rb = bExt[j];
            float proj = fabs(T[0] * R[0][j] + T[1] * R[1][j] + T[2] * R[2][j]);
            if (proj > ra + rb) return false;
        }

        // Cross products of axes
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                float ra = aExt[(i + 1) % 3] * AbsR[(i + 2) % 3][j] + aExt[(i + 2) % 3] * AbsR[(i + 1) % 3][j];
                float rb = bExt[(j + 1) % 3] * AbsR[i][(j + 2) % 3] + bExt[(j + 2) % 3] * AbsR[i][(j + 1) % 3];

                float proj =
                    fabs(T[(i + 1) % 3] * R[(i + 2) % 3][j] - T[(i + 2) % 3] * R[(i + 1) % 3][j]);

                if (proj > ra + rb) return false;
            }
        }

        return true; // all axes overlap
    }


    static inline bool CapsuleVCapsule(const Vec3& p1, float r1, float h1, const Vec3& p2, float r2, float h2)
    {
        Vec3 a1 = CapsuleSegmentA(p1, h1);
        Vec3 b1 = CapsuleSegmentB(p1, h1);

        Vec3 a2 = CapsuleSegmentA(p2, h2);
        Vec3 b2 = CapsuleSegmentB(p2, h2);

        float distSq = DistSqSegmentToSegment(a1, b1, a2, b2);
        float r = r1 + r2;

        return distSq <= r * r;
    }


    static inline bool RayVSphere(const Vec3& ro, const Vec3& rd, const Vec3& c, float r, float& t)
    {
        Vec3 oc = ro - c;
        float b = oc.Dot(rd);
        float cval = oc.LengthSquared() - r * r;
        float disc = b * b - cval;
        if (disc < 0) return false;

        t = -b - sqrtf(disc);
        return t >= 0;
    }


    static inline bool RayVAABB(const Vec3& ro, const Vec3& rd, const Vec3& bPos, const Vec3& e, float& t)
    {
        float tmin = (bPos.x - e.x - ro.x) / rd.x;
        float tmax = (bPos.x + e.x - ro.x) / rd.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (bPos.y - e.y - ro.y) / rd.y;
        float tymax = (bPos.y + e.y - ro.y) / rd.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        float tzmin = (bPos.z - e.z - ro.z) / rd.z;
        float tzmax = (bPos.z + e.z - ro.z) / rd.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        t = tmin >= 0 ? tmin : tmax; // pick first intersection in front of ray
        return t >= 0;
    }


    static inline bool RayOBB(const Vec3& rayOrig, const Vec3& rayDir, const Vec3& obbPos, const Vec3& halfExt, const Matrix3x3& axes, float& tOut)
    {
        // Transform to OBB local space
        Vec3 p = rayOrig - obbPos;
        Vec3 localOrig(
            p.Dot(axes[0]),
            p.Dot(axes[1]),
            p.Dot(axes[2])
        );

        Vec3 localDir(
            rayDir.Dot(axes[0]),
            rayDir.Dot(axes[1]),
            rayDir.Dot(axes[2])
        );

        // Ray vs AABB test in local space
        float tMin = 0.0f;
        float tMax = 1e9f;

        for (int i = 0; i < 3; i++)
        {
            float o = localOrig[i];
            float d = localDir[i];
            float minB = -halfExt[i];
            float maxB = halfExt[i];

            if (fabs(d) < 1e-6f)
            {
                if (o < minB || o > maxB) return false;
            }
            else
            {
                float t1 = (minB - o) / d;
                float t2 = (maxB - o) / d;
                if (t1 > t2) std::swap(t1, t2);

                if (t1 > tMin) tMin = t1;
                if (t2 < tMax) tMax = t2;

                if (tMin > tMax) return false;
            }
        }

        tOut = tMin;
        return true;
    }


    static inline bool RayCapsule(const Vec3& rayOrig, const Vec3& rayDir, const Vec3& capPos, float radius, float halfH, float& tOut)
    {
        // Capsule segment endpoints
        Vec3 a = CapsuleSegmentA(capPos, halfH);
        Vec3 b = CapsuleSegmentB(capPos, halfH);
        Vec3 ab = b - a;
        float abLenSq = ab.Dot(ab);

        // Compute the closest point between ray and the capsule axis segment
        Vec3 ao = rayOrig - a;

        float dDotAB = rayDir.Dot(ab);
        float aoDotAB = ao.Dot(ab);

        float A = abLenSq - dDotAB * dDotAB;
        float B = abLenSq * ao.Dot(rayDir) - aoDotAB * dDotAB;
        float C = abLenSq * ao.Dot(ao) - aoDotAB * aoDotAB - radius * radius * abLenSq;

        // Quadratic discriminant
        float disc = B * B - A * C;
        if (disc < 0.0f) return false;

        float sqrtD = sqrtf(disc);
        float t = (-B - sqrtD) / A;
        if (t < 0.0f) return false;

        // Check if closest point lies within cylinder *segment*
        float proj = aoDotAB + t * dDotAB;
        if (proj < 0.0f || proj > abLenSq)
        {
            // Check spherical caps instead
            float tSphere;

            // Hit sphere at A
            Vec3 ao2 = rayOrig - a;
            float B2 = rayDir.Dot(ao2);
            float C2 = ao2.Dot(ao2) - radius * radius;
            float disc2 = B2 * B2 - C2;
            if (disc2 >= 0.0f)
            {
                tSphere = -B2 - sqrtf(disc2);
                if (tSphere >= 0.0f)
                {
                    tOut = tSphere;
                    return true;
                }
            }

            // Hit sphere at B
            Vec3 bo2 = rayOrig - b;
            float B3 = rayDir.Dot(bo2);
            float C3 = bo2.Dot(bo2) - radius * radius;
            float disc3 = B3 * B3 - C3;
            if (disc3 >= 0.0f)
            {
                tSphere = -B3 - sqrtf(disc3);
                if (tSphere >= 0.0f)
                {
                    tOut = tSphere;
                    return true;
                }
            }

            return false;
        }

        tOut = t;
        return true;
    }
private:

    bool TestSphere(const Collider* o)  const
    {
        Vec3 p1 = Transform->GetWorldPosition() + _sphereParams.Offset;

        switch (o->type)
        {
        case ColliderType::Sphere:
            return SphereVSphere(
                p1, _sphereParams.radius,
                o->Transform->GetWorldPosition() + o->_sphereParams.Offset,
                o->_sphereParams.radius
            );

        case ColliderType::AABB:
            return SphereVAABB(
                p1, _sphereParams.radius,
                o->Transform->GetWorldPosition() + o->_AABBParams.offset,
                o->_AABBParams.halfExtents
            );

        case ColliderType::OBB:
            return SphereVOBB(
                p1, _sphereParams.radius,
                o->Transform->GetWorldPosition() + o->_OBBParams.offset,
                o->_OBBParams.halfExtents,
                o->_OBBParams.orientation
            );

        case ColliderType::Capsule:
            return SphereVCapsule(
                p1, _sphereParams.radius,
                o->Transform->GetWorldPosition() + o->_capsuleParams.offset,
                o->_capsuleParams.radius,
                o->_capsuleParams.halfHeight
            );
        }
        return false;
    }


    bool TestAABB(const Collider* o) const
    {
        Vec3 p1 = Transform->GetWorldPosition() + _AABBParams.offset;

        switch (o->type)
        {
        case ColliderType::Sphere:
            return SphereVAABB(
                o->Transform->GetWorldPosition() + o->_sphereParams.Offset,
                o->_sphereParams.radius,
                p1,
                _AABBParams.halfExtents
            );

        case ColliderType::AABB:
            return AABBvAABB(
                p1, _AABBParams.halfExtents,
                o->Transform->GetWorldPosition() + o->_AABBParams.offset,
                o->_AABBParams.halfExtents
            );

        case ColliderType::OBB:
            return AABBvOBB(
                p1, _AABBParams.halfExtents,
                o->Transform->GetWorldPosition() + o->_OBBParams.offset,
                o->_OBBParams.halfExtents,
                o->_OBBParams.orientation
            );

        case ColliderType::Capsule:
            return AABBvCapsule(
                p1, _AABBParams.halfExtents,
                o->Transform->GetWorldPosition() + o->_capsuleParams.offset,
                o->_capsuleParams.radius,
                o->_capsuleParams.halfHeight
            );
        }

        return false;
    }


    bool TestOBB(const Collider* o)  const
    {
        Vec3 p1 = this->Transform->GetWorldPosition() + _OBBParams.offset;

        switch (o->type)
        {
        case ColliderType::Sphere:
            return SphereVOBB(
                o->Transform->GetWorldPosition() + o->_sphereParams.Offset,
                o->_sphereParams.radius,
                p1,
                _OBBParams.halfExtents,
                _OBBParams.orientation
            );

        case ColliderType::AABB:
            return AABBvOBB(
                o->Transform->GetWorldPosition() + o->_AABBParams.offset,
                o->_AABBParams.halfExtents,
                p1,
                _OBBParams.halfExtents,
                _OBBParams.orientation
            );

        case ColliderType::OBB:
            return OBBvOBB(
                _OBBParams.orientation,
                p1,
                _OBBParams.halfExtents,
                o->_OBBParams.orientation,
                o->Transform->GetWorldPosition() + o->_OBBParams.offset,
                o->_OBBParams.halfExtents
            );

        case ColliderType::Capsule:
            return OBBvCapsule(
                p1, _OBBParams.halfExtents, _OBBParams.orientation,
                o->Transform->GetWorldPosition() + o->_capsuleParams.offset,
                o->_capsuleParams.radius,
                o->_capsuleParams.halfHeight
            );
        }

        return false;
    }


    bool TestCapsule(const Collider* o)  const
    {
        Vec3 p1 = Transform->GetWorldPosition() + _capsuleParams.offset;

        switch (o->type)
        {
        case ColliderType::Sphere:
            return SphereVCapsule(
                o->Transform->GetWorldPosition() + o->_sphereParams.Offset,
                o->_sphereParams.radius,
                p1,
                _capsuleParams.radius,
                _capsuleParams.halfHeight
            );

        case ColliderType::AABB:
            return AABBvCapsule(
                o->Transform->GetWorldPosition() + o->_AABBParams.offset,
                o->_AABBParams.halfExtents,
                p1,
                _capsuleParams.radius,
                _capsuleParams.halfHeight
            );

        case ColliderType::OBB:
            return OBBvCapsule(
                o->Transform->GetWorldPosition() + o->_OBBParams.offset,
                o->_OBBParams.halfExtents,
                o->_OBBParams.orientation,
                p1,
                _capsuleParams.radius,
                _capsuleParams.halfHeight
            );

        case ColliderType::Capsule:
            return CapsuleVCapsule(
                p1,
                _capsuleParams.radius,
                _capsuleParams.halfHeight,
                o->Transform->GetWorldPosition() + o->_capsuleParams.offset,
                o->_capsuleParams.radius,
                o->_capsuleParams.halfHeight
            );
        }

        return false;
    }
};