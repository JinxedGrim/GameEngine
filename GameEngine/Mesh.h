#pragma once
#include <memory>
#include "Math.h"
#include "Texture.h"
#include "Materials.h"

// this is GPU style (typically SoA) vertex data, where each vertex has its own attributes
struct Vertex
{
	Vec4 Points[3] = {};
	Vec3 Normals[3] = {};
	Vec3 VertexNormals[3] = {};
	Vec3 VertexPositions[4] = {};
	Vec3 Col = Vec3(255, 0, 0);
	TextureCoords TexCoords[3] = {};
};


struct VertexAttributes
{
	Vec3 Attribute[3];
};


struct ClipVertex
{
	Vec4 Clip;
	Vec4 World;
	Vec3 Normal;
};


struct VertexBuffer
{
	// All interleaved  xyz,xyz
	std::vector<float> Vertices;
	std::vector<float> VertexNormals;
	std::vector<float> TexCoords;


	__inline const  Vec3& GetVertex(const size_t& index) const
	{
		size_t offset = index * 3;

		return 
		{
			Vertices[offset],
			Vertices[offset + 1],
			Vertices[offset + 2],
		};
	}


	__inline const  Vec3& GetNormal(const size_t& index) const
	{
		size_t offset = index * 3;

		return
		{
			VertexNormals[offset],
			VertexNormals[offset + 1],
			VertexNormals[offset + 2],
		};
	}


	__inline const  TextureCoords& GetTextureCoord(const size_t& index) const
	{
		size_t offset = index * 3;

		return
		{
			TexCoords[offset],
			TexCoords[offset + 1],
			TexCoords[offset + 2],
		};
	}


	VertexBuffer Copy() const
	{
		return *this;
	}


	void ScaleGeometry(Vec3 Factor)
	{
		for (size_t i = 0; i < Vertices.size(); i += 3)
		{
			Vertices[i] *= Factor.x;
			Vertices[i + 1] *= Factor.y;
			Vertices[i + 2] *= Factor.z;
		}
	}


	void ScaleGeometrySSE(const Vec3& factor)
	{
		const __m128 s0 = _mm_setr_ps(factor.x, factor.y, factor.z, factor.x);
		const __m128 s1 = _mm_setr_ps(factor.y, factor.z, factor.x, factor.y);
		const __m128 s2 = _mm_setr_ps(factor.z, factor.x, factor.y, factor.z);

		size_t i = 0;
		const size_t n = Vertices.size();
		float* p = Vertices.data();

		for (; i + 12 <= n; i += 12)
		{
			_mm_storeu_ps(p + i, _mm_mul_ps(_mm_loadu_ps(p + i), s0));
			_mm_storeu_ps(p + i + 4, _mm_mul_ps(_mm_loadu_ps(p + i + 4), s1));
			_mm_storeu_ps(p + i + 8, _mm_mul_ps(_mm_loadu_ps(p + i + 8), s2));
		}

		for (; i < n; i += 3)
		{
			p[i] *= factor.x;
			p[i + 1] *= factor.y;
			p[i + 2] *= factor.z;
		}
	}


	void ScaleGeometryAVX(const Vec3& factor)
	{
		const __m256 s0 = _mm256_setr_ps(
			factor.x, factor.y, factor.z,
			factor.x, factor.y, factor.z,
			factor.x, factor.y);

		const __m256 s1 = _mm256_setr_ps(
			factor.z, factor.x, factor.y,
			factor.z, factor.x, factor.y,
			factor.z, factor.x);

		const __m256 s2 = _mm256_setr_ps(
			factor.y, factor.z, factor.x,
			factor.y, factor.z, factor.x,
			factor.y, factor.z);

		size_t i = 0;
		const size_t n = Vertices.size();
		float* p = Vertices.data();

		for (; i + 24 <= n; i += 24)
		{
			_mm256_storeu_ps(p + i,
				_mm256_mul_ps(_mm256_loadu_ps(p + i), s0));

			_mm256_storeu_ps(p + i + 8,
				_mm256_mul_ps(_mm256_loadu_ps(p + i + 8), s1));

			_mm256_storeu_ps(p + i + 16,
				_mm256_mul_ps(_mm256_loadu_ps(p + i + 16), s2));
		}

		for (; i < n; i += 3)
		{
			p[i] *= factor.x;
			p[i + 1] *= factor.y;
			p[i + 2] *= factor.z;
		}
	}


	void TranslateGeometry(const Vec3& offset)
	{
		for (size_t i = 0; i < Vertices.size(); i += 3)
		{
			Vertices[i] += offset.x;
			Vertices[i + 1] += offset.y;
			Vertices[i + 2] += offset.z;
		}
	}


	void TranslateGeometryAVX(const Vec3& offset)
	{
		const __m256 t0 = _mm256_setr_ps(
			offset.x, offset.y, offset.z,
			offset.x, offset.y, offset.z,
			offset.x, offset.y);

		const __m256 t1 = _mm256_setr_ps(
			offset.z, offset.x, offset.y,
			offset.z, offset.x, offset.y,
			offset.z, offset.x);

		const __m256 t2 = _mm256_setr_ps(
			offset.y, offset.z, offset.x,
			offset.y, offset.z, offset.x,
			offset.y, offset.z);

		size_t i = 0;
		const size_t n = Vertices.size();
		float* p = Vertices.data();

		for (; i + 24 <= n; i += 24)
		{
			_mm256_storeu_ps(p + i,
				_mm256_add_ps(_mm256_loadu_ps(p + i), t0));

			_mm256_storeu_ps(p + i + 8,
				_mm256_add_ps(_mm256_loadu_ps(p + i + 8), t1));

			_mm256_storeu_ps(p + i + 16,
				_mm256_add_ps(_mm256_loadu_ps(p + i + 16), t2));
		}

		for (; i < n; i += 3)
		{
			p[i] += offset.x;
			p[i + 1] += offset.y;
			p[i + 2] += offset.z;
		}
	}


