#pragma once

#ifndef MATH_RIGHT_HANDED
#ifndef MATH_LEFT_HANDED
#define MATH_LEFT_HANDED
#endif
#endif

#ifndef PixelRound
#define PixelRound(Val) (int)(Val + 0.5f)
#endif

#ifndef PixelRoundFloor
#define PixelRoundFloor(Val) (int)(Val)
#endif

#ifndef PixelRoundMinMax
#define PixelRoundMinMax(Val, Minimum, Maximum) std::max(Minimum, std::min(PixelRoundFloor(Val), Maximum))
#endif

#define ContIdx(x, y, Width) (SIZE_T)(x + (Width * y))

#include "VectorMath.h"
#include "MatrixMath.h"

struct Ray
{
	Vec3 origin;
	Vec3 direction;
	Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.Normalized()) {}
};


Vec3 Vec3::operator * (const Matrix& m) const
{
	// For row-Vector, row-major, left handed storage:
 
	//x' = x*m00 + y*m10 + z*m20 + w*m30;
	//y' = x*m01 + y*m11 + z*m21 + w*m31;
	//z' = x*m02 + y*m12 + z*m22 + w*m32;

	return Vec3(
		x * m._11 + y * m._21 + z * m._31 + m._41,
		x * m._12 + y * m._22 + z * m._32 + m._42,
		x * m._13 + y * m._23 + z * m._33 + m._43
	);
}


void Vec3::operator *= (const Matrix& b)
{
	float Tmpx = this->x;
	float Tmpy = this->y;
	float Tmpz = this->z;

	this->x = Tmpx * b._11 + Tmpy * b._21 + Tmpz * b._31 + b._41;
	this->y = Tmpx * b._12 + Tmpy * b._22 + Tmpz * b._32 + b._42;
	this->z = Tmpx * b._13 + Tmpy * b._23 + Tmpz * b._33 + b._43;
}


Vec4 Vec4::operator * (const Matrix& b) const
{
	return Vec4(
		x * b._11 + y * b._21 + z * b._31 + w * b._41,
		x * b._12 + y * b._22 + z * b._32 + w * b._42,
		x * b._13 + y * b._23 + z * b._33 + w * b._43,
		x * b._14 + y * b._24 + z * b._34 + w * b._44
	);
}


Vec3 Vec3::operator * (const Matrix3x3& m) const
{
	return Vec3(
		x * m._11 + y * m._21 + z * m._31,
		x * m._12 + y * m._22 + z * m._32,
		x * m._13 + y * m._23 + z * m._33
	);
}


void Vec4::operator *= (const Matrix& b)
{
	float _tmpx = this->x;
	float _tmpy = this->y;
	float _tmpz = this->z;
	float _tmpw = this->w;


	this->x = _tmpx * b._11 + _tmpy * b._21 + _tmpz * b._31 + _tmpw * b._41;
	this->y = _tmpx * b._12 + _tmpy * b._22 + _tmpz * b._32 + _tmpw * b._42;
	this->z = _tmpx * b._13 + _tmpy * b._23 + _tmpz * b._33 + _tmpw * b._43;
	this->w = _tmpx * b._14 + _tmpy * b._24 + _tmpz * b._34 + _tmpw * b._44;
}


Matrix Matrix::CreateScalarMatrix(const Vec3& Scalar)
{
	Matrix Out;

	Out._matrix[0][0] = Scalar.x;
	Out._matrix[1][1] = Scalar.y;
	Out._matrix[2][2] = Scalar.z;
	Out._matrix[3][3] = 1.f;

	return Out;
}


Matrix Matrix::CreateTranslationMatrix(const Vec3& Translation)
{
	Matrix Out = Matrix::CreateIdentity();
	Out.SetTranslation(Translation);
	return Out;
}


Matrix Matrix::CreateRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
{
	Matrix Rx; // Pitch
	Matrix Ry; // Yaw
	Matrix Rz; // Roll
	Matrix::CreateRotationX(&Rx, RotationDeg.x);      // pitch
	Matrix::CreateRotationY(&Ry, RotationDeg.y);      // yaw
	Matrix::CreateRotationZ(&Rz, RotationDeg.z);      // roll

	Matrix R = Rx * Ry * Rz;

	return R;
}


//__inline Matrix Matrix::CreateMatrixFromRigthForwardUp(const Vec3& Right, const Vec3& Up, const Vec3& Forward, const Vec3& Trans)
//{
//	Matrix rotMatrix = Matrix::CreateIdentity();
//
//	// row-major: row0 = right, row1 = up, row2 = forward
//	rotMatrix._11 = Right.x;  rotMatrix._12 = Right.y;  rotMatrix._13 = Right.z;  rotMatrix._14 = 0.0f;
//	rotMatrix._21 = Up.x;     rotMatrix._22 = Up.y;     rotMatrix._23 = Up.z;     rotMatrix._24 = 0.0f;
//	rotMatrix._31 = Forward.x; rotMatrix._32 = Forward.y; rotMatrix._33 = Forward.z; rotMatrix._34 = 0.0f;
//
//	// translation row
//	rotMatrix._41 = Trans.x; rotMatrix._42 = Trans.y; rotMatrix._43 = Trans.z; rotMatrix._44 = 1.0f;
//
//	return rotMatrix;
//}


