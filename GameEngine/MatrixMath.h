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
		float _matrix[3][3];
	};

public:
	Matrix3x3()
	{
		memset(_matrix, 0, sizeof(this->_matrix));
	}

	Matrix3x3(float v[3][3])
	{
		memcpy(_matrix, v, sizeof(this->_matrix));
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
		return Vec3(_matrix[i][0], _matrix[i][1], _matrix[i][2]);
	}

	Vec3 Col(int i) const
	{
		return Vec3(_matrix[0][i], _matrix[1][i], _matrix[2][i]);
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
				R._matrix[r][c] =
					_matrix[r][0] * B._matrix[0][c]
					+ _matrix[r][1] * B._matrix[1][c]
					+ _matrix[r][2] * B._matrix[2][c];
			}
		}
		return R;
	}

	Matrix3x3 Transposed() const
	{
		Matrix3x3 R;
		R._11 = _11; R._12 = _21; R._13 = _31;
		R._21 = _12; R._22 = _22; R._23 = _32;
		R._31 = _13; R._32 = _23; R._33 = _33;
		return R;
	}

	Vec3 operator[](int i) const
	{
		return Row(i);
	}


	__inline Vec3 GetRight() const
	{
		return Vec3(this->_matrix[0][0], this->_matrix[0][1], this->_matrix[0][2]);
	}


	__inline Vec3 GetUp() const
	{
		return Vec3(this->_matrix[1][0], this->_matrix[1][1], this->_matrix[1][2]);
	}


	__inline Vec3 GetForward() const
	{
		return Vec3(this->_matrix[2][0], this->_matrix[2][1], this->_matrix[2][2]);
	}
	

	void Inverse()
	{
		float a = this->_11, b = this->_12, c = this->_13;
		float d = this->_21, e = this->_22, f = this->_23;
		float g = this->_31, h = this->_32, i = this->_33;

		float det =
			a * (e * i - f * h)
			- b * (d * i - f * g)
			+ c * (d * h - e * g);

		float invDet = 1.0f / det;

		this->_11 = (e * i - f * h) * invDet;
		this->_12 = -(b * i - c * h) * invDet;
		this->_13 = (b * f - c * e) * invDet;

		this->_21 = -(d * i - f * g) * invDet;
		this->_22 = (a * i - c * g) * invDet;
		this->_23 = -(a * f - c * d) * invDet;

		this->_31 = (d * h - e * g) * invDet;
		this->_32 = -(a * h - b * g) * invDet;
		this->_33 = (a * e - b * d) * invDet;
	}

	Matrix3x3 Inversed()
	{
		float a = this->_11, b = this->_12, c = this->_13;
		float d = this->_21, e = this->_22, f = this->_23;
		float g = this->_31, h = this->_32, i = this->_33;

		float det =
			a * (e * i - f * h)
			- b * (d * i - f * g)
			+ c * (d * h - e * g);

		float invDet = 1.0f / det;

		Matrix3x3 Out;
		Out._11 = (e * i - f * h) * invDet;
		Out._12 = -(b * i - c * h) * invDet;
		Out._13 = (b * f - c * e) * invDet;

		Out._21 = -(d * i - f * g) * invDet;
		Out._22 = (a * i - c * g) * invDet;
		Out._23 = -(a * f - c * d) * invDet;

		Out._31 = (d * h - e * g) * invDet;
		Out._32 = -(a * h - b * g) * invDet;
		Out._33 = (a * e - b * d) * invDet;

		return Out;
	}
};


class Matrix
{
public:

	Matrix()
	{
		memset(this->_matrix, 0.0f, sizeof(this->_matrix));
	}

	
	Matrix(float Matrix[4][4])
	{
		std::memcpy(this->_matrix, Matrix, sizeof(_matrix));
	}


	static Matrix CreateIdentity()
	{
		Matrix Out;
		std::memset(Out._matrix, 0, sizeof(Out._matrix));
		Out._matrix[0][0] = 1.f;
		Out._matrix[1][1] = 1.f;
		Out._matrix[2][2] = 1.f;
		Out._matrix[3][3] = 1.f;
		return Out;
	}


	void MakeIdentity()
	{
		std::memset(_matrix, 0, sizeof(_matrix));
		_matrix[0][0] = 1.f;
		_matrix[1][1] = 1.f;
		_matrix[2][2] = 1.f;
		_matrix[3][3] = 1.f;
	}


