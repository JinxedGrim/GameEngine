#pragma once
#pragma once
#include <math.h>
#include <vector>
#include <ostream>

#define PI 3.14159265358979323846

#ifndef PixelRound
#define PixelRound(Val) (int)(Val + 0.5f)
#endif

#ifndef PixelRoundFloor
#define PixelRoundFloor(Val) (int)(Val)
#endif

#ifndef PixelRoundMinMax
#define PixelRoundMinMax(Val, Minimum, Maximum) std::max(Minimum, std::min(PixelRoundFloor(Val), Maximum))
#endif

#ifndef ToDegree
#define ToDegree(Deg) (float)((Deg) * (float)(180.0f / PI))
#endif

#ifndef ToRad
#define ToRad(Rad) (float)((Rad) * (float)(PI / 180.0f))
#endif

#ifndef ToDegreeD
#define ToDegreeD(Deg) (double)((Deg) * (180.0 / PI))
#endif

#ifndef ToRadD
#define ToRadD(Rad) (double)((Rad) * (PI / 180.0))
#endif

#define ContIdx(x, y, Width) (SIZE_T)(x + (Width * y))

#ifndef MATH_RIGHT_HANDED
#ifndef MATH_LEFT_HANDED
#define MATH_RIGHT_HANDED
#endif
#endif

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

namespace Matrices
{
	class Vec3;
	class Vec4;

	class Matrix
	{
	public:

		Matrix()
		{
			memset(this->fMatrix, 0.0f, sizeof(this->fMatrix));
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
			float c = cosf(AngleRads), s = sinf(AngleRads);
			Out->MakeIdentity();
			Out->_22 = c;
			Out->_23 = s;
			Out->_32 = -s;
			Out->_33 = c;
		}

		static void CreateRotationY(Matrix* Out, float AngleRads)
		{
			Out->MakeIdentity();

			float c = cosf(AngleRads);
			float s = sinf(AngleRads);

#ifdef MATH_LEFT_HANDED
			Out->_11 = c;
			Out->_13 = -s;
			Out->_31 = s;
			Out->_33 = c;
#else
			Out->_11 = c;
			Out->_13 = s;
			Out->_31 = -s;
			Out->_33 = c;
#endif
		}

		static void CreateRotationZ(Matrix* Out, float AngleRads)
		{
			float c = cosf(AngleRads), s = sinf(AngleRads);

			Out->MakeIdentity();

#ifdef MATH_LEFT_HANDED
			Out->_11 = c;
			Out->_12 = s;
			Out->_21 = -s;
			Out->_22 = c;
#else
			Out->_11 = c;
			Out->_12 = -s;
			Out->_21 = s;
			Out->_22 = c;
#endif
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

		void  MakeOrthoMatrix(const float& Left, const float& Right, const float& Top, const float& Bottom, const float& Near, const float& Far);

		static Matrix CalcOrthoMatrix(const float& Left, const float& Right, const float& Top, const float& Bottom, const float& Near, const float& Far);

		static Matrix CalcViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up);

		void MakeViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up);

		static __inline Matrix CreateMatrixFromRigthForwardUp(const Vec3&, const Vec3&, const Vec3&, const Vec3&);


		Matrix ExtractRotationScale() const
		{
			Matrix r = *this;
			// Zero translation (depends on your layout!)
			r.fMatrix[3][0] = 0.0f;
			r.fMatrix[3][1] = 0.0f;
			r.fMatrix[3][2] = 0.0f;
			return r;
		}

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

		inline bool NearlyEqual(float a, float b, float eps = 1e-6f) const
		{
			return fabsf(a - b) < eps;
		}

		bool operator==(const Matrix& rhs) const
		{
			for (int r = 0; r < 4; r++)
				for (int c = 0; c < 4; c++)
					if (!NearlyEqual(fMatrix[r][c], rhs.fMatrix[r][c]))
						return false;
			return true;
		}