//void Matrix::CalcScalarMatrix(const Vec3& Scalar)
//{
//	this->MakeIdentity();
//	this->_matrix[0][0] = 1.f * Scalar.x;
//	this->_matrix[1][1] = 1.f * Scalar.y;
//	this->_matrix[2][2] = 1.f * Scalar.z;
//	this->_matrix[3][3] = 1.f;
//}
//
//
//void Matrix::CalcTranslationMatrix(const Vec3& Translation)
//{
//	this->MakeIdentity();
//	this->_matrix[3][0] = Translation.x;
//	this->_matrix[3][1] = Translation.y;
//	this->_matrix[3][2] = Translation.z;
//}
//


//void Matrix::CalcRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
//{
//	this->MakeIdentity();
//	float RotRadsZ = RotationDeg.z;
//
//	Matrix A;
//	Matrix B;
//	Matrix C;
//	Matrix::CreateRotationX(&A, ToRad(RotationDeg.x));
//	Matrix::CreateRotationY(&B, ToRad(RotationDeg.y));
//	Matrix::CreateRotationZ(&C, ToRad(RotRadsZ));
//
//	A = A * B * C;
//
//	//return A;
//}


//void __fastcall Matrix::MakeOrthoMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& Near, const float& Far)
//{
//	this->_matrix[0][0] = 2.0f / (Right - Left);
//	this->_matrix[1][1] = 2.0f / (Top - Bottom);
//	this->_matrix[2][2] = -2.0f / (Far - Near);
//	this->_matrix[3][3] = 1.0f;
//
//	this->_matrix[3][0] = -(Right + Left) / (Right - Left);
//	this->_matrix[3][1] = -(Top + Bottom) / (Top - Bottom);
//	this->_matrix[3][2] = -(Far + Near) / (Far - Near);
//
//	// Zero out the rest of the matrix
//	this->_matrix[0][1] = this->_matrix[0][2] = this->_matrix[0][3] = 0.0f;
//	this->_matrix[1][0] = this->_matrix[1][2] = this->_matrix[1][3] = 0.0f;
//	this->_matrix[2][0] = this->_matrix[2][1] = this->_matrix[2][3] = 0.0f;
//}
//
//
//void __fastcall Matrix::MakeViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up)
//{
//	this->MakeIdentity();
//#if LH_COORDINATE_SYSTEM
//	Vec3 forward = (Target - Pos).Normalized(); // +Z forward
//#else
//	Vec3 forward = (Pos - Target).Normalized(); // -Z forward
//#endif
//
//	Vec3 right = Up.Cross(forward).Normalized(); // cross order chosen so right points +X
//	Vec3 realUp = forward.Cross(right); // completes orthonormal triad
//
//	// fill matrix with rows (row-major, row-vector)
//	this->_11 = right.x;  this->_12 = right.y;  this->_13 = right.z; this->_14 = 0.0f;
//	this->_21 = realUp.x; this->_22 = realUp.y; this->_23 = realUp.z; this->_24 = 0.0f;
//	this->_31 = forward.x; this->_32 = forward.y; this->_33 = forward.z; this->_34 = 0.0f;
//
//	this->_41 = Pos.x;    this->_42 = Pos.y;    this->_43 = Pos.z;    this->_44 = 1.0f;
//}


Matrix Matrix::ConstructViewMatrix(const Vec3& Right, const Vec3& Up, const Vec3& Forward, const Vec3& Origin)
{
	Matrix View = Matrix::CreateIdentity();
	//Matrix::CreateTranslationMatrix(-EyePos);
	
	// Rotation part (inverse rotation = transpose for orthogonal)
	View._11 = Right.x;   View._21 = Up.x;   View._31 = Forward.x;
	View._12 = Right.y;   View._22 = Up.y;   View._32 = Forward.y;
	View._13 = Right.z;   View._23 = Up.z;   View._33 = Forward.z;

	View.Transpose3x3();

	View._41 = -Right.Dot(Origin);
	View._42 = -Up.Dot(Origin);
	View._43 = -Forward.Dot(Origin);
	View._44 = 1.0f;

	return View;
}


