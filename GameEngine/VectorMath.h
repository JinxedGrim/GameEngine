#pragma once
#include <ostream>

constexpr float PI = 3.14159265358979323846f;

constexpr float ToDegree(float r) { return r * (180.0f / PI); }
constexpr float ToRad(float d) { return d * (PI / 180.0f); }

#ifndef ToDegreeD
#define ToDegreeD(Deg) (double)((Deg) * (180.0 / PI))
#endif

#ifndef ToRadD
#define ToRadD(Rad) (double)((Rad) * (PI / 180.0))
#endif

// TODO Probably replace with std::clamp
float Clamp(float Min, float Max, float Val)
{
	if (Val < Min)
	{
		Val = Min;
	}
	if (Val > Max)
	{
		Val = Max;
	}
	return Val;
}

class Vec4;
class Matrix;


class Vec2
{
public:
	Vec2(float _x, float _y)
	{
		this->x = _x;
		this->y = _y;
	}
	Vec2()
	{
		this->x = 0.0f;
		this->y = 0.0f;
	}
	float Distance(Vec2 b)
	{
		return (float)sqrt(pow(b.x - this->x, 2) + pow(b.y - this->y, 2));
	}
	Vec2 operator - (const Vec2& b) const
	{
		return
		{
			this->x - b.x,
			this->y - b.y,
		};
	}
	float x = 0;
	float y = 0;
};