		bool operator!=(const Matrix& rhs) const
		{
			return !(*this == rhs);
		}

		// Transpose the matrix (swap rows and columns)
		void Transpose()
		{
			Matrix result;

			for (int row = 0; row < 4; ++row)
			{
				for (int col = 0; col < 4; ++col)
				{
					result.fMatrix[col][row] = fMatrix[row][col];
				}
			}

			*this = result;
		}

		// Transpose the upper-left 3x3 portion of the matrix
		void Transpose3x3()
		{
			Matrix result;

			for (int row = 0; row < 3; ++row)
			{
				for (int col = 0; col < 3; ++col)
				{
					result.fMatrix[col][row] = fMatrix[row][col];
				}
			}

			// Copy the unchanged elements
			result.fMatrix[3][3] = fMatrix[3][3];

			*this = result;
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

		void QuickInverse()
		{
			float tmp = this->fMatrix[0][1];
			float tmp1 = this->fMatrix[1][2];
			float tmp2 = this->fMatrix[0][2];

			this->fMatrix[0][1] = this->fMatrix[1][0];
			this->fMatrix[0][2] = this->fMatrix[2][0];
			this->fMatrix[0][3] = 0.0f;

			this->fMatrix[1][0] = tmp;
			this->fMatrix[1][2] = this->fMatrix[2][1];
			this->fMatrix[1][3] = 0.0f;

			this->fMatrix[2][0] = tmp2;
			this->fMatrix[2][1] = tmp1;
			this->fMatrix[2][3] = 0.0f;


			// Calculate the last column of the output matrix manually
			float t0 = -(this->fMatrix[3][0]);
			float t1 = -(this->fMatrix[3][1]);
			float t2 = -(this->fMatrix[3][2]);

			this->fMatrix[3][0] = t0 * this->fMatrix[0][0] + t1 * this->fMatrix[1][0] + t2 * this->fMatrix[2][0];
			this->fMatrix[3][1] = t0 * this->fMatrix[0][1] + t1 * this->fMatrix[1][1] + t2 * this->fMatrix[2][1];
			this->fMatrix[3][2] = t0 * this->fMatrix[0][2] + t1 * this->fMatrix[1][2] + t2 * this->fMatrix[2][2];
			this->fMatrix[3][3] = 1.0f;
		}

		__inline Matrix InverseSRT() const;

		static Matrix CalcPerspectiveMatrix(float Fov, float AspectRatio, float Near, float Far)
		{
			float FovRads = abs(1.0f / tanf((ToRad(Fov * 0.5f))));

			Matrix Out;

			Out.fMatrix[0][0] = AspectRatio / FovRads;
			Out.fMatrix[1][1] = -FovRads; // THIS NEGATIVE IS FOR TERRAGL
			Out.fMatrix[2][2] = Far / (Far - Near);
			Out.fMatrix[3][2] = (-Far * Near) / (Far - Near);
			Out.fMatrix[2][3] = 1.0f;
			Out.fMatrix[3][3] = 0.0f;

			return Out;
		}

		__inline void Decompose(Vec3& outScale, Vec3& outEuler, Vec3& outPos) const;

		__inline Vec3 ToEulerAnglesXYZ() const;

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


	class Matrix3x3
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13;
				float _21, _22, _23;
				float _31, _32, _33;
			};
			float m[3][3];
		};

	public:
		Matrix3x3()
		{
			memset(m, 0, sizeof(m));
		}

		Matrix3x3(float v[3][3])
		{
			memcpy(m, v, sizeof(m));
		}

		static Matrix3x3 Identity()
		{
			Matrix3x3 R;
			R._11 = 1; R._22 = 1; R._33 = 1;
			return R;
		}

		Vec3 Row(int i) const
		{
			return Vec3(m[i][0], m[i][1], m[i][2]);
		}

		Vec3 Col(int i) const
		{
			return Vec3(m[0][i], m[1][i], m[2][i]);
		}

		Vec3 operator*(const Vec3& v) const
		{
			return Vec3(
				_11 * v.x + _12 * v.y + _13 * v.z,
				_21 * v.x + _22 * v.y + _23 * v.z,
				_31 * v.x + _32 * v.y + _33 * v.z
			);
		}

		Matrix3x3 operator*(const Matrix3x3& B) const
		{
			Matrix3x3 R;
			for (int r = 0; r < 3; r++)
			{
				for (int c = 0; c < 3; c++)
				{
					R.m[r][c] =
						m[r][0] * B.m[0][c]
						+ m[r][1] * B.m[1][c]
						+ m[r][2] * B.m[2][c];
				}
			}
			return R;
		}

		Matrix3x3 Transposed() const
		{
			Matrix3x3 R;
			for (int r = 0; r < 3; r++)
				for (int c = 0; c < 3; c++)
					R.m[c][r] = m[r][c];
			return R;
		}

		Vec3 operator[](int i) const
		{
			return Row(i);
		}

	};

	inline Matrix3x3 ExtractRotation3x3(const Matrix& M)
	{
		Matrix3x3 R;

		R._11 = M._11; R._12 = M._12; R._13 = M._13;
		R._21 = M._21; R._22 = M._22; R._23 = M._23;
		R._31 = M._31; R._32 = M._32; R._33 = M._33;

		return R;
	}

	__inline Vec3 Matrix::ToEulerAnglesXYZ() const
	{
		float r00 = fMatrix[0][0], r01 = fMatrix[0][1], r02 = fMatrix[0][2];
		float r10 = fMatrix[1][0], r11 = fMatrix[1][1], r12 = fMatrix[1][2];
		float r20 = fMatrix[2][0], r21 = fMatrix[2][1], r22 = fMatrix[2][2];

		float pitch, yaw, roll;

		// ---- yaw extraction (from forward.x and forward.z) ----
		yaw = std::atan2(r20, r22);

		// ---- pitch extraction ----
		// pitch = asin(-forward.y)
		float cy = std::sqrt(r20 * r20 + r22 * r22);
		pitch = std::atan2(-r21, cy);

		// ---- roll extraction ----
		if (cy > 1e-6f)     // not in gimbal lock
		{
			roll = std::atan2(r01, r00);
		}
		else
		{
			// gimbal lock: pitch is +-90
			roll = std::atan2(-r12, r11);
		}

		return Vec3(
			ToDegree(pitch),
			ToDegree(yaw),
			ToDegree(roll)
		);
	}

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
		Matrix Out = Matrix::CreateIdentity();
		Out._41 = Translation.x;
		Out._42 = Translation.y;
		Out._43 = Translation.z;
		return Out;
	}

	Matrix Matrix::CreateRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
	{
		float RotRadsZ = RotationDeg.z;
		Matrix A;
		Matrix B;
		Matrix C;
		Matrix::CreateRotationX(&A, ToRad(RotationDeg.x)); // pitch
		Matrix::CreateRotationY(&B, ToRad(RotationDeg.y)); // yaw
		Matrix::CreateRotationZ(&C, ToRad(RotRadsZ));      // roll

		A = A * B * C;

		return A;
	}

	__inline Matrix Matrix::CreateMatrixFromRigthForwardUp(const Vec3& Right, const Vec3& Up, const Vec3& Forward, const Vec3& Trans)
	{
		Matrix rotMatrix = Matrix::CreateIdentity();

		// row-major: row0 = right, row1 = up, row2 = forward
		rotMatrix._11 = Right.x;  rotMatrix._12 = Right.y;  rotMatrix._13 = Right.z;  rotMatrix._14 = 0.0f;
		rotMatrix._21 = Up.x;     rotMatrix._22 = Up.y;     rotMatrix._23 = Up.z;     rotMatrix._24 = 0.0f;
		rotMatrix._31 = Forward.x; rotMatrix._32 = Forward.y; rotMatrix._33 = Forward.z; rotMatrix._34 = 0.0f;

		// translation row
		rotMatrix._41 = Trans.x; rotMatrix._42 = Trans.y; rotMatrix._43 = Trans.z; rotMatrix._44 = 1.0f;

		return rotMatrix;
	}

	void Matrix::CalcScalarMatrix(const Vec3& Scalar)
	{
		this->MakeIdentity();
		this->fMatrix[0][0] = 1.f * Scalar.x;
		this->fMatrix[1][1] = 1.f * Scalar.y;
		this->fMatrix[2][2] = 1.f * Scalar.z;
		this->fMatrix[3][3] = 1.f;
	}

	void Matrix::CalcTranslationMatrix(const Vec3& Translation)
	{
		Matrix Out;
		this->MakeIdentity();
		this->fMatrix[3][0] = Translation.x;
		this->fMatrix[3][1] = Translation.y;
		this->fMatrix[3][2] = Translation.z;
	}

	void Matrix::CalcRotationMatrix(const Vec3& RotationDeg) // pitch yaw roll
	{
		this->MakeIdentity();
		float RotRadsZ = RotationDeg.z;

		Matrix A;
		Matrix B;
		Matrix C;
		Matrix::CreateRotationX(&A, -ToRad(RotationDeg.x));
		Matrix::CreateRotationY(&B, ToRad(RotationDeg.y));
		Matrix::CreateRotationZ(&C, ToRad(RotRadsZ));

		A = A * B * C;

		//return A;
	}

	void __fastcall Matrix::MakeOrthoMatrix(const float& Left, const float& Right, const float& Bottom, const float& Top, const float& Near, const float& Far)
	{
		this->fMatrix[0][0] = 2.0f / (Right - Left);
		this->fMatrix[1][1] = 2.0f / (Top - Bottom);
		this->fMatrix[2][2] = -2.0f / (Far - Near);
		this->fMatrix[3][3] = 1.0f;

		this->fMatrix[3][0] = -(Right + Left) / (Right - Left);
		this->fMatrix[3][1] = -(Top + Bottom) / (Top - Bottom);
		this->fMatrix[3][2] = -(Far + Near) / (Far - Near);

		// Zero out the rest of the matrix
		this->fMatrix[0][1] = this->fMatrix[0][2] = this->fMatrix[0][3] = 0.0f;
		this->fMatrix[1][0] = this->fMatrix[1][2] = this->fMatrix[1][3] = 0.0f;
		this->fMatrix[2][0] = this->fMatrix[2][1] = this->fMatrix[2][3] = 0.0f;
	}

	void __fastcall Matrix::MakeViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up)
	{
#if LH_COORDINATE_SYSTEM
		Vec3 forward = (Target - Pos).Normalized(); // +Z forward
#else
		Vec3 forward = (Pos - Target).Normalized(); // -Z forward
#endif

		Vec3 right = Up.Cross(forward).Normalized(); // cross order chosen so right points +X
		Vec3 realUp = forward.Cross(right); // completes orthonormal triad

		// fill matrix with rows (row-major, row-vector)
		this->_11 = right.x;  this->_12 = right.y;  this->_13 = right.z; this->_14 = 0.0f;
		this->_21 = realUp.x; this->_22 = realUp.y; this->_23 = realUp.z; this->_24 = 0.0f;
		this->_31 = forward.x; this->_32 = forward.y; this->_33 = forward.z; this->_34 = 0.0f;

		this->_41 = Pos.x;    this->_42 = Pos.y;    this->_43 = Pos.z;    this->_44 = 1.0f;
	}

	Matrix Matrix::CalcViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up)
	{
		Matrix ViewMat;

#if LH_COORDINATE_SYSTEM
		Vec3 forward = (Target - Pos).Normalized(); // +Z forward
#else
		Vec3 forward = (Pos - Target).Normalized(); // -Z forward
#endif

		Vec3 right = Up.Cross(forward).Normalized(); // cross order chosen so right points +X
		Vec3 realUp = forward.Cross(right); // completes orthonormal triad

		// fill matrix with rows (row-major, row-vector)
		ViewMat._11 = right.x;  ViewMat._12 = right.y;  ViewMat._13 = right.z; ViewMat._14 = 0.0f;
		ViewMat._21 = realUp.x; ViewMat._22 = realUp.y; ViewMat._23 = realUp.z; ViewMat._24 = 0.0f;
		ViewMat._31 = forward.x; ViewMat._32 = forward.y; ViewMat._33 = forward.z; ViewMat._34 = 0.0f;

		ViewMat._41 = Pos.x;    ViewMat._42 = Pos.y;    ViewMat._43 = Pos.z;    ViewMat._44 = 1.0f;

		return ViewMat;
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
		Vec3 t(fMatrix[3][0], fMatrix[3][1], fMatrix[3][2]);
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
		outPos = Vec3(
			fMatrix[3][0],
			fMatrix[3][1],
			fMatrix[3][2]
		);

		// 2. Remove translation
		Matrix M = *this;
		M.fMatrix[3][0] = M.fMatrix[3][1] = M.fMatrix[3][2] = 0.0f;

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
		outEuler = R.ToEulerAnglesXYZ(); // You must implement this
	}

}