	// Transpose the matrix (swap rows and columns)
	void Transpose()
	{
		std::swap(_matrix[0][1], _matrix[1][0]);
		std::swap(_matrix[0][2], _matrix[2][0]);
		std::swap(_matrix[0][3], _matrix[3][0]);
		std::swap(_matrix[1][2], _matrix[2][1]);
		std::swap(_matrix[1][3], _matrix[3][1]);
		std::swap(_matrix[2][3], _matrix[3][2]);
	}


	// Transpose the upper-left 3x3 portion of the matrix
	void Transpose3x3()
	{
		std::swap(_matrix[0][1], _matrix[1][0]);
		std::swap(_matrix[0][2], _matrix[2][0]);
		std::swap(_matrix[1][2], _matrix[2][1]);
	}


	__inline Vec3 GetTranslation() const
	{
		return Vec3(this->_41, this->_42, this->_43);
	}


	__inline void SetTranslation(const Vec3& t)
	{
		this->_41 = t.x;
		this->_42 = t.y;
		this->_43 = t.z;
	}


	__inline Vec3 GetRight() const
	{
		return Vec3(_matrix[0][0], _matrix[0][1], _matrix[0][2]);
	}


	__inline Vec3 GetUp() const
	{
		return Vec3(_matrix[1][0], _matrix[1][1], _matrix[1][2]);
	}


	__inline Vec3 GetForward() const
	{
		return Vec3(_matrix[2][0], _matrix[2][1], _matrix[2][2]);
	}


	__inline void SetRight(const Vec3& R)
	{
		this->_11 = R.x;
		this->_12 = R.y;
		this->_13 = R.z;
	}


	__inline void SetUp(const Vec3& U)
	{
		this->_21 = U.x;
		this->_22 = U.y;
		this->_23 = U.z;
	}