	void TranslateGeometrySSE(const Vec3& offset)
	{
		const __m128 t0 = _mm_setr_ps(offset.x, offset.y, offset.z, offset.x);
		const __m128 t1 = _mm_setr_ps(offset.y, offset.z, offset.x, offset.y);
		const __m128 t2 = _mm_setr_ps(offset.z, offset.x, offset.y, offset.z);

		size_t idx = 0;
		const size_t n = Vertices.size();
		float* p = Vertices.data();

		for (; idx + 12 <= n; idx += 12)
		{
			_mm_storeu_ps(p + idx, _mm_add_ps(_mm_loadu_ps(p + idx), t0));
			_mm_storeu_ps(p + idx + 4, _mm_add_ps(_mm_loadu_ps(p + idx + 4), t1));
			_mm_storeu_ps(p + idx + 8, _mm_add_ps(_mm_loadu_ps(p + idx + 8), t2));
		}

		for (; idx < n; idx += 3)
		{
			p[idx] += offset.x;
			p[idx + 1] += offset.y;
			p[idx + 2] += offset.z;
		}
	}


	void TransformGeometryAVX(const Matrix& matrix)
	{
		float* p = Vertices.data();
		const size_t n = Vertices.size();

		auto transform = [&](const __m256 x,
			const __m256 y,
			const __m256 z,
			int column) -> __m256
		{
			__m256 result = _mm256_mul_ps(
				x, _mm256_set1_ps(matrix._matrix[0][column]));

			result = _mm256_add_ps(result, _mm256_mul_ps(
				y, _mm256_set1_ps(matrix._matrix[1][column])));

			result = _mm256_add_ps(result, _mm256_mul_ps(
				z, _mm256_set1_ps(matrix._matrix[2][column])));

			return _mm256_add_ps(result,
				_mm256_set1_ps(matrix._matrix[3][column]));
		};

		size_t i = 0;

		for (; i + 24 <= n; i += 24)
		{
			__m256 x = _mm256_setr_ps(
				p[i], p[i + 3], p[i + 6], p[i + 9],
				p[i + 12], p[i + 15], p[i + 18], p[i + 21]);

			__m256 y = _mm256_setr_ps(
				p[i + 1], p[i + 4], p[i + 7], p[i + 10],
				p[i + 13], p[i + 16], p[i + 19], p[i + 22]);

			__m256 z = _mm256_setr_ps(
				p[i + 2], p[i + 5], p[i + 8], p[i + 11],
				p[i + 14], p[i + 17], p[i + 20], p[i + 23]);

			__m256 rx = transform(x, y, z, 0);
			__m256 ry = transform(x, y, z, 1);
			__m256 rz = transform(x, y, z, 2);

			alignas(32) float xs[8], ys[8], zs[8];

			_mm256_store_ps(xs, rx);
			_mm256_store_ps(ys, ry);
			_mm256_store_ps(zs, rz);

			for (int j = 0; j < 8; ++j)
			{
				p[i + j * 3] = xs[j];
				p[i + j * 3 + 1] = ys[j];
				p[i + j * 3 + 2] = zs[j];
			}
		}

		// Remaining vertices
		for (; i < n; i += 3)
		{
			float x = p[i], y = p[i + 1], z = p[i + 2];

			for (int c = 0; c < 3; ++c)
			{
				p[i + c] =
					x * matrix._matrix[0][c] +
					y * matrix._matrix[1][c] +
					z * matrix._matrix[2][c] +
					matrix._matrix[3][c];
			}
		}
	}


	void TransformGeometrySSE(const Matrix& matrix)
	{
		float* p = Vertices.data();
		const size_t n = Vertices.size();

		auto transform = [&](const __m128 x,
			const __m128 y,
			const __m128 z,
			int column) -> __m128
		{
			__m128 result = _mm_mul_ps(
				x, _mm_set1_ps(matrix._matrix[0][column]));

			result = _mm_add_ps(result, _mm_mul_ps(
				y, _mm_set1_ps(matrix._matrix[1][column])));

			result = _mm_add_ps(result, _mm_mul_ps(
				z, _mm_set1_ps(matrix._matrix[2][column])));

			return _mm_add_ps(result,
				_mm_set1_ps(matrix._matrix[3][column]));
		};

		size_t i = 0;

		for (; i + 12 <= n; i += 12)
		{
			__m128 x = _mm_setr_ps(
				p[i], p[i + 3], p[i + 6], p[i + 9]);

			__m128 y = _mm_setr_ps(
				p[i + 1], p[i + 4], p[i + 7], p[i + 10]);

			__m128 z = _mm_setr_ps(
				p[i + 2], p[i + 5], p[i + 8], p[i + 11]);

			__m128 rx = transform(x, y, z, 0);
			__m128 ry = transform(x, y, z, 1);
			__m128 rz = transform(x, y, z, 2);

			alignas(16) float xs[4], ys[4], zs[4];

			_mm_store_ps(xs, rx);
			_mm_store_ps(ys, ry);
			_mm_store_ps(zs, rz);

			for (int j = 0; j < 4; ++j)
			{
				p[i + j * 3] = xs[j];
				p[i + j * 3 + 1] = ys[j];
				p[i + j * 3 + 2] = zs[j];
			}
		}

		// Remaining vertices
		for (; i < n; i += 3)
		{
			float x = p[i], y = p[i + 1], z = p[i + 2];

			for (int c = 0; c < 3; ++c)
			{
				p[i + c] =
					x * matrix._matrix[0][c] +
					y * matrix._matrix[1][c] +
					z * matrix._matrix[2][c] +
					matrix._matrix[3][c];
			}
		}
	}