namespace Vectors
{
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
		float Distance(Vec3& b) const
		{
			return (float)sqrt(pow(b.x - this->x, 2) + pow(b.y - this->y, 2) + pow(b.z - this->z, 2));
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
			return (this->y > this->x ? (this->x > this->y ? 2 : 1) : (this->x > this->x ? 2 : 0));
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

	std::ostream& operator << (std::ostream& os, const Vec3& v)
	{
		os << "x: " << v.x << " y: " << v.y << " z: " << v.z;
		return os;
	}
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
			x *= invW;
			y *= invW;
			z *= invW;
			this->w = w;
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

	Vec4 operator * (const Matrix& b) const
	{
		Vec4 Out;

		Out.x = this->x * b.fMatrix[0][0] + this->y * b.fMatrix[1][0] + this->z * b.fMatrix[2][0] + this->w * b.fMatrix[3][0];
		Out.y = this->x * b.fMatrix[0][1] + this->y * b.fMatrix[1][1] + this->z * b.fMatrix[2][1] + this->w * b.fMatrix[3][1];
		Out.z = this->x * b.fMatrix[0][2] + this->y * b.fMatrix[1][2] + this->z * b.fMatrix[2][2] + this->w * b.fMatrix[3][2];
		Out.w = this->x * b.fMatrix[0][3] + this->y * b.fMatrix[1][3] + this->z * b.fMatrix[2][3] + this->w * b.fMatrix[3][3];

		return Out;
	}

	void operator *= (const Matrix& b)
	{
		float _tmpx = this->x;
		float _tmpy = this->y;
		float _tmpz = this->z;


		this->x = _tmpx * b.fMatrix[0][0] + _tmpy * b.fMatrix[1][0] + _tmpz * b.fMatrix[2][0] + this->w * b.fMatrix[3][0];
		this->y = _tmpx * b.fMatrix[0][1] + _tmpy * b.fMatrix[1][1] + _tmpz * b.fMatrix[2][1] + this->w * b.fMatrix[3][1];
		this->z = _tmpx * b.fMatrix[0][2] + _tmpy * b.fMatrix[1][2] + _tmpz * b.fMatrix[2][2] + this->w * b.fMatrix[3][2];
		this->w = _tmpx * b.fMatrix[0][3] + _tmpy * b.fMatrix[1][3] + _tmpz * b.fMatrix[2][3] + this->w * b.fMatrix[3][3];
	}

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


#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif