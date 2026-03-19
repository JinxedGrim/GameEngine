#pragma once
#include "Math.h"
#include "Mesh.h"

struct RaycastHit
{
	bool hit = false;
	float distance = 0.0f;
	Vec3 point;
	Vec3 normal;
};


bool RayIntersectsSphere(const Ray& ray, const Vec3& center, float radius, RaycastHit* Out)
{
	Vec3 oc = ray.origin - center;
	float a = ray.direction.Dot(ray.direction);         // should be 1 if normalized
	float b = 2.0f * oc.Dot(ray.direction);
	float c = oc.Dot(oc) - radius * radius;

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0f)
		return false;

	float sqrtD = sqrtf(discriminant);

	float t0 = (-b - sqrtD) / (2 * a);
	float t1 = (-b + sqrtD) / (2 * a);

	float t = (t0 > 0.0f) ? t0 : t1;
	if (t <= 0.0f)
		return false;

	Out->hit = true;
	Out->distance = t;
	Out->point = ray.origin + ray.direction * t;
	Out->normal = (Out->point - center).Normalized();
	return true;
}


bool RayIntersectsAABB(const Ray& ray, const Vec3& center, const Vec3& halfExtents, RaycastHit* out)
{
	const float EPSILON = 1e-8f;

	Vec3 min = center - halfExtents;
	Vec3 max = center + halfExtents;

	float tmin = -FLT_MAX;
	float tmax = FLT_MAX;

	Vec3 hitNormal = Vec3(0, 0, 0);

	// ----- X slab -----
	if (fabs(ray.direction.x) < EPSILON)
	{
		// Ray parallel to X planes
		if (ray.origin.x < min.x || ray.origin.x > max.x)
			return false;
	}
	else
	{
		float invD = 1.0f / ray.direction.x;
		float t1 = (min.x - ray.origin.x) * invD;
		float t2 = (max.x - ray.origin.x) * invD;

		Vec3 n = Vec3(-1, 0, 0);

		if (t1 > t2)
		{
			std::swap(t1, t2);
			n = Vec3(1, 0, 0);
		}

		if (t1 > tmin)
		{
			tmin = t1;
			hitNormal = n;
		}

		tmax = std::min(tmax, t2);
		if (tmin > tmax)
			return false;
	}

	// ----- Y slab -----
	if (fabs(ray.direction.y) < EPSILON)
	{
		if (ray.origin.y < min.y || ray.origin.y > max.y)
			return false;
	}
	else
	{
		float invD = 1.0f / ray.direction.y;
		float t1 = (min.y - ray.origin.y) * invD;
		float t2 = (max.y - ray.origin.y) * invD;

		Vec3 n = Vec3(0, -1, 0);

		if (t1 > t2)
		{
			std::swap(t1, t2);
			n = Vec3(0, 1, 0);
		}

		if (t1 > tmin)
		{
			tmin = t1;
			hitNormal = n;
		}

		tmax = std::min(tmax, t2);
		if (tmin > tmax)
			return false;
	}

	// ----- Z slab -----
	if (fabs(ray.direction.z) < EPSILON)
	{
		if (ray.origin.z < min.z || ray.origin.z > max.z)
			return false;
	}
	else
	{
		float invD = 1.0f / ray.direction.z;
		float t1 = (min.z - ray.origin.z) * invD;
		float t2 = (max.z - ray.origin.z) * invD;

		Vec3 n = Vec3(0, 0, -1);

		if (t1 > t2)
		{
			std::swap(t1, t2);
			n = Vec3(0, 0, 1);
		}

		if (t1 > tmin)
		{
			tmin = t1;
			hitNormal = n;
		}

		tmax = std::min(tmax, t2);
		if (tmin > tmax)
			return false;
	}

	float t = tmin;
	if (t < 0.0f)
		t = tmax;

	if (t < 0.0f)
		return false;

	out->hit = true;
	out->distance = t;
	out->point = ray.origin + ray.direction * t;
	out->normal = hitNormal;

	return true;
}


bool RayIntersectsOBB(const Ray& ray, const Vec3& center, const Vec3& halfExtents, const Matrix3x3& orientation, RaycastHit* out)
{
	// Transform ray into OBB local space
	Vec3 p = ray.origin - center;
	Vec3 localOrigin = orientation.Transposed() * p;
	Vec3 localDir = orientation.Transposed() * ray.direction;

	// Now do standard local-space AABB intersection
	Ray localRay{ localOrigin, localDir };

	RaycastHit localHit;
	if (!RayIntersectsAABB(localRay, Vec3(0, 0, 0), halfExtents, &localHit))
		return false;

	// Convert hit back to world space
	out->hit = true;
	out->distance = localHit.distance;
	out->point = ray.origin + ray.direction * out->distance;
	out->normal = orientation * localHit.normal;

	return true;
}


bool RayIntersectsTriangle(const Ray& ray, const Vec3& v0, const Vec3& v1, const Vec3& v2, RaycastHit* Out)
{
	const float EPS = 1e-6f;

	Vec3 edge1 = v1 - v0;
	Vec3 edge2 = v2 - v0;

	Vec3 h = ray.direction.Cross(edge2);
	float a = edge1.Dot(h);

	if (fabs(a) < EPS)
		return false;

	float f = 1.0f / a;
	Vec3 s = ray.origin - v0;

	float u = f * s.Dot(h);
	if (u < 0.0f || u > 1.0f)
		return false;

	Vec3 q = s.Cross(edge1);
	float v = f * ray.direction.Dot(q);
	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = f * edge2.Dot(q);
	if (t <= EPS)
		return false;

	Out->hit = true;
	Out->distance = t;
	Out->point = ray.origin + ray.direction * t;
	Out->normal = edge1.Cross(edge2).Normalized();
	return true;
}


bool RayIntersectsCapsule(const Ray& ray, const Vec3& p0, const Vec3& p1, float radius, RaycastHit* out)
{
	const float EPS = 1e-6f;

	Vec3 d = ray.direction;
	Vec3 m = ray.origin - p0;
	Vec3 n = p1 - p0;

	float md = m.Dot(d);
	float nd = n.Dot(d);
	float mn = m.Dot(n);
	float nn = n.Dot(n);

	float a = nn - nd * nd;
	float k = m.Dot(m) - radius * radius;
	float c = nn * k - mn * mn;

	// Ray outside or parallel
	if (fabs(a) < EPS)
	{
		if (c > 0) return false;
	}
	else if (c > 0)
	{
		float b = nn * md - nd * mn;
		float discr = b * b - a * c;
		if (discr < 0) return false;
	}

	// Solve for t along the ray
	float t = (-md + sqrtf(md * md - k)) / d.Dot(d);
	if (t < 0) return false;

	out->hit = true;
	out->distance = t;
	out->point = ray.origin + d * t;

	// normal
	Vec3 proj = p0 + n * ((out->point - p0).Dot(n) / nn);
	out->normal = (out->point - proj).Normalized();

	return true;
}


// this overload is meant only to be used for a WorldRay && local set of triangles
// it will first transform the ray into the objects space then it will it preform the ray cast
bool RaycastMesh(Ray ray, const std::vector<Triangle>& triangles, RaycastHit* outHit, const Matrix* ObjectWorld)
{
	bool foundHit = false;
	float closest = FLT_MAX;

	Matrix InverseWorld = ObjectWorld->Inversed();
	Matrix3x3 normalMatrix = ObjectWorld->GetBasis3x3();
	Matrix3x3 InvNormal = normalMatrix.Inversed();

	RaycastHit temp;
	Vec3 worldRayOrigin = ray.origin;

	ray.origin = Vec4(ray.origin, 1.0f) * InverseWorld;
	ray.direction = (ray.direction * InvNormal).Normalized();

	for (const Triangle& tri : triangles)
	{
		Vec3 v0 = tri.Points.Points[0];
		Vec3 v1 = tri.Points.Points[1];
		Vec3 v2 = tri.Points.Points[2];

		if (RayIntersectsTriangle(ray, v0, v1, v2, &temp))
		{
			if (temp.distance < closest)
			{
				closest = temp.distance;
				*outHit = temp;
				foundHit = true;
			}
		}
	}

	if (!foundHit)
		return false;

	outHit->normal = (outHit->normal * normalMatrix).Normalized();
	outHit->point = outHit->point * *ObjectWorld;
	outHit->distance = worldRayOrigin.Distance(outHit->point);

	return foundHit;
}


bool RaycastMesh(const Ray& ray, const std::vector<Triangle>& triangles, RaycastHit* outHit)
{
	bool foundHit = false;
	float closest = FLT_MAX;

	RaycastHit temp;

	for (const Triangle& tri : triangles)
	{
		Vec3 v0 = tri.Points.Points[0];
		Vec3 v1 = tri.Points.Points[1];
		Vec3 v2 = tri.Points.Points[2];
		if (RayIntersectsTriangle(ray, v0, v1, v2, &temp))
		{
			if (temp.distance < closest)
			{
				closest = temp.distance;
				*outHit = temp;
				foundHit = true;
			}
		}
	}

	return foundHit;
}