	void TransformGeometry(const Matrix& matrix)
	{
		float* p = Vertices.data();
		const size_t n = Vertices.size();

		// Cache matrix coefficients
		const float m00 = matrix._matrix[0][0];
		const float m01 = matrix._matrix[0][1];
		const float m02 = matrix._matrix[0][2];

		const float m10 = matrix._matrix[1][0];
		const float m11 = matrix._matrix[1][1];
		const float m12 = matrix._matrix[1][2];

		const float m20 = matrix._matrix[2][0];
		const float m21 = matrix._matrix[2][1];
		const float m22 = matrix._matrix[2][2];

		const float tx = matrix._matrix[3][0];
		const float ty = matrix._matrix[3][1];
		const float tz = matrix._matrix[3][2];

		for (size_t i = 0; i < n; i += 3)
		{
			const float x = p[i];
			const float y = p[i + 1];
			const float z = p[i + 2];

			p[i] = x * m00 + y * m10 + z * m20 + tx;
			p[i + 1] = x * m01 + y * m11 + z * m21 + ty;
			p[i + 2] = x * m02 + y * m12 + z * m22 + tz;
		}
	}
};


struct SubMesh
{
	Material* material = nullptr;
	uint32_t StartIndex = 0;
	uint32_t Length = 0;
	bool BackFaceCulling = true;
};