class Vec3
{
public:
	Vec3()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
	}


	Vec3(float _x, float _y, float _z)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
	}


	Vec4 MakeVec4();


	// Clamps an angle
	Vec3 AngleNormalized()
	{
		Vec3 a = *this;
		a.x = Clamp(-89.0f, 89.0f, a.x);

		// wraps y between -180 and 180
		a.y = fmodf(a.y + 180.0f, 360.0f) - 180.0f;

		return a;
	}


	// clamps angles to keep them in between 2 values
	Vec3 Clamped(float MinPitch = -89, float MaxPitch = 89, float MinYaw = -180, float MaxYaw = 180) const
	{
		Vec3 NewAngles;
		NewAngles.x = Clamp(MinPitch, MaxPitch, this->x);
		NewAngles.y = Clamp(MinYaw, MaxYaw, this->y);
		NewAngles.z = 0;

		return NewAngles;
	}


	// Distance from one vec3 to another
	float Distance(Vec3& b) const
	{
		float dx = b.x - x;
		float dy = b.y - y;
		float dz = b.z - z;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}


	// Magnitude / Length of the vector3
	float __inline Magnitude() const
	{
		return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}


	// Returns the unit vector aka a vector of magnitude 1 with a persisting direction
	Vec3 Normalized() const
	{
		float Mag = this->Magnitude();

		if (Mag != 0)
		{
			return { this->x / Mag, this->y / Mag, this->z / Mag };
		}
		return Vec3();
	}


	__inline Vec3 GetAbs() const
	{
		return Vec3(fabsf(this->x), fabsf(this->y), fabsf(this->z));
	}


	__inline void Abs()
	{
		this->x = fabsf(this->x);
		this->y = fabsf(this->y);
		this->z = fabsf(this->z);
	}


	// assumes an abs val vec3
	__inline int GetDominantAxis() const
	{
		float ax = std::fabs(x);
		float ay = std::fabs(y);
		float az = std::fabs(z);

		if (ax >= ay && ax >= az) return 0;
		if (ay >= ax && ay >= az) return 1;
		return 2;
	}


	Vec3 __inline GetSignedCardinalDirection(const Vec3 B) const
	{
		Vec3 Tmp = this->GetDirectionToVector(B);

		int idx = Tmp.GetAbs().GetDominantAxis();

		Vec3 Out = Vec3(0, 0, 0);
		Out.data[idx] = Tmp.data[idx];

		return Out;
	}


	void __inline __fastcall Normalize()
	{
		float Mag = this->Magnitude();

		if (Mag != 0)
		{
			this->x /= Mag;
			this->y /= Mag;
			this->z /= Mag;
		}
	}


	// Calculates Dot Product (How similar two vecs are)
	const float Dot(const Vec3& Rhs) const
	{
		return { this->x * Rhs.x + this->y * Rhs.y + this->z * Rhs.z };
	}


	float __inline LengthSquared()
	{
		return this->Dot(*this);
	}


	// Calculates Cross Product
	void Cross(const Vec3* Rhs, Vec3* Out) const
	{
		*Out = Vec3((this->y * Rhs->z - this->z * Rhs->y), (this->z * Rhs->x - this->x * Rhs->z), (this->x * Rhs->y - this->y * Rhs->x));
	}


	Vec3 Cross(const Vec3* Rhs) const
	{
		return Vec3((this->y * Rhs->z - this->z * Rhs->y), (this->z * Rhs->x - this->x * Rhs->z), (this->x * Rhs->y - this->y * Rhs->x));
	}


	Vec3 Cross(const Vec3& Rhs) const
	{
		return Vec3((this->y * Rhs.z - this->z * Rhs.y), (this->z * Rhs.x - this->x * Rhs.z), (this->x * Rhs.y - this->y * Rhs.x));
	}


	// Calculates Cross Product Then Normalizes Result
	void CrossNormalized(Vec3* Rhs, Vec3* Out)
	{
		this->Cross(Rhs, Out);
		Out->Normalize();
	}


	Vec3 CrossNormalized(const Vec3& Rhs) const
	{
		Vec3 Out = this->Cross(Rhs);
		Out.Normalize();
		return Out;
	}


	Vec3 CalculateIntersectionPoint(const Vec3& LineEnd, const Vec3& PointOnPlane, const Vec3& PlaneNormalized, float* OutT = nullptr) const
	{
		Vec3 LineStart = *this;
		float Dist = -PlaneNormalized.Dot(PointOnPlane);
		float ad = LineStart.Dot(PlaneNormalized);
		float bd = LineEnd.Dot(PlaneNormalized);
		float t = (-Dist - ad) / (bd - ad);
		Vec3 LineStartToEnd = LineEnd - LineStart;
		Vec3 LineToIntersect = LineStartToEnd * t;

		if (OutT != nullptr)
			*OutT = t;

		return LineStart + LineToIntersect;
	}


	// Angle to a point
	float Angle(const Vec3 To) const
	{
		return (float)ToDegree(acos(Clamp(-1.f, 1.f, this->Dot(To))));
	}


	const Vec3 GetReflectection(const Vec3& SurfaceNormal) const
	{
		return *this - (SurfaceNormal * (2.0f * this->Dot(SurfaceNormal)));
	}


	const void Reflected(const Vec3& SurfaceNormal)
	{
		*this = *this - (SurfaceNormal * (2.0f * this->Dot(SurfaceNormal)));
	}


	// returns euler angles needed to look at a point specified by B
	Vec3 CalcAngle(const Vec3 b, const bool Degree = true)
	{
		// this will only work on right up forward games
		Vec3 Angles;

		Vec3 Dir = Vec3(this->x - b.x, this->y - b.y, this->z - b.z); // Gets the vector between the two points

		float Pitch = 0.f;
		float Yaw = 0.f;

		if (Degree)
		{
			Pitch = (float)(ToDegree(asin(Dir.y / Dir.Magnitude()))); // calculates pitch by doing the inverse sin of y / mag and converts from rad to deg (magnitude is the hypotenuse) this will be y
			Yaw = (float)(ToDegree(-atan2(Dir.x, -Dir.z))); // calculates yaw by doing the inverse tan of x / z and converts from rad to deg this will be x
		}

		Angles.x = Pitch; // some games store them in different orders be prepared to switch this
		Angles.y = Yaw; // some games store them in different orders be prepared to switch this
		Angles.z = 0.0f; // roll is 0 in fps games

		Angles.Clamped(); // clamping angles 

		return Angles;
	}


	Vec3 LerpedTo(const Vec3& B, float t)
	{
		return Vec3(this->x + (B.x - this->x) * t, this->y + (B.y - this->y) * t, this->z + (B.z - this->z) * t);
	}


	static Vec3 Lerp(const Vec3& A, const Vec3& B, float t)
	{
		return Vec3(A.x + (B.x - A.x) * t, A.y + (B.y - A.y) * t, A.z + (B.z - A.z) * t);
	}


	void Lerped(const Vec3& B, float t)
	{
		this->x = this->x + (B.x - this->x) * t;
		this->y = this->y + (B.y - this->y) * t;
		this->z = this->z + (B.z - this->z) * t;
	}


	Vec3 __inline __fastcall GetDirectionToVector(const Vec3 b) const
	{
		return (*this - b).Normalized();
	}


	// operators
	Vec3 operator + (const Vec3& b) const
	{
		return
		{
			this->x + b.x,
			this->y + b.y,
			this->z + b.z,
		};
	}


	Vec3 operator + (const float b) const
	{
		return
		{
			this->x + b,
			this->y + b,
			this->z + b,
		};
	}


	Vec3 operator - (const Vec3& b) const
	{
		return
		{
			this->x - b.x,
			this->y - b.y,
			this->z - b.z,
		};
	}


	Vec3 operator * (const float& b) const
	{
		return
		{
			this->x * b,
			this->y * b,
			this->z * b,
		};
	}


	Vec3 operator * (const double& b) const
	{
		return
		{
			this->x * (float)b,
			this->y * (float)b,
			this->z * (float)b,
		};
	}


	Vec3 operator * (const Vec3& b) const
	{
		return
		{
			this->x * b.x,
			this->y * b.y,
			this->z * b.z,
		};
	}


	Vec3 operator * (const int b) const
	{
		return
		{
			this->x * b,
			this->y * b,
			this->z * b,
		};
	}


	Vec3 operator * (const Matrix& b) const;
	void operator *= (const Matrix& b);

	Vec3 operator / (const Vec3& b) const
	{
		return
		{
			this->x / b.x,
			this->y / b.y,
			this->z / b.z,
		};
	}


	Vec3 operator / (const int b) const
	{
		return
		{
			this->x / b,
			this->y / b,
			this->z / b,
		};
	}


	Vec3 operator / (const float& b) const
	{
		return
		{
			this->x / b,
			this->y / b,
			this->z / b,
		};
	}


	void operator += (const Vec3& b)
	{
		this->x += b.x;
		this->y += b.y;
		this->z += b.z;
	}


	void operator -= (const Vec3& b)
	{
		this->x -= b.x;
		this->y -= b.y;
		this->z -= b.z;
	}


	void operator *= (const Vec3& b)
	{
		this->x *= b.x;
		this->y *= b.y;
		this->z *= b.z;
	}


	void operator *= (const float& b)
	{
		this->x *= b;
		this->y *= b;
		this->z *= b;
	}

	Vec3 operator-()
	{
		return
		{
			-this->x,
			-this->y,
			-this->z
		};
	}


	bool operator != (const Vec3& b)
	{
		return x != b.x || y != b.y || z != b.z;
	}

	bool operator == (const Vec3& b) const
	{
		return x == b.x && y == b.y && z == b.z;
	}

	float operator[](int i) const
	{
		return this->data[i];
	}

	friend std::ostream& operator<<(std::ostream& os, const Vec3& v);

