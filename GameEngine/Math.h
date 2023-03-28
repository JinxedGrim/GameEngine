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

		Out->_11 = 1;
		Out->_12 = 0;
		Out->_13 = 0;
		Out->_14 = 0;
		Out->_21 = 0;
		Out->_22 = cosf(AngleRads);
		Out->_23 = sinf(AngleRads);
		Out->_24 = 0;
		Out->_31 = 0;
		Out->_32 = -sinf(AngleRads);
		Out->_33 = cosf(AngleRads);
		Out->_34 = 0;
		Out->_41 = 0;
		Out->_42 = 0;
		Out->_43 = 0;
		Out->_44 = 1;
	}

	static void CreateRotationY(Matrix* Out, float AngleRads)
	{
		Out->_11 = cosf(AngleRads);
		Out->_12 = 0;
		Out->_13 = sinf(AngleRads);
		Out->_14 = 0;
		Out->_21 = 0;
		Out->_22 = 1;
		Out->_23 = 0;
		Out->_24 = 0;
		Out->_31 = -sin(AngleRads);
		Out->_32 = 0;
		Out->_33 = cos(AngleRads);
		Out->_34 = 0;
		Out->_41 = 0;
		Out->_42 = 0;
		Out->_43 = 0;
		Out->_44 = 1;
	}

	static void CreateRotationZ(Matrix* Out, float AngleRads)
	{
		Out->_11 = cosf(AngleRads);
		Out->_12 = sinf(AngleRads);
		Out->_13 = 0;
		Out->_14 = 0;
		Out->_21 = -sinf(AngleRads);
		Out->_22 = cosf(AngleRads);
		Out->_23 = 0;
		Out->_24 = 0;
		Out->_31 = 0;
		Out->_32 = 0;
		Out->_33 = 1;
		Out->_34 = 0;
		Out->_41 = 0;
		Out->_42 = 0;
		Out->_43 = 0;
		Out->_44 = 1;
	}

	static void CreateTranslationMatrix(Matrix* Out, float x, float y, float z)
	{
		Out->fMatrix[0][0] = 1.0f;
		Out->fMatrix[1][1] = 1.0f;
		Out->fMatrix[2][2] = 1.0f;
		Out->fMatrix[3][3] = 1.0f;
		Out->fMatrix[3][0] = x;
		Out->fMatrix[3][1] = y;
		Out->fMatrix[3][2] = z;
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

	static Matrix CreateRotationMatrix(float RotationXRads, float RotationYRads, float RotationZRads)
	{
		if (RotationZRads == 0.0f)
		{
			RotationZRads = ToRad(180);
		}

		Matrix A;
		Matrix B;
		Matrix C;
		CreateRotationX(&A, RotationXRads);
		CreateRotationY(&B, RotationYRads);
		CreateRotationZ(&C, RotationZRads);

		A = A * B * C;

		return A;
	}

	Matrix operator*(const Matrix b) const
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
	Vec3 Clamped(float MinPitch = -89, float MaxPitch = 89, float MinYaw = -180, float MaxYaw = 180)
	{
		Vec3 NewAngles;
		NewAngles.x = Clamp(MinPitch, MaxPitch, this->x);
		NewAngles.y = Clamp(MinYaw, MaxYaw, this->y);
		NewAngles.z = 0;

		return NewAngles;
	}

	// Distance from one vec3 to another
	float Distance(Vec3 b)
	{
		return (float)sqrt(pow(b.x - this->x, 2) + pow(b.y - this->y, 2) + pow(b.z - this->z, 2));
	}

	// Magnitude / Length of the vector3
	float Magnitude()
	{
		return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	// Returns the unit vector aka a vector of magnitude 1 with a persisting direction
	Vec3 Normalized()
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
	float Dot(Vec3 Rhs) 
	{
		return { this->x * Rhs.x + this->y * Rhs.y + this->z * Rhs.z };
	}

	// Calculates Dot Product Then Normalizes The Answer
	float DotNormalized(Vec3 Rhs)
	{
		Vec3 Lhs = this->Normalized();
		Rhs = Rhs.Normalized();
	}

	// Calculates Cross Product
	void Cross(const Vec3* Rhs, Vec3* Out)
	{
		*Out = Vec3((this->y * Rhs->z - this->z * Rhs->y), (this->z * Rhs->x - this->x * Rhs->z), (this->x * Rhs->y - this->y * Rhs->x));
	}

	Vec3 Cross(const Vec3* Rhs)
	{
		return Vec3((this->y * Rhs->z - this->z * Rhs->y), (this->z * Rhs->x - this->x * Rhs->z), (this->x * Rhs->y - this->y * Rhs->x));
	}

	Vec3 Cross(const Vec3 Rhs)
	{
		return Vec3((this->y * Rhs.z - this->z * Rhs.y), (this->z * Rhs.x - this->x * Rhs.z), (this->x * Rhs.y - this->y * Rhs.x));
	}

	// Calculates Cross Product Then Normalizes Result
	void CrossNormalized(Vec3* Rhs, Vec3* Out)
	{
		this->Cross(Rhs, Out);
		Out->Normalize();
	}

	Vec3 CrossNormalized(Vec3* Rhs)
	{
		Vec3 Out = this->Cross(Rhs);
		Out.Normalize();
		return Out;
	}

	Vec3 CrossNormalized(Vec3 Rhs)
	{
		Vec3 Out = this->Cross(Rhs);
		Out.Normalize();
		return Out;
	}

	// Angle to a point
	float Angle(const Vec3 To)
	{
		return (float)ToDegree(acos(Clamp(-1.f, 1.f, this->DotNormalized(To))));
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

	Vec3 operator + (const Vec3 b) const
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

	Vec3 operator - (const Vec3 b) const
	{
		return
		{
			this->x - b.x,
			this->y - b.y,
			this->z - b.z,
		};
	}

	Vec3 operator * (const float b) const
	{
		return
		{
			this->x * b,
			this->y * b,
			this->z * b,
		};
	}

	Vec3 operator * (const Vec3 b) const
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

	Vec3 operator * (const Matrix b) const
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

	Vec3 operator / (const Vec3 b) const
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

	void operator += (const Vec3 b)
	{
		this->x += b.x;
		this->y += b.y;
		this->z += b.z;
	}

	void operator -= (const Vec3 b)
	{
		this->x -= b.x;
		this->y -= b.y;
		this->z -= b.z;
	}

	void operator *= (const Vec3 b)
	{
		this->x *= b.x;
		this->y *= b.y;
		this->z *= b.z;
	}
	
	void operator *= (const Matrix b)
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
		//else
		//{
		//	return Out;
		//}

		//return Out;
	}

	bool operator != (const Vec3 b)
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
	float w = 0;
	friend std::ostream& operator<<(std::ostream& os, const Vec4& v);
};

std::ostream& operator << (std::ostream& os, const Vec4& v)
{
	os << "x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w;
	return os;
}