/*
class Triangle
{
	public:
	Triangle()
	{
		memset(Points.Points, 0, sizeof(Points.Points));
		memset(Points.Normals, 0, sizeof(Points.Normals));
	}


	Triangle(const Vec3& P1, const Vec3& P2, const Vec3& P3)
	{
		this->Points.Points[0] = P1;
		this->Points.Points[1] = P2;
		this->Points.Points[2] = P3;
	}


	Triangle(const Triangle& B)
	{
		*this = B;
	}


	void Translate(const Vec3& Value)
	{
		this->Points.Points[0] += Value;
		this->Points.Points[1] += Value;
		this->Points.Points[2] += Value;
	}


	const void Translated(Triangle* Out, const Vec3& Value)
	{
		Out->Points.Points[0] = this->Points.Points[0] + Value;
		Out->Points.Points[1] = this->Points.Points[1] + Value;
		Out->Points.Points[2] = this->Points.Points[2] + Value;
	}


	void Scale(const Vec3& Value)
	{
		this->Points.Points[0] *= Value;
		this->Points.Points[1] *= Value;
		this->Points.Points[2] *= Value;
	}


	const void Scaled(Triangle* Out, const Vec3& Value)
	{
		Out->Points.Points[0] = this->Points.Points[0] * Value;
		Out->Points.Points[1] = this->Points.Points[1] * Value;
		Out->Points.Points[2] = this->Points.Points[2] * Value;
	}


	void Rotate(const Matrix& Rot)
	{
		this->Points.Points[0] *= Rot;
		this->Points.Points[1] *= Rot;
		this->Points.Points[2] *= Rot;
	}


	const void Rotated(Triangle* Out, const Matrix& Rot)
	{
		Out->Points.Points[0] = this->Points.Points[0] * Rot;
		Out->Points.Points[1] = this->Points.Points[1] * Rot;
		Out->Points.Points[2] = this->Points.Points[2] * Rot;
	}

	
	void ApplyMatrix(const Matrix& MatToApply)
	{
		this->Points.Points[0] *= MatToApply;
		this->Points.Points[1] *= MatToApply;
		this->Points.Points[2] *= MatToApply;
	}


	void ApplyPerspectiveDivide()
	{
		this->Points.TexCoords[0].CorrectPerspective(this->Points.Points[0].w);
		this->Points.TexCoords[1].CorrectPerspective(this->Points.Points[1].w);
		this->Points.TexCoords[2].CorrectPerspective(this->Points.Points[2].w);

		this->Points.Points[0].CorrectPerspective();
		this->Points.Points[1].CorrectPerspective();
		this->Points.Points[2].CorrectPerspective();
	}


	const bool IsWindingCCW(const Vec4& a, const Vec4& b, const Vec4& c) const
	{
		Vec3 ab = (b / b.w).GetVec3() - (a / a.w).GetVec3();
		Vec3 ac = (c / c.w).GetVec3() - (a / a.w).GetVec3();
		Vec3 n = ab.Cross(ac);

		return n.z >= 0;  // CCW in LH DX
	}



	int ClipAgainstPlane(const Vec3& PointOnPlane, const Vec3& PlaneNormalized, Triangle& Out1, Triangle& Out2, bool DebugClip = false)
	{
		// Attributes
		Vec4 p0 = this->Points.Points[0];
		Vec4 p1 = this->Points.Points[1];
		Vec4 p2 = this->Points.Points[2];

		TextureCoords& t0 = this->Points.TexCoords[0];
		TextureCoords& t1 = this->Points.TexCoords[1];
		TextureCoords& t2 = this->Points.TexCoords[2];
		//

		// Attribute storage
		Vec4* InsidePoints[3] = {};
		Vec4* OutsidePoints[3] = {};
		TextureCoords* InsideTex[3] = {};
		TextureCoords* OutsideTex[3] = {};
		//

		float PlanePointDot = PlaneNormalized.Dot(PointOnPlane);
		float dist0 = PlaneNormalized.Dot(p0) - PlanePointDot;
		float dist1 = PlaneNormalized.Dot(p1) - PlanePointDot;
		float dist2 = PlaneNormalized.Dot(p2) - PlanePointDot;
		int InsideCount = 0;
		int OutsideCount = 0;
		int InsideTexCount = 0;
		int OutsideTexCount = 0;

		if (dist0 >= 0)
		{
			InsidePoints[InsideCount++] = &p0; 
			InsideTex[InsideTexCount++] = &t0;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p0; 
			OutsideTex[OutsideTexCount++] = &t0;
		}

		if (dist1 >= 0)
		{
			InsidePoints[InsideCount++] = &p1; 
			InsideTex[InsideTexCount++] = &t1;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p1; 
			OutsideTex[OutsideTexCount++] = &t1;
		}

		if (dist2 >= 0)
		{
			InsidePoints[InsideCount++] = &p2; 
			InsideTex[InsideTexCount++] = &t2;
		}
		else
		{
			OutsidePoints[OutsideCount++] = &p2; 
			OutsideTex[OutsideTexCount++] = &t2;
		}

		int Count = 0;

		if (InsideCount == 3)
		{
			Out1 = *this;
			Count = 1;
		}
		else if (InsideCount == 1)
		{
			Out1 = *this;

			if (DebugClip)
			{
				Out1.Points.Col = Vec3(0, 0, 255);
				Out1.OverrideTextureColor = true;
			}

			Out1.Points.Points[0] = *InsidePoints[0];
			Out1.Points.TexCoords[0] = *InsideTex[0];

			float t = 0.0f;
			Out1.Points.Points[1] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.Points.TexCoords[1] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out1.Points.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[1], PointOnPlane, PlaneNormalized, &t);
			Out1.Points.TexCoords[2] = InsideTex[0]->Lerp(*OutsideTex[1], t);

			Count = 1;
		}
		else if (InsideCount == 2)
		{
			Out1 = *this;
			Out2 = *this;

			if (DebugClip)
			{
				Out1.Points.Col = Vec3(0, 255, 0);
				Out1.OverrideTextureColor = true;
				Out2.Points.Col = Vec3(255, 0, 0);
				Out2.OverrideTextureColor = true;
			}


			Out1.Points.Points[0] = *InsidePoints[0];
			Out1.Points.Points[1] = *InsidePoints[1];
			Out1.Points.TexCoords[0] = *InsideTex[0];
			Out1.Points.TexCoords[1] = *InsideTex[1];

			float t = 0.0f;
			Out1.Points.Points[2] = InsidePoints[0]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out1.Points.TexCoords[2] = InsideTex[0]->Lerp(*OutsideTex[0], t);

			Out2.Points.Points[0] = *InsidePoints[1];
			Out2.Points.Points[1] = Out1.Points.Points[2];
			Out2.Points.TexCoords[0] = *InsideTex[1];
			Out2.Points.TexCoords[1] = Out1.Points.TexCoords[2];

			Out2.Points.Points[2] = InsidePoints[1]->CalculateIntersectionPoint(*OutsidePoints[0], PointOnPlane, PlaneNormalized, &t);
			Out2.Points.TexCoords[2] = InsideTex[1]->Lerp(*OutsideTex[0], t);

			Count = 2;
		}

		return Count;
	}


	int ClipAgainstPlane(const Vec4& plane, Triangle& Out1, Triangle& Out2, bool DebugClip) const
	{
		// Attributes to interp
		Vec4 in[3] = { Points.Points[0], Points.Points[1], Points.Points[2] };
		Vec4 world[3] = { WorldSpaceVerts.Points[0], WorldSpaceVerts.Points[1], WorldSpaceVerts.Points[2] };
		Vec3 Normals[3] = { Points.VertexNormals[0], Points.VertexNormals[1], Points.VertexNormals[2] };

		float w[3] = { Points.Points[0].w, Points.Points[1].w, Points.Points[2].w }; // store original w
		float d[3];
		bool inside[3];

		int inCount = 0;
		for (int i = 0; i < 3; i++)
		{
			d[i] = plane.Dot(in[i]);
			inside[i] = d[i] >= 0.0f;
			if (inside[i]) inCount++;
		}

		if (inCount == 0) return 0;

		auto IntersectPos = [&](int a, int b)
		{
			return d[a] / (d[a] - d[b]);
		};

		auto InterpolatePos = [&](int a, int b, Vec4* Inside, float t)
		{
			Vec4 pos = Inside[a] + (Inside[b] - Inside[a]) * t;

			// Interpolate w
			pos.w = w[a] + (w[b] - w[a]) * t;

			return pos;
		};

		auto InterpolateVec3 = [&](int a, int b, Vec3* Inside, float t)
		{
				Vec3 pos = Inside[a] + (Inside[b] - Inside[a]) * t;
				return pos;
		};

		Vec4 outPos[4];
		Vec4 outWorld[4];
		Vec3 outNormal[4];

		int outCount = 0;

		for (int i = 0; i < 3; i++)
		{
			int j = (i + 1) % 3;

			if (inside[i])
			{
				outPos[outCount] = in[i];
				outWorld[outCount] = world[i];
				outNormal[outCount] = Normals[i];
				outCount++;
			}

			if (inside[i] ^ inside[j])
			{
				float t = IntersectPos(i, j);
				outPos[outCount] = InterpolatePos(i, j, in, t);
				outWorld[outCount] = InterpolatePos(i, j, world, t);
				outNormal[outCount] = InterpolateVec3(i, j, Normals, t);
				outCount++;
			}
		}

		if (outCount == 3)
		{
			Out1 = *this;
			Out1.Points.Points[0] = outPos[0];
			Out1.Points.Points[1] = outPos[1];
			Out1.Points.Points[2] = outPos[2];
			Out1.Points.Normals[0] = outNormal[0];
			Out1.Points.Normals[1] = outNormal[1];
			Out1.Points.Normals[2] = outNormal[2];
			Out1.WorldSpaceVerts.Points[0] = outWorld[0];
			Out1.WorldSpaceVerts.Points[1] = outWorld[1];
			Out1.WorldSpaceVerts.Points[2] = outWorld[2];

			if (DebugClip && this->OverrideTextureColor != true)
			{
				bool Same = true;

				for (int i = 0; i < 3; i++)
				{
					if (!(Points.Points[i].GetVec3() == Out1.Points.Points[i].GetVec3()))
					{
						Same = false;
						break;
					}
				}

				if (!Same)
				{
					Out1.Points.Col = Vec3(0, 255, 0);
					Out1.OverrideTextureColor = true;
				}
			}

			return 1;
		}

		if (outCount == 4)
		{
			Out1 = *this;
			Out1.Points.Points[0] = outPos[0];
			Out1.Points.Points[1] = outPos[1];
			Out1.Points.Points[2] = outPos[2];
			Out1.Points.Normals[0] = outNormal[0];
			Out1.Points.Normals[1] = outNormal[1];
			Out1.Points.Normals[2] = outNormal[2];
			Out1.WorldSpaceVerts.Points[0] = outWorld[0];
			Out1.WorldSpaceVerts.Points[1] = outWorld[1];
			Out1.WorldSpaceVerts.Points[2] = outWorld[2];

			Out2 = *this;
			Out2.Points.Points[0] = outPos[0];
			Out2.Points.Points[1] = outPos[2];
			Out2.Points.Points[2] = outPos[3];
			Out2.Points.Normals[0] = outNormal[0];
			Out2.Points.Normals[1] = outNormal[2];
			Out2.Points.Normals[2] = outNormal[3];
			Out2.WorldSpaceVerts.Points[0] = outWorld[0];
			Out2.WorldSpaceVerts.Points[1] = outWorld[2];
			Out2.WorldSpaceVerts.Points[2] = outWorld[3];

			if (DebugClip)
			{
				Out1.Points.Col = Vec3(255, 0, 0);
				Out1.OverrideTextureColor = true;
				Out2.Points.Col = Vec3(0, 0, 255);
				Out2.OverrideTextureColor = true;
			}

			return 2;
		}

		return 0;
	}


	Vertex Points;
	Vertex WorldSpaceVerts = {};

	Vec3 FaceNormal = Vec3();

	Material* Material;

	bool OverrideTextureColor = false;
};
*/