public:
	union {
		struct {
			float x, y, z;
		};
		float data[3];
	};
};


class Vec4
{
public:

	Vec4()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		this->z = 0.0f;
		this->w = 1.0f;
	}

	Vec4(float _x, float _y, float _z, float _w)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->w = _w;
	}

	Vec4(const Vec3& _Vec, float _w)
	{
		this->x = _Vec.x;
		this->y = _Vec.y;
		this->z = _Vec.z;
		this->w = _w;
	}

	__inline Vec3 xyz() const
	{
		return Vec3(this->x, this->y, this->z);
	}

	__inline Vec3 GetVec3() const
	{
		return Vec3(this->x, this->y, this->z);
	}

	void __inline CorrectPerspective()
	{
		if (this->w != 0)
		{
			float invW = 1.0f / this->w;
			this->x *= invW;
			this->y *= invW;
			this->z *= invW;
		}
	}

	Vec4 CalculateIntersectionPoint(const Vec4& LineEnd, const Vec3& PointOnPlane, const Vec3& PlaneNormal, float* OutT = nullptr) const
	{
		const Vec4& LineStart = *this;

		// Distance from plane origin
		float Dist = -PlaneNormal.Dot(PointOnPlane);

		// Signed distances of each endpoint
		float ad = PlaneNormal.Dot(LineStart.GetVec3());
		float bd = PlaneNormal.Dot(LineEnd.GetVec3());

		// Solve intersection parameter
		float t = (-Dist - ad) / (bd - ad);

		if (OutT) *OutT = t;

		// Interpolate FULL 4D vector (x,y,z,w)
		return LineStart + (LineEnd - LineStart) * t;
	}

	Vec4 operator * (const Matrix& b) const;

	void operator *= (const Matrix& b);

	Vec4& operator=(const Vec3& b)
	{
		x = b.x;
		y = b.y;
		z = b.z;
		w = 1.0f;
		return *this;
	}

	Vec4 operator-(const Vec3& b) const
	{
		return
		{
			this->x - b.x,
			this->y - b.y,
			this->z - b.z,
			this->w
		};
	}

	Vec4 operator+(const Vec3& b) const
	{
		return
		{
			this->x + b.x,
			this->y + b.y,
			this->z + b.z,
			this->w
		};
	}

	void operator+=(const Vec3& b)
	{
		this->x += b.x;
		this->y += b.y;
		this->z += b.z;
	}

	Vec4 operator*(const Vec3& b) const
	{
		return
		{
			this->x * b.x,
			this->y * b.y,
			this->z * b.z,
			this->w
		};
	}

	Vec4 operator*(const float& b) const
	{
		return
		{
			this->x * b,
			this->y * b,
			this->z * b,
			this->w
		};
	}

	Vec4 operator/(const float& b) const
	{
		return
		{
			this->x / b,
			this->y / b,
			this->z / b,
			this->w
		};
	}

	Vec4 operator-(const Vec4& b) const
	{
		return
		{
			this->x - b.x,
			this->y - b.y,
			this->z - b.z,
			this->w
		};
	}

	Vec4 operator+(const Vec4& b) const
	{
		return
		{
			this->x + b.x,
			this->y + b.y,
			this->z + b.z,
			this->w
		};
	}

	void operator*=(const Vec3& b)
	{
		this->x *= b.x;
		this->y *= b.y;
		this->z *= b.z;
	}

	operator Vec3() const
	{
		return Vec3(x, y, z);
	}

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	friend std::ostream& operator<<(std::ostream& os, const Vec4& v);
};


