#pragma once
#include <math.h>
#include <vector>
#include <Windows.h>

#define PI 3.14159265358979323846

#define ToDegree(Deg) (float)((Deg) * (float)(180.0f / PI))
#define ToRad(Rad) (float)((Rad) * (float)(PI / 180.0f))
#define ToDegreeD(Deg) (double)((Deg) * (180.0 / PI))
#define ToRadD(Rad) (double)((Rad) * (PI / 180.0))

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

class Vec3;

class Matrix
{
public:

	Matrix()
	{
		memset(this->fMatrix, 0, sizeof(this->fMatrix));
	}

	Matrix(float Matrix[4][4])
	{
		int i, j;

		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				this->fMatrix[i][j] = Matrix[i][j];
			}
		}
	}

	static void CreateRotationX(Matrix* Out, float AngleRads)
	{
		Out->MakeIdentity();
		Out->_22 = cosf(AngleRads);
		Out->_23 = sinf(AngleRads);
		Out->_32 = -sinf(AngleRads);
		Out->_33 = cosf(AngleRads);
	}

	static void CreateRotationY(Matrix* Out, float AngleRads)
	{
		Out->MakeIdentity();
		Out->_11 = cosf(AngleRads);
		Out->_13 = sinf(AngleRads);
		Out->_31 = -sinf(AngleRads);
		Out->_33 = cosf(AngleRads);
	}

	static void CreateRotationZ(Matrix* Out, float AngleRads)
	{
		Out->MakeIdentity();
		Out->_11 = cosf(AngleRads);
		Out->_12 = sinf(AngleRads);
		Out->_21 = -sinf(AngleRads);
		Out->_22 = cosf(AngleRads);
	}

	static Matrix CreateIdentity()
	{
		Matrix Out;
		Out.fMatrix[0][0] = 1.f;
		Out.fMatrix[1][1] = 1.f;
		Out.fMatrix[2][2] = 1.f;
		Out.fMatrix[3][3] = 1.f;

		return Out;
	}

	void MakeIdentity()
	{
		Matrix Out;
		Out.fMatrix[0][0] = 1.f;
		Out.fMatrix[1][1] = 1.f;
		Out.fMatrix[2][2] = 1.f;
		Out.fMatrix[3][3] = 1.f;

		*this = Out;
	}

	static Matrix CreateScalarMatrix(const Vec3&);

	static Matrix CreateRotationMatrix(const Vec3&);

	static Matrix CreateTranslationMatrix(const Vec3&);

	void CalcScalarMatrix(const Vec3&);

	void CalcRotationMatrix(const Vec3&);

	void CalcTranslationMatrix(const Vec3&);

	Matrix operator*(const Matrix& b) const
	{
		Matrix result;

		for (int row = 0; row < 4; row++) 
		{
			for (int col = 0; col < 4; col++) 
			{
				float sum = 0;

				for (int i = 0; i < 4; i++) 
				{
					sum += fMatrix[row][i] * b.fMatrix[i][col];			
				}
				result.fMatrix[row][col] = sum;
			}
		}

		return result;
	}

	void operator *= (const float& b)
	{
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				this->fMatrix[row][col] = this->fMatrix[row][col] * b;
			}
		}
	}

	void operator *= (const Matrix& b)
	{
		*this = *this * b;
	}

	Matrix QuickInversed()
	{
		Matrix OutMat;

		// Assign values to the output matrix manually
		OutMat.fMatrix[0][0] = this->fMatrix[0][0];
		OutMat.fMatrix[0][1] = this->fMatrix[1][0];
		OutMat.fMatrix[0][2] = this->fMatrix[2][0];
		OutMat.fMatrix[0][3] = 0.0f;

		OutMat.fMatrix[1][0] = this->fMatrix[0][1];
		OutMat.fMatrix[1][1] = this->fMatrix[1][1];
		OutMat.fMatrix[1][2] = this->fMatrix[2][1];
		OutMat.fMatrix[1][3] = 0.0f;

		OutMat.fMatrix[2][0] = this->fMatrix[0][2];
		OutMat.fMatrix[2][1] = this->fMatrix[1][2];
		OutMat.fMatrix[2][2] = this->fMatrix[2][2];
		OutMat.fMatrix[2][3] = 0.0f;

		// Calculate the last column of the output matrix manually
		float t0 = -(this->fMatrix[3][0]);
		float t1 = -(this->fMatrix[3][1]);
		float t2 = -(this->fMatrix[3][2]);

		OutMat.fMatrix[3][0] = t0 * OutMat.fMatrix[0][0] + t1 * OutMat.fMatrix[1][0] + t2 * OutMat.fMatrix[2][0];
		OutMat.fMatrix[3][1] = t0 * OutMat.fMatrix[0][1] + t1 * OutMat.fMatrix[1][1] + t2 * OutMat.fMatrix[2][1];
		OutMat.fMatrix[3][2] = t0 * OutMat.fMatrix[0][2] + t1 * OutMat.fMatrix[1][2] + t2 * OutMat.fMatrix[2][2];
		OutMat.fMatrix[3][3] = 1.0f;

		return OutMat;
	}