struct Triangle
{
	static constexpr uint32_t InvalidIndex = UINT32_MAX;

	union
	{
		struct
		{
			uint32_t V1, V2, V3 = InvalidIndex;
		};

		uint32_t VertexIndices[3];
	};
	union
	{
		struct
		{
			uint32_t N1, N2, N3 = InvalidIndex;
		};

		uint32_t NormalIndices[3];
	};
	union
	{
		struct
		{
			uint32_t T1, T2, T3 = InvalidIndex;
		};

		uint32_t TextureIndices[3];
	};

	Vec3 Color = Vec3(255.0f, 0.0f, 0.0f);


	Triangle(uint32_t _v1, uint32_t _v2, uint32_t _v3)
	{
		this->V1 = _v1;
		this->V2 = _v2;
		this->V3 = _v3;
	}


	Triangle()
	{

	}


	Vec3 GetPoint(const VertexBuffer& Source, const int i) const
	{
		return Source.GetVertex(this->VertexIndices[i]);
	}


	Vec3 GetNormals(const VertexBuffer& Source, const int i) const
	{
		return Source.GetNormal(this->NormalIndices[i]);
	}


	TextureCoords GetTextureCoords(const VertexBuffer& Source, const int i) const
	{
		return Source.GetTextureCoord(this->TextureIndices[i]);
	}
};


struct TriangleGeometry
{
	Vec3 v1;
	Vec3 v2;
	Vec3 v3;
};


struct ObjectFileCommand
{
	std::string Command = "";
	std::string Args = "";
};


class ObjectFileLoader
{
	std::string FilePath = "";
	std::ifstream ifs = std::ifstream();

public:

	std::string MtlLibFn = "";


	ObjectFileLoader(std::string File)
	{
		this->FilePath = File;

		this->Load(this->FilePath);
	}

	bool Load(std::string FilePath)
	{
		this->ifs = std::ifstream(FilePath);

		if (!this->ifs.is_open())
			return false;

		return true;
	}


	ObjectFileCommand ParseLine()
	{
		ObjectFileCommand Cmd;

		if (!ifs.eof())
		{
			char Line[128];
			char Unused;
			std::istringstream SS;

			ifs.getline(Line, 128);

			SS = std::istringstream(Line);

			std::string command;
			SS >> command;

			// get args 
			// return command + args
			Cmd.Command = command;
			Cmd.Args = SS.str();

			return Cmd;
		}

		return Cmd;
	}
};


class Mesh
{
public:

	Mesh()
	{

	}


	Mesh(const std::vector<TriangleGeometry>& Geometry, std::string MeshName = "")
	{
		this->ClearMeshData();


		MeshName = std::move(MeshName);

		// Reserve enough capacity for worst-case geometry.
		_VertBuffer.Vertices.reserve(Geometry.size() * 9);
		Triangles.reserve(Geometry.size());

		// Convert geometry triangles into indexed triangles.
		for (const TriangleGeometry& source : Geometry)
		{
			Triangle tri{};

			tri.VertexIndices[0] = AddVertex(source.v1);
			tri.VertexIndices[1] = AddVertex(source.v2);
			tri.VertexIndices[2] = AddVertex(source.v3);

			// NormalIndices and TextureIndices remain UINT32_MAX.
			Triangles.push_back(tri);
		}

		// Assign the entire mesh a default material.
		Material* defaultMaterial = Material::GetNullMaterial();
		Materials.push_back(defaultMaterial);

		SubMesh sub{};
		sub.material = defaultMaterial;
		sub.StartIndex = 0;

		SubMeshes.push_back(sub);

		// Generate missing normals, finish submesh and counts.
		FinalizeMesh();
	}


	Mesh(std::string Fn)
	{
		this->LoadMesh(Fn);
	}


	Mesh(Mesh m, Material* Mat)
	{
		*this = m;

		if (this->_VertBuffer.VertexNormals.size() == 0)
		{
			this->CalculateFaceNormals();
		}

		this->ForceMatInfo(Mat);
	}


	void TranslateTriangles(Vec3 Value)
	{
		this->_VertBuffer.TranslateGeometry(Value);
	}