Matrix Matrix::CalcLookAtMatrix(const Vec3& Origin, const Vec3& Forward, const Vec3& Up)
{
	Vec3 F = Forward.Normalized();
	Vec3 R = Up.Cross(Forward).Normalized();
	Vec3 U = Forward.Cross(R);

	return Matrix::ConstructViewMatrix(R, U, F, Origin);
}


Matrix Matrix::CalcViewMatrix(const Vec3& Origin, const Vec3& TargetPos, const Vec3& Up)
{
	// LEFT HANDED
	Vec3 forward = (TargetPos - Origin).Normalized();

	return 	Matrix::CalcLookAtMatrix(Origin, forward, Up);
}


Matrix Matrix::CalcInverseView(const Vec3& Up)
{
	Vec3 Forward = this->GetForward().Normalized();
	Vec3 Right = Up.Cross(Forward).Normalized();
	Vec3 RealUp = Forward.Cross(Right).Normalized();

	Matrix Out;
	Out.SetRight(Right);
	Out.SetUp(RealUp);
	Out.SetForward(Forward);

	Out.SetTranslation(this->GetTranslation());

	return Out.QuickInversed();
}


Matrix Matrix::CalcOrthoMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& Near, const float& Far)
{
	Matrix OrthoMat;

	OrthoMat._matrix[0][0] = 2.0f / (Right - Left);
	OrthoMat._matrix[1][1] = 2.0f / (Top - Bottom);
	OrthoMat._matrix[2][2] = 1.0f / (Far - Near);
	OrthoMat._matrix[3][3] = 1.0f;

	OrthoMat._matrix[3][0] = -(Right + Left) / (Right - Left);
	OrthoMat._matrix[3][1] = -(Top + Bottom) / (Top - Bottom);
	OrthoMat._matrix[3][2] = -Near / (Far - Near);

	// Zero out the rest of the matrix
	OrthoMat._matrix[0][1] = OrthoMat._matrix[0][2] = OrthoMat._matrix[0][3] = 0.0f;
	OrthoMat._matrix[1][0] = OrthoMat._matrix[1][2] = OrthoMat._matrix[1][3] = 0.0f;
	OrthoMat._matrix[2][0] = OrthoMat._matrix[2][1] = OrthoMat._matrix[2][3] = 0.0f;

	return OrthoMat;
}







/*__inline Matrix Matrix::InverseSRT() const
{
	Matrix out;

	// Extract scale
	float sx = Vec3(_matrix[0][0], _matrix[0][1], _matrix[0][2]).Magnitude();
	float sy = Vec3(_matrix[1][0], _matrix[1][1], _matrix[1][2]).Magnitude();
	float sz = Vec3(_matrix[2][0], _matrix[2][1], _matrix[2][2]).Magnitude();

	// Rotation = normalize basis
	Vec3 X = Vec3(_matrix[0][0], _matrix[0][1], _matrix[0][2]) / sx;
	Vec3 Y = Vec3(_matrix[1][0], _matrix[1][1], _matrix[1][2]) / sy;
	Vec3 Z = Vec3(_matrix[2][0], _matrix[2][1], _matrix[2][2]) / sz;

	// Build rotation inverse (transpose)
	out._matrix[0][0] = X.x; out._matrix[0][1] = Y.x; out._matrix[0][2] = Z.x;
	out._matrix[1][0] = X.y; out._matrix[1][1] = Y.y; out._matrix[1][2] = Z.y;
	out._matrix[2][0] = X.z; out._matrix[2][1] = Y.z; out._matrix[2][2] = Z.z;

	// Apply inverse scale
	out._matrix[0][0] /= sx;
	out._matrix[1][1] /= sy;
	out._matrix[2][2] /= sz;

	// Invert translation
	Vec3 t = this->GetTranslation();
	Vec3 invT = -(t);

	// Compute transformed translation
	Vec3 newT;
	newT.x = invT.Dot(Vec3(out._matrix[0][0], out._matrix[1][0], out._matrix[2][0]));
	newT.y = invT.Dot(Vec3(out._matrix[0][1], out._matrix[1][1], out._matrix[2][1]));
	newT.z = invT.Dot(Vec3(out._matrix[0][2], out._matrix[1][2], out._matrix[2][2]));

	out._matrix[3][0] = newT.x;
	out._matrix[3][1] = newT.y;
	out._matrix[3][2] = newT.z;
	out._matrix[3][3] = 1.0f;

	return out;
}
*/





