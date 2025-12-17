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


//__inline Vec3 Matrix::ToEulerAnglesXYZ() const
//{
//	float r00 = fMatrix[0][0], r01 = fMatrix[0][1], r02 = fMatrix[0][2];
//	float r10 = fMatrix[1][0], r11 = fMatrix[1][1], r12 = fMatrix[1][2];
//	float r20 = fMatrix[2][0], r21 = fMatrix[2][1], r22 = fMatrix[2][2];
//
//	float pitch, yaw, roll;
//
//	// ---- yaw extraction (from forward.x and forward.z) ----
//	yaw = std::atan2(r20, r22);
//
//	// ---- pitch extraction ----
//	// pitch = asin(-forward.y)
//	float cy = std::sqrt(r20 * r20 + r22 * r22);
//	pitch = std::atan2(-r21, cy);
//
//	// ---- roll extraction ----
//	if (cy > 1e-6f)     // not in gimbal lock
//	{
//		roll = std::atan2(r01, r00);
//	}
//	else
//	{
//		// gimbal lock: pitch is +-90
//		roll = std::atan2(-r12, r11);
//	}
//
//	return Vec3(
//		ToDegree(pitch),
//		ToDegree(yaw),
//		ToDegree(roll)
//	);
//}


Matrix Matrix::CreateScalarMatrix(const Vec3& Scalar)
{
	Matrix Out;

	Out.fMatrix[0][0] = Scalar.x;
	Out.fMatrix[1][1] = Scalar.y;
	Out.fMatrix[2][2] = Scalar.z;
	Out.fMatrix[3][3] = 1.f;

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

	Matrix R = Rz * Ry * Rx;

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
//	this->fMatrix[0][0] = 1.f * Scalar.x;
//	this->fMatrix[1][1] = 1.f * Scalar.y;
//	this->fMatrix[2][2] = 1.f * Scalar.z;
//	this->fMatrix[3][3] = 1.f;
//}
//
//
//void Matrix::CalcTranslationMatrix(const Vec3& Translation)
//{
//	this->MakeIdentity();
//	this->fMatrix[3][0] = Translation.x;
//	this->fMatrix[3][1] = Translation.y;
//	this->fMatrix[3][2] = Translation.z;
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
//	this->fMatrix[0][0] = 2.0f / (Right - Left);
//	this->fMatrix[1][1] = 2.0f / (Top - Bottom);
//	this->fMatrix[2][2] = -2.0f / (Far - Near);
//	this->fMatrix[3][3] = 1.0f;
//
//	this->fMatrix[3][0] = -(Right + Left) / (Right - Left);
//	this->fMatrix[3][1] = -(Top + Bottom) / (Top - Bottom);
//	this->fMatrix[3][2] = -(Far + Near) / (Far - Near);
//
//	// Zero out the rest of the matrix
//	this->fMatrix[0][1] = this->fMatrix[0][2] = this->fMatrix[0][3] = 0.0f;
//	this->fMatrix[1][0] = this->fMatrix[1][2] = this->fMatrix[1][3] = 0.0f;
//	this->fMatrix[2][0] = this->fMatrix[2][1] = this->fMatrix[2][3] = 0.0f;
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


Matrix Matrix::ConstructViewMatrix(const Vec3& Right, const Vec3& Up, const Vec3& Forward, const Vec3& EyePos)
{
	Matrix View = Matrix::CreateIdentity();
	//Matrix::CreateTranslationMatrix(-EyePos);
	
	// Rotation part (inverse rotation = transpose for orthogonal)
	View._11 = Right.x;   View._12 = Up.x;   View._13 = Forward.x;
	View._21 = Right.y;   View._22 = Up.y;   View._23 = Forward.y;
	View._31 = Right.z;   View._32 = Up.z;   View._33 = Forward.z;

	View._14 = -Right.Dot(EyePos);
	View._24 = -Up.Dot(EyePos);
	View._34 = -Forward.Dot(EyePos);
	View._44 = 1.0f;

	return View;
}


Matrix Matrix::CalcLookAtMatrix(const Vec3& EyePos, const Vec3& Dir, const Vec3& Up)
{
	Vec3 Forward = Dir.Normalized();
	Vec3 Right = Up.Cross(Forward).Normalized();
	Vec3 UpReal = Forward.Cross(Right);

	return Matrix::ConstructViewMatrix(Right, UpReal, Forward, EyePos);
}


Matrix Matrix::CalcViewMatrix(const Vec3& EyePos, const Vec3& TargetPos, const Vec3& Up)
{
	// LEFT HANDED
	Vec3 forward = (TargetPos - EyePos).Normalized();

	return 	Matrix::CalcLookAtMatrix(EyePos, forward, Up);
}


Matrix Matrix::CalcOrthoMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& Near, const float& Far)
{
	Matrix OrthoMat;

	OrthoMat.fMatrix[0][0] = 2.0f / (Right - Left);
	OrthoMat.fMatrix[1][1] = 2.0f / (Top - Bottom);
	OrthoMat.fMatrix[2][2] = -2.0f / (Far - Near);
	OrthoMat.fMatrix[3][3] = 1.0f;

	OrthoMat.fMatrix[3][0] = -(Right + Left) / (Right - Left);
	OrthoMat.fMatrix[3][1] = -(Top + Bottom) / (Top - Bottom);
	OrthoMat.fMatrix[3][2] = -(Far + Near) / (Far - Near);

	// Zero out the rest of the matrix
	OrthoMat.fMatrix[0][1] = OrthoMat.fMatrix[0][2] = OrthoMat.fMatrix[0][3] = 0.0f;
	OrthoMat.fMatrix[1][0] = OrthoMat.fMatrix[1][2] = OrthoMat.fMatrix[1][3] = 0.0f;
	OrthoMat.fMatrix[2][0] = OrthoMat.fMatrix[2][1] = OrthoMat.fMatrix[2][3] = 0.0f;

	return OrthoMat;
}