	void ScaleTriangles(Vec3 Value)
	{
		this->_VertBuffer.ScaleGeometry(Value);
	}


	Mesh ScaledTriangles(Vec3 Value)
	{
		Mesh Out = *this;
		Out._VertBuffer.ScaleGeometry(Value);
		return Out;
	}


	void ForceMatInfo(Material* MatToApply)
	{
		this->Materials.clear();
		this->Materials.push_back(MatToApply);
		this->SubMeshes.clear();

		SubMesh sub = SubMesh();

		sub.material = MatToApply;
		sub.StartIndex = 0;
		sub.Length = this->Triangles.size();

		SubMeshes.push_back(sub);
	}


	uint32_t AddVertex(const Vec3& vertex)
	{
		const size_t count = _VertBuffer.Vertices.size() / 3;

		for (size_t i = 0; i < count; ++i)
		{
			const size_t offset = i * 3;

			if (_VertBuffer.Vertices[offset] == vertex.x &&
				_VertBuffer.Vertices[offset + 1] == vertex.y &&
				_VertBuffer.Vertices[offset + 2] == vertex.z)
			{
				return (uint32_t)(i);
			}
		}

		const uint32_t index = (uint32_t)(count);

		_VertBuffer.Vertices.push_back(vertex.x);
		_VertBuffer.Vertices.push_back(vertex.y);
		_VertBuffer.Vertices.push_back(vertex.z);

		return index;
	}


	uint32_t ResolveOBJIndex(int index, size_t count)
	{
		if (index == 0)
			throw std::runtime_error("Invalid OBJ index");

		int64_t resolved = index > 0
			? static_cast<int64_t>(index) - 1
			: static_cast<int64_t>(count) + index;

		if (resolved < 0 || resolved >= static_cast<int64_t>(count))
			throw std::out_of_range("OBJ index out of range");

		return static_cast<uint32_t>(resolved);
	}


	bool HasValidNormals() const
	{
		const size_t count = _VertBuffer.VertexNormals.size() / 3;

		for (const Triangle& tri : Triangles)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (tri.NormalIndices[i] >= count || tri.NormalIndices[i] == UINT32_MAX)
					return false;
			}
		}

		return true;
	}


	void ClearMeshData()
	{
		_VertBuffer.Vertices.clear();
		_VertBuffer.VertexNormals.clear();
		_VertBuffer.TexCoords.clear();

		Triangles.clear();
		SubMeshes.clear();
		Materials.clear();

		VertexCount = 0;
		TriangleCount = 0;
		NormalCount = 0;
		TexCoordsCount = 0;
		MatCount = 0;
	}


	bool AddMaterial(std::string MtlLibFn,  std::string MaterialName, std::string Prefix)
	{
		this->SubMeshes.back().Length = (int)Triangles.size() - this->SubMeshes.back().StartIndex;

		if (this->SubMeshes.back().Length > 0)
		{
			TerraPGE::Core::LogInfo("[MATERIAL]", "Created new SubMesh for material: " + MaterialName + " Previous length: " + std::to_string(this->SubMeshes.back().Length));
			SubMesh sub = SubMesh();
			sub.StartIndex = this->Triangles.size();
			
			this->SubMeshes.push_back(sub);
		}

		Material* CurrMat = Material::LoadMaterialFile(MtlLibFn, MaterialName, Prefix);

		if (CurrMat == nullptr)
			return false;

		this->Materials.push_back(CurrMat);
		this->SubMeshes.back().material = this->Materials.back();

		this->MatCount = this->Materials.size();
	}


	void FinalizeMesh()
	{
		TerraPGE::Core::LogInfo("[Mesh]", "Finished Loading");

		if (!SubMeshes.empty())
		{
			SubMesh& sub = SubMeshes.back();

			sub.Length = Triangles.size() - sub.StartIndex;

			if (sub.Length == 0)
				SubMeshes.pop_back();
		}

		if (!Triangles.empty() && !HasValidNormals())
			CalculateFaceNormals();

		VertexCount = static_cast<int>(_VertBuffer.Vertices.size() / 3);
		TriangleCount = static_cast<int>(Triangles.size());
		NormalCount = static_cast<int>(_VertBuffer.VertexNormals.size() / 3);
		TexCoordsCount = static_cast<int>(_VertBuffer.TexCoords.size() / 3);
		MatCount = static_cast<int>(Materials.size());

#ifdef _DEBUG
		TerraPGE::Core::LogInfo("[Mesh]", "Mesh Name: " + this->MeshName);
		std::cout << "Vertices: " << VertexCount << "   Normals:   ";
		std::cout << NormalCount << "   TexturedCahce:   " << TexCoordsCount;
		std::cout << "   Faces:   " << TriangleCount << "   SubMeshes:   " << SubMeshes.size() << "\n\n";
#endif
	}


	bool LoadMesh(std::string FnPath, std::string Prefix="Assets\\")
	{
		ClearMeshData();

		std::string MtlLibFn = "";
		Material* CurrMat = Material::GetNullMaterial();
		this->Materials.push_back(CurrMat);

		MatCount = 1;

		if (this->SubMeshes.size() == 0)
		{
			SubMesh sub = SubMesh();
			sub.material = CurrMat;
			this->SubMeshes.push_back(sub);

			this->SubMeshes.back().StartIndex = 0;
		}

		TerraPGE::Core::LogInfo("[MATERIAL]", "Loading Mesh: " + Prefix + FnPath);

		ObjectFileLoader ObjLoader(Prefix + FnPath);
		ObjectFileCommand cmd = ObjLoader.ParseLine();

		while (cmd.Command != "")
		{
			std::istringstream SS = std::istringstream(cmd.Args);

			// Find and Load material
			if (cmd.Command == "mtllib")
			{
				SS >> MtlLibFn;
			}
			else if (cmd.Command == "usemtl")
			{
				std::string MaterialName;
				SS >> MaterialName;

				AddMaterial(MtlLibFn, MaterialName, Prefix);
			}

			// Vertex definitions
			else if (cmd.Command == "v")
			{
				Vec3 Vert;
				SS >> Vert.x >> Vert.y >> Vert.z;
				_VertBuffer.Vertices.push_back(Vert.x);
				_VertBuffer.Vertices.push_back(Vert.y);
				_VertBuffer.Vertices.push_back(Vert.z);
			}
			else if (cmd.Command == "vn")
			{
				Vec3 Normal;
				SS >> Normal.x >> Normal.y >> Normal.z;

				_VertBuffer.VertexNormals.push_back(Normal.x);
				_VertBuffer.VertexNormals.push_back(Normal.y);
				_VertBuffer.VertexNormals.push_back(Normal.z);
			}
			else if (cmd.Command == "vt")
			{
				TextureCoords TexCoord;
				SS >> TexCoord.u >> TexCoord.v;

//				if (std::fabs(TexCoord.u) > 1.0f)
//				{
//					TexCoord.u = std::fabs(fmod(TexCoord.u, 1.0f));
//				}
//				if (std::fabs(TexCoord.v) > 1.0f)
//				{
//					TexCoord.v = std::fabs(fmod(TexCoord.v, 1.0f));
//				}

				_VertBuffer.TexCoords.push_back(TexCoord.u);
				_VertBuffer.TexCoords.push_back(TexCoord.v);
				_VertBuffer.TexCoords.push_back(1.0f);
			}

			// Create Triangle			
			else if (cmd.Command == "f")
			{
				std::vector<std::string> Indices;
				std::string IndexStr;
				while (SS >> IndexStr)
				{
					if (IndexStr[0] == 'f')
						continue; // Skip the 'f' character

					Indices.push_back(IndexStr);
				}
				if (Indices.size() == 3)
				{
					Triangle Tmp;

					for (int i = 0; i < 3; i++)
					{
						std::stringstream IndexSS(Indices[i]);
						std::string IndexPart;

						int vertexIndex = 0, texCoordIndex = 0, normalIndex = 0;

						if (std::getline(IndexSS, IndexPart, '/'))
						{
							if (IndexPart.empty())
								std::runtime_error("Invalid OBJ position index");

							vertexIndex = ResolveOBJIndex(std::stoi(IndexPart), _VertBuffer.Vertices.size() / 3);
							Tmp.VertexIndices[i] = vertexIndex;
						}

						if (std::getline(IndexSS, IndexPart, '/'))
						{
							if (!IndexPart.empty())
								texCoordIndex = ResolveOBJIndex(std::stoi(IndexPart), _VertBuffer.TexCoords.size() / 3);
						}

						if (texCoordIndex >= 0)
							Tmp.TextureIndices[i] = texCoordIndex;
						else
							Tmp.TextureIndices[i] = UINT32_MAX;

						if (std::getline(IndexSS, IndexPart, '/'))
						{
							if (!IndexPart.empty())
								normalIndex = ResolveOBJIndex(std::stoi(IndexPart), _VertBuffer.VertexNormals.size() / 3);
						}

						if (normalIndex >= 0)
							Tmp.NormalIndices[i] = normalIndex;
						else
							Tmp.TextureIndices[i] = UINT32_MAX;
					}

					this->Triangles.push_back(Tmp);
				}
			}

			cmd = ObjLoader.ParseLine();
		}

		this->SubMeshes.back().Length = this->Triangles.size() - this->SubMeshes.back().StartIndex;
		MeshName = std::filesystem::path(FnPath).stem().string();

		FinalizeMesh();

		return true;
	}


	void CalculateFaceNormals()
	{
		_VertBuffer.VertexNormals.clear();
		_VertBuffer.VertexNormals.reserve(Triangles.size() * 3);

		for (Triangle& tri : Triangles)
		{
			const Vec3 p0 = _VertBuffer.GetVertex(tri.VertexIndices[0]);
			const Vec3 p1 = _VertBuffer.GetVertex(tri.VertexIndices[1]);
			const Vec3 p2 = _VertBuffer.GetVertex(tri.VertexIndices[2]);

			Vec3 normal = (p1 - p0).Cross(p2 - p0);

			if (normal.LengthSquared() > 1e-12f)
				normal.Normalize();
			else
				normal = Vec3(0.0f, 0.0f, 0.0f);

			const uint32_t normalIndex =
				static_cast<uint32_t>(_VertBuffer.VertexNormals.size() / 3);

			_VertBuffer.VertexNormals.push_back(normal.x);
			_VertBuffer.VertexNormals.push_back(normal.y);
			_VertBuffer.VertexNormals.push_back(normal.z);

			tri.NormalIndices[0] = normalIndex;
			tri.NormalIndices[1] = normalIndex;
			tri.NormalIndices[2] = normalIndex;
		}

		NormalCount = static_cast<int>(
			_VertBuffer.VertexNormals.size() / 3
		);
	}


	void CalculateSmoothFaceNormals()
	{
		//std::vector<Vec3> accumulatedNormals(this->VertexCache.size(), Vec3());

		//for (const IndexedTriangle& triangle : Triangles)
		//{
		//	const uint32_t i0 = triangle.Indices[0];
		//	const uint32_t i1 = triangle.Indices[1];
		//	const uint32_t i2 = triangle.Indices[2];

		//	const Vec3 edge1 = Vertices[i1].Position - Vertices[i0].Position;
		//	const Vec3 edge2 = Vertices[i2].Position - Vertices[i0].Position;

		//	const Vec3 faceNormal = edge1.Cross(edge2);

		//	accumulatedNormals[i0] += faceNormal;
		//	accumulatedNormals[i1] += faceNormal;
		//	accumulatedNormals[i2] += faceNormal;
		//}

		//for (Vec3& normal : accumulatedNormals)
		//{
		//	if (normal.LengthSquared() > 0.000001f)
		//	{
		//		normal.Normalize();
		//	}
		//}

		//this->NormalCache = this->Normals;
		//this->Normals = Normals;
	}


	std::vector<Triangle> Triangles = {};
	VertexBuffer _VertBuffer;
	std::vector<SubMesh> SubMeshes;
	std::vector<Material*> Materials = {};

	std::string MeshName = "";

	int VertexCount = 0;
	int TriangleCount = 0;
	int NormalCount = 0;

	int TexCount = 0;
	int TexCoordsCount = 0;
	int MatCount = 0;
};


namespace PrimitiveMeshes
{
	const Mesh CubeMesh = Mesh(
		{
			{ {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f} },
			{ {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },

			{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f} },
			{ {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f} },

			{ {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f} },
			{ {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },

			{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} },
			{ {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },

			{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f} },
			{ {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f} },

			{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
			{ {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
		},
		"Cube");


	class Cube : public Mesh
	{
	public:
		void AddDefaultTextureCoords()
		{
			_VertBuffer.TexCoords = {
				0.0f, 1.0f, 1.0f,
				0.0f, 0.0f, 1.0f,
				1.0f, 0.0f, 1.0f,
				1.0f, 1.0f, 1.0f
			};

			for (size_t i = 0; i < Triangles.size(); ++i)
			{
				Triangle& tri = Triangles[i];

				if (i % 2 == 1)
				{
					tri.TextureIndices[0] = 0;
					tri.TextureIndices[1] = 2;
					tri.TextureIndices[2] = 3;
				}
				else
				{
					tri.TextureIndices[0] = 0;
					tri.TextureIndices[1] = 1;
					tri.TextureIndices[2] = 2;
				}
			}

			TexCoordsCount = 4;
		}


		Cube(float Length, float Width, float Height, Material* Mat = Material::GetNullMaterial()) : Mesh(PrimitiveMeshes::CubeMesh, Mat)
		{
			AddDefaultTextureCoords();

			this->MeshName = "Cube";
		}
	};


	class Sphere : public Mesh
	{
		public:
		Sphere(float Radius, int Rings = 32, int LatitudeSegments = 16, Material* Mat = Material::GetNullMaterial()) : Mesh()
		{
			if (Radius <= 0.0f || Rings < 3 || LatitudeSegments < 2)
				throw std::invalid_argument("Invalid sphere parameters");

			MeshName = "Sphere";

			const int verticesPerRing = Rings + 1;
			const size_t vertexCount = static_cast<size_t>(LatitudeSegments + 1) * verticesPerRing;

			// Preallocate geometry storage.
			_VertBuffer.Vertices.reserve(vertexCount * 3);
			_VertBuffer.VertexNormals.reserve(vertexCount * 3);
			_VertBuffer.TexCoords.reserve(vertexCount * 3);

			Triangles.reserve(
				static_cast<size_t>(LatitudeSegments - 1) * Rings * 2
			);

			// Generate vertex attributes.
			for (int lat = 0; lat <= LatitudeSegments; ++lat)
			{
				const float theta =
					static_cast<float>(lat) * PI / LatitudeSegments;

				const float sinTheta = std::sin(theta);
				const float cosTheta = std::cos(theta);

				for (int lon = 0; lon <= Rings; ++lon)
				{
					const float phi = static_cast<float>(lon) * 2.0f * PI / Rings;

					const float sinPhi = std::sin(phi);
					const float cosPhi = std::cos(phi);

					const float nx = sinTheta * cosPhi;
					const float ny = cosTheta;
					const float nz = sinTheta * sinPhi;

					_VertBuffer.Vertices.push_back(Radius * nx);
					_VertBuffer.Vertices.push_back(Radius * ny);
					_VertBuffer.Vertices.push_back(Radius * nz);

					_VertBuffer.VertexNormals.push_back(nx);
					_VertBuffer.VertexNormals.push_back(ny);
					_VertBuffer.VertexNormals.push_back(nz);

					// Spherical UV mapping.
					const float u = static_cast<float>(lon) / Rings;
					const float v = static_cast<float>(lat) / LatitudeSegments;

					_VertBuffer.TexCoords.push_back(u);
					_VertBuffer.TexCoords.push_back(v);
					_VertBuffer.TexCoords.push_back(1.0f);
				}
			}

			for (int lat = 0; lat < LatitudeSegments; ++lat)
			{
				for (int lon = 0; lon < Rings; ++lon)
				{
					const uint32_t current = lat * verticesPerRing + lon;

					const uint32_t next = current + 1;
					const uint32_t below = current + verticesPerRing;
					const uint32_t belowNext = below + 1;

					if (lat > 0)
					{
						AddTriangle(current, below, next);
					}

					if (lat < LatitudeSegments - 1)
					{
						AddTriangle(next, below, belowNext);
					}
				}
			}

			ForceMatInfo(Mat);

			VertexCount = static_cast<int>(vertexCount);
			NormalCount = static_cast<int>(vertexCount);
			TexCoordsCount = static_cast<int>(vertexCount);
			TriangleCount = static_cast<int>(Triangles.size());
		}

		private:
		void AddTriangle(uint32_t a, uint32_t b, uint32_t c)
		{
			Triangle tri{};

			tri.VertexIndices[0] = a;
			tri.VertexIndices[1] = b;
			tri.VertexIndices[2] = c;

			// Each position has a matching smooth normal.
			tri.NormalIndices[0] = a;
			tri.NormalIndices[1] = b;
			tri.NormalIndices[2] = c;

			// Each position also has a matching UV.
			tri.TextureIndices[0] = a;
			tri.TextureIndices[1] = b;
			tri.TextureIndices[2] = c;

			Triangles.push_back(tri);
		}
	};


	class Plane : public Mesh
	{

		Plane()
		{

		}

	};
};