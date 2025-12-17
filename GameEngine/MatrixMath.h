#pragma once

#define SNAP_ZERO(x) (fabsf(x) < 1e-6f ? 0.0f : (x))

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

	Matrix3x3(Matrix);

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


	__inline Vec3 GetRight() const
	{
		return Vec3(m[0][0], m[0][1], m[0][2]);
	}


	__inline Vec3 GetUp() const
	{
		return Vec3(m[1][0], m[1][1], m[1][2]);
	}


	__inline Vec3 GetForward() const
	{
		return Vec3(m[2][0], m[2][1], m[2][2]);
	}

};


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
		Matrix result = *this;

		for (int row = 0; row < 3; ++row)
		{
			for (int col = 0; col < 3; ++col)
			{
				result.fMatrix[row][col] = this->fMatrix[col][row];
			}
		}

		*this = result;
	}


	Matrix QuickInversed()
	{
		Matrix out = *this;

		Vec3 t = this->GetTranslation();
		out.SetTranslation(Vec3(0.0f, 0.0f, 0.0f));

		out.Transpose3x3();

		Vec3 newT;
		newT = t * out.Extract3x3();

		out.SetTranslation(-newT);
		out._44 = 1.f;

		return out;
	}


	//void QuickInverse()
	//{
	//	this->Transpose3x3();
	//	fMatrix[0][3] = fMatrix[1][3] = fMatrix[2][3] = 0.0f;

	//	float tx = fMatrix[3][0];
	//	float ty = fMatrix[3][1];
	//	float tz = fMatrix[3][2];

	//	fMatrix[3][0] = -(tx * fMatrix[0][0] + ty * fMatrix[0][1] + tz * fMatrix[0][2]);
	//	fMatrix[3][1] = -(tx * fMatrix[1][0] + ty * fMatrix[1][1] + tz * fMatrix[1][2]);
	//	fMatrix[3][2] = -(tx * fMatrix[2][0] + ty * fMatrix[2][1] + tz * fMatrix[2][2]);
	//	fMatrix[3][3] = 1.0f;

		//float tmp = this->fMatrix[0][1];
		//float tmp1 = this->fMatrix[1][2];
		//float tmp2 = this->fMatrix[0][2];

		//this->fMatrix[0][1] = this->fMatrix[1][0];
		//this->fMatrix[0][2] = this->fMatrix[2][0];
		//this->fMatrix[0][3] = 0.0f;

		//this->fMatrix[1][0] = tmp;
		//this->fMatrix[1][2] = this->fMatrix[2][1];
		//this->fMatrix[1][3] = 0.0f;

		//this->fMatrix[2][0] = tmp2;
		//this->fMatrix[2][1] = tmp1;
		//this->fMatrix[2][3] = 0.0f;


		// Calculate the last column of the output matrix manually
		//float t0 = -(this->fMatrix[3][0]);
		//float t1 = -(this->fMatrix[3][1]);
		//float t2 = -(this->fMatrix[3][2]);

		//this->fMatrix[3][0] = t0 * this->fMatrix[0][0] + t1 * this->fMatrix[1][0] + t2 * this->fMatrix[2][0];
		//this->fMatrix[3][1] = t0 * this->fMatrix[0][1] + t1 * this->fMatrix[1][1] + t2 * this->fMatrix[2][1];
		//this->fMatrix[3][2] = t0 * this->fMatrix[0][2] + t1 * this->fMatrix[1][2] + t2 * this->fMatrix[2][2];
		//this->fMatrix[3][3] = 1.0f;