/*
__inline void Matrix::Decompose(Vec3& outScale, Vec3& outEuler, Vec3& outPos) const
{
	// 1. Extract translation (T)
	outPos = this->GetTranslation();

	// 2. Remove translation
	Matrix M = this->GetBasis();

	// 3. Extract scale from the basis vectors (since M = S*R)
	Vec3 X(M._matrix[0][0], M._matrix[0][1], M._matrix[0][2]);
	Vec3 Y(M._matrix[1][0], M._matrix[1][1], M._matrix[1][2]);
	Vec3 Z(M._matrix[2][0], M._matrix[2][1], M._matrix[2][2]);

	outScale = Vec3(
		X.Magnitude(),
		Y.Magnitude(),
		Z.Magnitude()
	);

	// 4. Normalize basis vectors to get the pure rotation matrix R
	if (outScale.x != 0) X = X / outScale.x;
	if (outScale.y != 0) Y = Y / outScale.y;
	if (outScale.z != 0) Z = Z / outScale.z;

	Matrix R = Matrix::CreateIdentity();
	R._matrix[0][0] = X.x; R._matrix[0][1] = X.y; R._matrix[0][2] = X.z;
	R._matrix[1][0] = Y.x; R._matrix[1][1] = Y.y; R._matrix[1][2] = Y.z;
	R._matrix[2][0] = Z.x; R._matrix[2][1] = Z.y; R._matrix[2][2] = Z.z;

	// 5. Convert R to Euler
	outEuler = R.ExtractEuler(); // You must implement this
}
*/

#define COLOR_NORMAL 0
#define COLOR_255 1

enum class ColorModes
{
	RGB = 0,
	NormalizedRGB = 1,
	sRGB = 2
};

class Color
{
public:
	Color()
	{

	}

	Color(const float R, const float G, const float B, const float A = 255.0f, const ColorModes Mode = ColorModes::RGB)
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;

		if (Mode != ColorModes::RGB && A > 1.0f)
		{
			this->A = this->A / 255.0f;
		}
	}

	static Color RainbowColor(int step)
	{
		float h = fmodf(step * 0.05f, 1.0f);
		float r, g, b;

		float i = floorf(h * 6.0f);
		float f = h * 6.0f - i;
		float q = 1.0f - f;

		switch ((int)i % 6)
		{
			case 0: r = 1; g = f; b = 0; break;
			case 1: r = q; g = 1; b = 0; break;
			case 2: r = 0; g = 1; b = f; break;
			case 3: r = 0; g = q; b = 1; break;
			case 4: r = f; g = 0; b = 1; break;
			case 5: r = 1; g = 0; b = q; break;
		}

		return Color((int)(r * 255), (int)(g * 255), (int)(b * 255));
	}

	static __inline float LinearToSRGB_Channel(float c)
	{
		c = std::clamp(c, 0.0f, 1.0f);

		if (c <= 0.0031308f)
			return 12.92f * c;
		else
			return 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
	}

	static inline float SRGBToLinear_Channel(float c)
	{
		c = std::clamp(c, 0.0f, 1.0f);

		if (c <= 0.04045f)
			return c / 12.92f;
		else
			return powf((c + 0.055f) / 1.055f, 2.4f);
	}


	float CalculateLuminance(ColorModes Mode = ColorModes::RGB)
	{
		if (Mode == ColorModes::RGB)
		{
			this->R /= 255.0f;
			this->G /= 255.0f;
			this->B /= 255.0f;
		}

		return 0.2126f * this->R + 0.7152f * this->G + 0.0722f * this->B;
	}

	Color(const Vec3 RGB, const float A = 255.0f, const int Mode = COLOR_255)
	{
		this->R = RGB.x;
		this->G = RGB.y;
		this->B = RGB.z;
		this->A = A;

		if (Mode != COLOR_255 && A > 1.0f)
		{
			this->A = this->A / 255.0f;
		}
	}

	void Normalize()
	{
		this->R = this->R / 255.0f;
		this->G = this->G / 255.0f;
		this->B = this->B / 255.0f;
		this->A = this->A / 255.0f;
	}

	Color Normalized()
	{
		return Color(this->R / 255.0f, this->G / 255.0f, this->B / 255.0f);
	}


	Vec3 GetRGB() const
	{
		return Vec3(R * (A / 255.0f), G * (A / 255.0f), B * (A / 255.0f));
	}

	void Denormalize()
	{
		this->R = this->R * 255.0f;
		this->G = this->G * 255.0f;
		this->B = this->B * 255.0f;
		this->A = this->A * 255.0f;
	}

	Color Denormalized()
	{
		return Color(this->R * 255.0f, this->G * 255.0f, this->B * 255.0f);
	}

	void TosRGB()
	{
		this->R = LinearToSRGB_Channel(R);
		this->G = LinearToSRGB_Channel(G);
		this->B = LinearToSRGB_Channel(B);
	}

	void ToLinear()
	{
		this->R = SRGBToLinear_Channel(R);
		this->G = SRGBToLinear_Channel(G);
		this->B = SRGBToLinear_Channel(B);
	}

	float R = 255.0f;
	float G = 255.0f;
	float B = 255.0f;
	float A = 255.0f;
};

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif 