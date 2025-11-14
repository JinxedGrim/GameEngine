#pragma once
#include "Math.h"
#include "Collider.h"
#include "Mesh.h"


struct Ray
{
	Vec3 origin;
	Vec3 direction; // MUST be normalized

	Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.Normalized()) {}
};


struct RaycastHit
{
	bool hit = false;
	float distance = 0.0f;
	Vec3 point;
	Vec3 normal;
	const Collider* hitCollider = nullptr;
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
	Vec3 min = center - halfExtents;
	Vec3 max = center + halfExtents;

	float tmin = (min.x - ray.origin.x) / ray.direction.x;
	float tmax = (max.x - ray.origin.x) / ray.direction.x;
	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (min.y - ray.origin.y) / ray.direction.y;
	float tymax = (max.y - ray.origin.y) / ray.direction.y;
	if (tymin > tymax) std::swap(tymin, tymax);

	if (tmin > tymax || tymin > tmax)
		return false;

	if (tymin > tmin) tmin = tymin;
	if (tymax < tmax) tmax = tymax;

	float tzmin = (min.z - ray.origin.z) / ray.direction.z;
	float tzmax = (max.z - ray.origin.z) / ray.direction.z;
	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if (tmin > tzmax || tzmin > tmax)
		return false;

	float t = tmin;
	if (t < 0) t = tmax;
	if (t < 0) return false;

	out->hit = true;
	out->distance = t;
	out->point = ray.origin + ray.direction * t;

	// normal
	Vec3 localPoint = out->point - center;
	Vec3 absPoint = localPoint.GetAbs();
	if (absPoint.x > absPoint.y && absPoint.x > absPoint.z)
		out->normal = Vec3(localPoint.x > 0 ? 1 : -1, 0, 0);
	else if (absPoint.y > absPoint.x && absPoint.y > absPoint.z)
		out->normal = Vec3(0, localPoint.y > 0 ? 1 : -1, 0);
	else
		out->normal = Vec3(0, 0, localPoint.z > 0 ? 1 : -1);

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


bool RaycastMesh(const Ray& ray, const std::vector<Triangle>& triangles, RaycastHit* outHit)
{
	bool foundHit = false;
	float closest = FLT_MAX;

	RaycastHit temp;

	for (const Triangle& tri : triangles)
	{
		if (RayIntersectsTriangle(ray, tri.Points[0], tri.Points[1], tri.Points[2], &temp))
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