//	}


	__inline Matrix InverseSRT() const;


	__inline void Decompose(Vec3& outScale, Vec3& outEuler, Vec3& outPos) const;


	static void CreateRotationX(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();
		float c = SNAP_ZERO(cosf(ToRad(AngleDegrees))), s = SNAP_ZERO(sinf(ToRad(AngleDegrees)));

		// Row-vector, left-handed
		Out->_22 = c;
		Out->_23 = s;
		Out->_32 = -s;
		Out->_33 = c;
	}


	static void CreateRotationY(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();
		float c = SNAP_ZERO(cosf(ToRad(AngleDegrees))), s = SNAP_ZERO(sinf(ToRad(AngleDegrees)));

		// Row-vector, left-handed
		Out->_11 = c;
		Out->_13 = -s;
		Out->_31 = s;
		Out->_33 = c;
	}


	static void CreateRotationZ(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();

		float c = SNAP_ZERO(cosf(ToRad(AngleDegrees)));
		float s = SNAP_ZERO(sinf(ToRad(AngleDegrees)));

		// Row-vector, left-handed
		Out->_11 = c;
		Out->_12 = s;
		Out->_21 = -s;
		Out->_22 = c;
	}

	
	static Matrix CreateScalarMatrix(const Vec3&);


	static Matrix CreateRotationMatrix(const Vec3&);


	static Matrix CreateTranslationMatrix(const Vec3&);


	static Matrix CalcPerspectiveMatrix(float Fov, float AspectRatio, float Near, float Far)
	{
		float TanHalf = tanf(ToRad(Fov * 0.5f));

		float Fx = 1.0f / (TanHalf * AspectRatio);
		float Fy = (1.0f / tanf(ToRad(Fov * 0.5f)));
		float FN = Far - Near;

		float A = Far / (FN);             // = 10/9
		float B = ((-Near * Far) / FN);  // = -10/9

		Matrix M = Matrix::CreateIdentity();

		M._11 = Fx;
		M._22 = -Fy;                       // keep if using DX/Vulkan inverted Y
		M._33 =  A;
		M._34 =  1.0f;
		M._43 =  B;
		M._44 =  0.0f;

		//Projection.fMatrix[0][0] = 1.0f / (tanHalf * aspect);
		//Projection.fMatrix[1][1] = 1.0f / tanHalf;

		//Projection.fMatrix[2][2] = A;
		//Projection.fMatrix[2][3] = B;

		//Projection.fMatrix[3][2] = 1.0f;
		//Projection.fMatrix[3][3] = 0.0f;

		return M;
	}


	__inline Matrix3x3 GetBasis3x3() const
	{
		Matrix3x3 R;

		R._11 = this->_11; R._12 = this->_12; R._13 = this->_13;
		R._21 = this->_21; R._22 = this->_22; R._23 = this->_23;
		R._31 = this->_31; R._32 = this->_32; R._33 = this->_33;

		return R;
	}


	__inline Matrix GetBasis() const
	{
		Matrix R = Matrix::CreateIdentity();

		R._11 = this->_11; R._12 = this->_12; R._13 = this->_13;
		R._21 = this->_21; R._22 = this->_22; R._23 = this->_23;
		R._31 = this->_31; R._32 = this->_32; R._33 = this->_33;

		R.SetTranslation(Vec3(0, 0, 0));

		return R;
	}


	__inline Matrix3x3 Extract3x3() const { return GetBasis3x3(); }


	Vec3 ExtractEuler()
	{
		Matrix3x3 B = this->GetBasis();
		Vec3 Scale = this->ExtractScale();

		Vec3 R = B.GetRight() / Scale.x;
		Vec3 U = B.GetUp() / Scale.y;
		Vec3 F = B.GetForward() / Scale.z;

		R.Normalize();
		U.Normalize();
		F.Normalize();

		float pitch, yaw, roll;

		// pitch = rotation around X
		// yaw   = rotation around Y
		// roll  = rotation around Z
		if (std::fabs(F.y) < 0.99999f)  // no gimbal lock
		{
			pitch = std::asin(-F.y);                   // Rx
			yaw = std::atan2(F.x, F.z);             // Ry
			roll = std::atan2(R.y, U.y);             // Rz
		}
		else
		{
			// gimbal lock, pitch = ±90°, yaw + roll ambiguous
			pitch = (F.y < 0) ? PI : -PI;
			yaw = 0.0f;
			roll = std::atan2(-U.x, R.x);
		}

		return Vec3(ToDegree(pitch), ToDegree(yaw), ToDegree(roll));
	}

	__inline Vec3 ExtractScale()
	{
		Vec3 X(this->fMatrix[0][0], this->fMatrix[0][1], this->fMatrix[0][2]);
		Vec3 Y(this->fMatrix[1][0], this->fMatrix[1][1], this->fMatrix[1][2]);
		Vec3 Z(this->fMatrix[2][0], this->fMatrix[2][1], this->fMatrix[2][2]);

		return Vec3(
			X.Magnitude(),
			Y.Magnitude(),
			Z.Magnitude()
		);
	}



	static Matrix ConstructViewMatrix(const Vec3& Right, const Vec3& Dir, const Vec3& Up, const Vec3& EyePos);


	static Matrix CalcLookAtMatrix(const Vec3& EyePos, const Vec3& Dir, const Vec3& Up);


	void CalcScalarMatrix(const Vec3&);


	void CalcRotationMatrix(const Vec3&);


	void CalcTranslationMatrix(const Vec3&);


	static Matrix CalcOrthoMatrix(const float& Left, const float& Right, const float& Top, const float& Bottom, const float& Near, const float& Far);


	static Matrix CalcViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up);


	void  MakeOrthoMatrix(const float& Left, const float& Right, const float& Top, const float& Bottom, const float& Near, const float& Far);


	void MakeViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up);


	static __inline Matrix CreateMatrixFromRigthForwardUp(const Vec3&, const Vec3&, const Vec3&, const Vec3&);


	//Matrix GetRTMat()
	//{
	//	Matrix r = *this;

	//	// row 0 = right
	//	Vec3 right(r.fMatrix[0][0], r.fMatrix[0][1], r.fMatrix[0][2]);
	//	right.Normalize();
	//	r.fMatrix[0][0] = right.x;
	//	r.fMatrix[0][1] = right.y;
	//	r.fMatrix[0][2] = right.z;

	//	// row 1 = up
	//	Vec3 up(r.fMatrix[1][0], r.fMatrix[1][1], r.fMatrix[1][2]);
	//	up.Normalize();
	//	r.fMatrix[1][0] = up.x;
	//	r.fMatrix[1][1] = up.y;
	//	r.fMatrix[1][2] = up.z;

	//	// row 2 = forward
	//	Vec3 fwd(r.fMatrix[2][0], r.fMatrix[2][1], r.fMatrix[2][2]);
	//	fwd.Normalize();
	//	r.fMatrix[2][0] = fwd.x;
	//	r.fMatrix[2][1] = fwd.y;
	//	r.fMatrix[2][2] = fwd.z;

	//	return r;
	//}


	inline bool NearlyEqual(float a, float b, float eps = 1e-6f) const
	{
		return fabsf(a - b) < eps;
	}


	__inline Matrix operator*(const Matrix& b) const
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


	__inline void operator *= (const float& b)
	{
		for (int row = 0; row < 4; row++)
		{
			for (int col = 0; col < 4; col++)
			{
				this->fMatrix[row][col] = this->fMatrix[row][col] * b;
			}
		}
	}


	__inline void operator *= (const Matrix& b)
	{
		*this = *this * b;
	}


	__inline bool operator==(const Matrix& rhs) const
	{
		for (int r = 0; r < 4; r++)
			for (int c = 0; c < 4; c++)
				if (!NearlyEqual(fMatrix[r][c], rhs.fMatrix[r][c]))
					return false;
		return true;
	}


	__inline bool operator!=(const Matrix& rhs) const
	{
		return !(*this == rhs);
	}


	__inline Vec3 GetRight() const
	{
		return Vec3(fMatrix[0][0], fMatrix[0][1], fMatrix[0][2]);
	}


	__inline Vec3 GetUp() const
	{
		return Vec3(fMatrix[1][0], fMatrix[1][1], fMatrix[1][2]);
	}


	__inline Vec3 GetForward() const
	{
		return Vec3(fMatrix[2][0], fMatrix[2][1], fMatrix[2][2]);
	}


	__inline Vec3 GetTranslation() const
	{
		return Vec3(this->_41, this->_42, this->_43);
	}

	__inline void SetTranslation(const Vec3 &t)
	{
		this->_41 = t.x;
		this->_42 = t.y;
		this->_43 = t.z;
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

	os << "\n";

	for (i = 0; i < 4; i++)
	{
		os << "[";
		for (j = 0; j < 4; j++)
		{
			os << v.fMatrix[i][j];
			os << ", ";
		}
		os << "]\n";
	}

	return os;
}


Matrix3x3::Matrix3x3(Matrix m)
{
	for (int row = 0; row < 3; ++row)
	{
		for (int col = 0; col < 3; ++col)
		{
			this->m[row][col] = m.fMatrix[row][col];
		}
	}
}