	__inline void SetForward(const Vec3& F)
	{
		this->_31 = F.x;
		this->_32 = F.y;
		this->_33 = F.z;
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


	void NormalizeBasis()
	{
		Vec3 R = this->GetRight();
		Vec3 U = this->GetUp();
		Vec3 F = this->GetForward();

		F = R.Cross(U).Normalized();
		U = F.Cross(R).Normalized();

		this->SetRight(R);
		this->SetUp(U);
		this->SetForward(F);
	}


	// Can only be used when scaling isnt a factor
	// Mainly used for camera view
	Matrix QuickInversed()
	{
		Matrix out = *this;

		Vec3 t = this->GetTranslation();
		out.SetTranslation(Vec3(0.0f, 0.0f, 0.0f));

		out.Transpose3x3();

		Vec3 newT;
		newT = -t * out.Extract3x3();

		out.SetTranslation(newT);
		out._44 = 1.f;

		return out;
	}


	__inline Matrix InverseSRT() const;


	static void CreateRotationX(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();
		float c = cosf(ToRad(AngleDegrees)), s = sinf(ToRad(AngleDegrees));

		Out->_22 = c;
		Out->_23 = -s;
		Out->_32 = s;
		Out->_33 = c;
	}


	static void CreateRotationY(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();
		float c = cosf(ToRad(AngleDegrees)), s = sinf(ToRad(AngleDegrees));

		Out->_11 = c;
		Out->_13 = -s;
		Out->_31 = s;
		Out->_33 = c;
	}


	static void CreateRotationZ(Matrix* Out, float AngleDegrees)
	{
		Out->MakeIdentity();

		float c = cosf(ToRad(AngleDegrees));
		float s = sinf(ToRad(AngleDegrees));

		Out->_11 = c;
		Out->_12 = -s;
		Out->_21 = s;
		Out->_22 = c;
	}

	
	static Matrix CreateScalarMatrix(const Vec3&);


	static Matrix CreateRotationMatrix(const Vec3&);


	static Matrix CreateTranslationMatrix(const Vec3&);


	static Matrix CalcPerspectiveMatrix(float Fov, float AspectRatio, float Near, float Far)
	{
		float TanHalf = tanf(ToRad(Fov * 0.5f));

		float Fx = 1.0f / (TanHalf / AspectRatio);
		float Fy = (1.0f / TanHalf);
		float FN = Far - Near;

		float A = Far / (FN);
		float B = -Near * Far / FN;  

		Matrix M = Matrix::CreateIdentity();

		M._11 = Fx;
		M._22 = -Fy;                       
		M._33 =  A;
		M._34 =  1.0f;
		M._43 =  B;
		M._44 =  0.0f;

		return M;
	}


	__inline Matrix3x3 Extract3x3() const { return GetBasis3x3(); }


	Vec3 ExtractEuler()
	{
		Matrix3x3 B = GetBasis();
		Vec3 Scale = ExtractScale();

		Vec3 R = B.GetRight() / Scale.x;
		Vec3 U = B.GetUp() / Scale.y;
		Vec3 F = B.GetForward() / Scale.z;

		R.Normalize();
		U.Normalize();
		F.Normalize();

		float pitch, yaw, roll;

		if (fabs(U.z) < 0.99999f)
		{
			pitch = -asinf(U.z);
			yaw = -atan2f(-F.x, F.z);
			roll = atan2f(-U.x, U.y);
		}
		else
		{
			pitch = (U.z > 0) ? PI * 0.5f : -PI * 0.5f;
			yaw = -atan2f(R.z, F.z);
			roll = 0.0f;
		}

		return Vec3(
			ToDegree(pitch),
			ToDegree(yaw),
			ToDegree(roll)
		);
	}


	__inline Vec3 ExtractScale()
	{
		Vec3 X = this->GetRight();
		Vec3 Y = this->GetUp();
		Vec3 Z = this->GetForward();

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


	Matrix CalcInverseView(const Vec3& Up);


	void  MakeOrthoMatrix(const float& Left, const float& Right, const float& Top, const float& Bottom, const float& Near, const float& Far);


	void MakeViewMatrix(const Vec3& Pos, const Vec3& Target, const Vec3& Up);


	static __inline Matrix CreateMatrixFromRigthForwardUp(const Vec3&, const Vec3&, const Vec3&, const Vec3&);


	inline bool NearlyEqual(float a, float b, float eps = 1e-6f) const
	{
		return fabsf(a - b) < eps;
	}


	// General 3x3 inverse
	Matrix3x3 Inverse3x3()
	{
		float a = this->_11, b = this->_12, c = this->_13;
		float d = this->_21, e = this->_22, f = this->_23;
		float g = this->_31, h = this->_32, i = this->_33;

		float det =
			a * (e * i - f * h)
			- b * (d * i - f * g)
			+ c * (d * h - e * g);

		float invDet = 1.0f / det;

		Matrix3x3 out;

		out._11 = (e * i - f * h) * invDet;
		out._12 = -(b * i - c * h) * invDet;
		out._13 = (b * f - c * e) * invDet;

		out._21 = -(d * i - f * g) * invDet;
		out._22 = (a * i - c * g) * invDet;
		out._23 = -(a * f - c * d) * invDet;

		out._31 = (d * h - e * g) * invDet;
		out._32 = -(a * h - b * g) * invDet;
		out._33 = (a * e - b * d) * invDet;

		return out;
	}


	void SetBasis(const Matrix3x3* Basis)
	{
		std::memcpy(&_matrix[0][0], (Basis->_matrix), sizeof(float) * 9);
	}


	// works for affine transforms only 
	Matrix Inversed()
	{
		Matrix out;

		Matrix3x3 Upper = this->Inverse3x3();

		Vec3 NewTrans = -(this->GetTranslation() * Upper);

		out.SetBasis(&Upper);
		out.SetTranslation(NewTrans);

		return out;
	}


	// works for affine transforms only 
	void Inverse()
	{

		Matrix3x3 Upper = this->Inverse3x3();

		Vec3 NewTrans = -(this->GetTranslation() * Upper);

		this->SetBasis(&Upper);
		this->SetTranslation(NewTrans);
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
					sum += _matrix[row][i] * b._matrix[i][col];
				}

				result._matrix[row][col] = sum;
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
				this->_matrix[row][col] = this->_matrix[row][col] * b;
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
				if (!NearlyEqual(_matrix[r][c], rhs._matrix[r][c]))
					return false;
		return true;
	}


	__inline bool operator!=(const Matrix& rhs) const
	{
		return !(*this == rhs);
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
		float _matrix[4][4];
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
			os << v._matrix[i][j];
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
			this->_matrix[row][col] = m._matrix[row][col];
		}
	}
}