public:
	union
	{
		struct
		{
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float fMatrix[4][4];
	};
};

std::ostream& operator << (std::ostream& os, const Matrix& v)
{
	int i, j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			os << v.fMatrix[i][j];
			os << " ";
		}
		os << "\n";
	}

	return os;
}

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
	Vec2 operator - (const Vec2 b)
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

	// Clamps an angle
	Vec3 AngleNormalized()
	{
		if (this->x < -89.f)
			this->x = -89.f;
		if (this->x > 89.f)
			this->x = 89.f;
		if (this->y < -180.f) // maybe -180
			this->y += 360.f;
		if (this->y > 180.f) // maybe 180
			this->y -= 360.f;
		return { this->x, this->y, this->z };
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
	float Distance(Vec3 &b) const
	{
		return (float)sqrt(pow(b.x - this->x, 2) + pow(b.y - this->y, 2) + pow(b.z - this->z, 2));
	}

	// Magnitude / Length of the vector3
	float Magnitude() const
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

	void Normalize()
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

	Vec3 CalculateIntersectionPoint(const Vec3& LineEnd, const Vec3 &PointOnPlane, const Vec3 &PlaneNormalized) const
	{
		Vec3 LineStart = *this;
		float Dist = -PlaneNormalized.Dot(PointOnPlane);
		float ad = LineStart.Dot(PlaneNormalized);
		float bd = LineEnd.Dot(PlaneNormalized);
		float t = (-Dist - ad) / (bd - ad);
		Vec3 LineStartToEnd = LineEnd - LineStart;
		Vec3 LineToIntersect = LineStartToEnd * t;
		return LineStart + LineToIntersect;
	}

	// Angle to a point
	float Angle(const Vec3 To) const
	{
		return (float)ToDegree(acos(Clamp(-1.f, 1.f, this->Dot(To))));
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

	Vec3 operator * (const Matrix& b) const
	{
		Vec3 Out;

		Out.x = this->x * b.fMatrix[0][0] + this->y * b.fMatrix[1][0] + this->z * b.fMatrix[2][0] + b.fMatrix[3][0];
		Out.y = this->x * b.fMatrix[0][1] + this->y * b.fMatrix[1][1] + this->z * b.fMatrix[2][1] + b.fMatrix[3][1];
		Out.z = this->x * b.fMatrix[0][2] + this->y * b.fMatrix[1][2] + this->z * b.fMatrix[2][2] + b.fMatrix[3][2];
		float w = this->x * b.fMatrix[0][3] + this->y * b.fMatrix[1][3] + this->z * b.fMatrix[2][3] + b.fMatrix[3][3];

		if (w != 0)
		{
			Out.x /= w;
			Out.y /= w;
			Out.z /= w;
		}
		else
		{
			return Out;
		}

		return Out;
	}

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
	
	void operator *= (const Matrix& b)
	{
		float Tmpx = this->x;
		float Tmpy = this->y;
		float Tmpz = this->z;

		this->x = Tmpx * b.fMatrix[0][0] + Tmpy * b.fMatrix[1][0] + Tmpz * b.fMatrix[2][0] + b.fMatrix[3][0];
		this->y = Tmpx * b.fMatrix[0][1] + Tmpy * b.fMatrix[1][1] + Tmpz * b.fMatrix[2][1] + b.fMatrix[3][1];
		this->z = Tmpx * b.fMatrix[0][2] + Tmpy * b.fMatrix[1][2] + Tmpz * b.fMatrix[2][2] + b.fMatrix[3][2];
		float w = Tmpx * b.fMatrix[0][3] + Tmpy * b.fMatrix[1][3] + Tmpz * b.fMatrix[2][3] + b.fMatrix[3][3];

		if (w != 0)
		{
			this->x /= w;
			this->y /= w;
			this->z /= w;
		}
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
		if (this->x != b.x)
		{
			return false;
		}
		else if (this->y != b.y)
		{
			return false;
		}
		else if (this->z != b.z)
		{
			return false;
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Vec3& v);
public:
	float x;
	float y;
	float z;
};

Matrix Matrix::CreateScalarMatrix(const Vec3& Scalar)
{
	Matrix Out;

	Out.fMatrix[0][0] = 1.f * Scalar.x;
	Out.fMatrix[1][1] = 1.f * Scalar.y;
	Out.fMatrix[2][2] = 1.f * Scalar.z;
	Out.fMatrix[3][3] = 1.f;

	return Out;
}

Matrix Matrix::CreateTranslationMatrix(const Vec3& Translation)
{
	Matrix Out;
	Out.fMatrix[0][0] = 1.0f;
	Out.fMatrix[1][1] = 1.0f;
	Out.fMatrix[2][2] = 1.0f;
	Out.fMatrix[3][3] = 1.0f;
	Out.fMatrix[3][0] = Translation.x;
	Out.fMatrix[3][1] = Translation.y;
	Out.fMatrix[3][2] = Translation.z;
	return Out;
}

Matrix Matrix::CreateRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
{
	float RotRadsZ = RotationDeg.z;
	if (RotRadsZ == 0.0f)
	{
		RotRadsZ = 180;
	}

	Matrix A;
	Matrix B;
	Matrix C;
	Matrix::CreateRotationX(&A, -ToRad(RotationDeg.x));
	Matrix::CreateRotationY(&B, ToRad(RotationDeg.y));
	Matrix::CreateRotationZ(&C, ToRad(RotRadsZ));

	A = A * B * C;

	return A;
}

void Matrix::CalcScalarMatrix(const Vec3& Scalar)
{
	this->fMatrix[0][0] = 1.f * Scalar.x;
	this->fMatrix[1][1] = 1.f * Scalar.y;
	this->fMatrix[2][2] = 1.f * Scalar.z;
	this->fMatrix[3][3] = 1.f;
}

void Matrix::CalcTranslationMatrix(const Vec3& Translation)
{
	Matrix Out;
	this->fMatrix[0][0] = 1.0f;
	this->fMatrix[1][1] = 1.0f;
	this->fMatrix[2][2] = 1.0f;
	this->fMatrix[3][3] = 1.0f;
	this->fMatrix[3][0] = Translation.x;
	this->fMatrix[3][1] = Translation.y;
	this->fMatrix[3][2] = Translation.z;
}

void Matrix::CalcRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
{
	float RotRadsZ = RotationDeg.z;
	if (RotRadsZ == 0.0f)
	{
		RotRadsZ = 180;
	}

	Matrix A;
	Matrix B;
	Matrix C;
	Matrix::CreateRotationX(&A, -ToRad(RotationDeg.x));
	Matrix::CreateRotationY(&B, ToRad(RotationDeg.y));
	Matrix::CreateRotationZ(&C, ToRad(RotRadsZ));

	A = A * B * C;

	//return A;
}


std::ostream& operator << (std::ostream& os, const Vec3& v)
{
	os << "x: " << v.x << " y: " << v.y << " z: " << v.z;
	return os;
}

class Vec4
{
public:
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
	friend std::ostream& operator<<(std::ostream& os, const Vec4& v);
};

std::ostream& operator << (std::ostream& os, const Vec4& v)
{
	os << "x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w;
	return os;
}