Vec4 Vec3::MakeVec4()
{
	return Vec4(this->x, this->y, this->z, 1.0f);
}


std::ostream& operator << (std::ostream& os, const Vec4& v)
{
	os << "x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w;
	return os;
}
std::ostream& operator << (std::ostream& os, const Vec3& v)
{
	os << "x: " << v.x << " y: " << v.y << " z: " << v.z;
	return os;
}


Vec3 CalculateBarycentricCoordinates(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& P)
{
	// Calculate vectors from vertex A to points B and C
	Vec3 AB = B - A;
	Vec3 AC = C - A;

	// Calculate vectors from vertex A to point P
	Vec3 AP = P - A;

	// Calculate dot products and cross products
	float dotABAB = AB.Dot(AB);
	float dotABAC = AB.Dot(AC);
	float dotAPAB = AP.Dot(AB);
	float dotAPAC = AP.Dot(AC);

	// Calculate barycentric coordinates
	float invDenom = 1.0f / (dotABAB * dotABAC - dotABAC * dotABAC);
	float u = (dotABAB * dotAPAC - dotABAC * dotAPAB) * invDenom;
	float v = (dotABAC * dotAPAC - dotABAC * dotAPAB) * invDenom;
	float w = 1.0f - u - v;

	return Vec3(u, v, w);
}


Vec3 CalculateBarycentricCoordinatesScreenSpace(const Vec2& PixelCoord, const Vec2& Vert0PixelCoord, const Vec2& Vert1PixelCoord, const Vec2& Vert2PixelCoord)
{
	// Calculate the vectors from vertex0 to the fragment and the other two vertices.
	Vec2 v0f = PixelCoord - Vert0PixelCoord;
	Vec2 v1f = PixelCoord - Vert1PixelCoord;
	Vec2 v2f = PixelCoord - Vert2PixelCoord;

	// Calculate the areas of the full triangle and the sub-triangles formed by the fragment.
	float triangleArea = 0.5f * ((Vert1PixelCoord.x - Vert0PixelCoord.x) * (Vert2PixelCoord.y - Vert0PixelCoord.y) -
		(Vert2PixelCoord.x - Vert0PixelCoord.x) * (Vert1PixelCoord.y - Vert0PixelCoord.y));

	float alpha = 0.5f * ((v1f.x * v2f.y - v2f.x * v1f.y) / triangleArea);
	float beta = 0.5f * ((v2f.x * v0f.y - v0f.x * v2f.y) / triangleArea);
	float gamma = 0.5f * ((v0f.x * v1f.y - v1f.x * v0f.y) / triangleArea);

	return Vec3(alpha, beta, gamma);
}


__inline Vec3 BarycentricPerspectiveCorrection(Vec3 bc, float w0, float w1, float w2)
{
	// perspective-correct them
	float invW0 = 1.0f / w0;
	float invW1 = 1.0f / w1;
	float invW2 = 1.0f / w2;

	float denom = bc.x * invW0 + bc.y * invW1 + bc.z * invW2;
	bc.x = (bc.x * invW0) / denom;
	bc.y = (bc.y * invW1) / denom;
	bc.z = (bc.z * invW2) / denom;

	return bc;
}


__inline Vec4 BarycentricInterpolation(const Vec3& BaryCoords, const Vec3& A, const Vec3& B, const Vec3& C, float ClipSapceW)
{
	return Vec4((A * BaryCoords.x) + (B * BaryCoords.y) + (C * BaryCoords.z), 1.0f);
}


Vec3 CalculateFaceNormal(const Vec3& p1, const Vec3& p2, const Vec3& p3) {
	// Calculate two vectors in the plane of the face
	Vec3 v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
	Vec3 v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);

	// Calculate the cross product of the two vectors to get the normal
	Vec3 normal(
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
	);

	// Normalize the normal
	float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	if (length != 0.0f) {
		normal.x /= length;
		normal.y /= length;
		normal.z /= length;
	}

	return normal;
}