__inline Matrix Matrix::InverseSRT() const
{
	Matrix out;

	// Extract scale
	float sx = Vec3(fMatrix[0][0], fMatrix[0][1], fMatrix[0][2]).Magnitude();
	float sy = Vec3(fMatrix[1][0], fMatrix[1][1], fMatrix[1][2]).Magnitude();
	float sz = Vec3(fMatrix[2][0], fMatrix[2][1], fMatrix[2][2]).Magnitude();

	// Rotation = normalize basis
	Vec3 X = Vec3(fMatrix[0][0], fMatrix[0][1], fMatrix[0][2]) / sx;
	Vec3 Y = Vec3(fMatrix[1][0], fMatrix[1][1], fMatrix[1][2]) / sy;
	Vec3 Z = Vec3(fMatrix[2][0], fMatrix[2][1], fMatrix[2][2]) / sz;

	// Build rotation inverse (transpose)
	out.fMatrix[0][0] = X.x; out.fMatrix[0][1] = Y.x; out.fMatrix[0][2] = Z.x;
	out.fMatrix[1][0] = X.y; out.fMatrix[1][1] = Y.y; out.fMatrix[1][2] = Z.y;
	out.fMatrix[2][0] = X.z; out.fMatrix[2][1] = Y.z; out.fMatrix[2][2] = Z.z;

	// Apply inverse scale
	out.fMatrix[0][0] /= sx;
	out.fMatrix[1][1] /= sy;
	out.fMatrix[2][2] /= sz;

	// Invert translation
	Vec3 t = this->GetTranslation();
	Vec3 invT = -(t);

	// Compute transformed translation
	Vec3 newT;
	newT.x = invT.Dot(Vec3(out.fMatrix[0][0], out.fMatrix[1][0], out.fMatrix[2][0]));
	newT.y = invT.Dot(Vec3(out.fMatrix[0][1], out.fMatrix[1][1], out.fMatrix[2][1]));
	newT.z = invT.Dot(Vec3(out.fMatrix[0][2], out.fMatrix[1][2], out.fMatrix[2][2]));

	out.fMatrix[3][0] = newT.x;
	out.fMatrix[3][1] = newT.y;
	out.fMatrix[3][2] = newT.z;
	out.fMatrix[3][3] = 1.0f;

	return out;
}


__inline void Matrix::Decompose(Vec3& outScale, Vec3& outEuler, Vec3& outPos) const
{
	// 1. Extract translation (T)
	outPos = this->GetTranslation();

	// 2. Remove translation
	Matrix M = this->GetBasis();

	// 3. Extract scale from the basis vectors (since M = S*R)
	Vec3 X(M.fMatrix[0][0], M.fMatrix[0][1], M.fMatrix[0][2]);
	Vec3 Y(M.fMatrix[1][0], M.fMatrix[1][1], M.fMatrix[1][2]);
	Vec3 Z(M.fMatrix[2][0], M.fMatrix[2][1], M.fMatrix[2][2]);

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
	R.fMatrix[0][0] = X.x; R.fMatrix[0][1] = X.y; R.fMatrix[0][2] = X.z;
	R.fMatrix[1][0] = Y.x; R.fMatrix[1][1] = Y.y; R.fMatrix[1][2] = Y.z;
	R.fMatrix[2][0] = Z.x; R.fMatrix[2][1] = Z.y; R.fMatrix[2][2] = Z.z;

	// 5. Convert R to Euler
	outEuler = R.ExtractEuler(); // You must implement this
}


#define COLOR_NORMAL 0
#define COLOR_255 1

class Color
{
public:
	Color()
	{

	}

	Color(const float R, const float G, const float B, const float A = 255.0f, const int Mode = COLOR_255)
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;

		if (Mode != COLOR_255 && A > 1.0f)
		{
			this->A = this->A / 255.0f;
		}
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

	Vec3 GetRGB() const
	{
		return Vec3(R * (A / 255.0f), G * (A / 255.0f), B * (A / 255.0f));
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