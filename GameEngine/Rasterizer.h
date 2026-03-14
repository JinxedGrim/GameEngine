#pragma once
#include "EngineCore.h"
#include "Shading.h"
#include "Mesh.h"
#include "Renderable.h"

// Important mathematical concepts used in this rasterizer
// Barycentric Interpolation (alpha-beta-gamma) (Derived from edge equations)
// 


namespace TerraPGE::Renderer
{
	static bool DebugDepthBuffer = false;
	static bool DebugShadows = false;
	static bool DebugShadowMap = false;
	static bool DebugShadowValue = false;
	static bool SkipDepthTesting = false;
	static float TestDepth = -1.0f;
	static float TestDepthMapped = -1.0f;
	static bool DoShadows = true;
	static bool WireFrame = false;
	static bool ShowTriLines = false; // TODO make a shader for this
	static float TestHdrExposure = 1.0f;
	static bool UseHDR = true;
	static bool DoGammaCorrection = true;
	static bool DoLighting = true;
	static bool DebugClip = false;
	static bool DoCull = true;

	namespace RenderingCore
	{
		__inline void SetPixelFrameBuffer(float* Dst, const int x, const int y, const int Width, const float R, const float G, const float B)
		{
			int index = ContIdx(x, y, Width) * 3;
			Dst[index++] = R;
			Dst[index++] = G;
			Dst[index] = B;
		}
	}


	struct FragmentInfo
	{
		Vec3 BaryCoords;
		Vec4 InterpolatedPos;
		Vec4 NdcPos;
		float Depth;
	};


	struct BaryCoords
	{
		float Alpha;
		float Beta;
		float Gamma;

		void SetBary(const float A, const float B, const float G)
		{
			this->Alpha = A;
			this->Beta = B;
			this->Gamma = G;
		}


		float NaiveInterpolate(const float& a, const float& b, const float& c)
		{
			return a * this->Alpha + b * this->Beta + this->Gamma * c;
		}


		const float PerspectiveCorrectInterpolate(float Val0, float Val1, float Val2, float w0, float w1, float w2)
		{
			// Undivide && interpolate

			if (w0 == 0)
				w0 = FLOAT_LOWEST;
			if (w1 == 0)
				w1 = FLOAT_LOWEST;
			if (w2 == 0)
				w2 = FLOAT_LOWEST;

			float InvW0 = 1 / w0;
			float InvW1 = 1 / w1;
			float InvW2 = 1 / w2;

			float InterpolatedW = this->NaiveInterpolate(InvW0, InvW1, InvW2);

			if (InterpolatedW <= 0.0f)
				InterpolatedW = FLOAT_LOWEST;

			// 
			float vInterpolated =
				(Val0 * InvW0) * this->Alpha +
				(Val1 * InvW1) * this->Beta +
				(Val2 * InvW2) * this->Gamma;

			// 
			return vInterpolated / InterpolatedW;
		}
	};


	namespace RasterUtils
	{
		Vec3 MapInterpedVec3ToColor(Vec3 p0, Vec3 p1, Vec3 p2, Vec3 InterpedVal)
		{
			Vec3 minPos = Vec3(
				std::min({ p0.x, p1.x, p2.x }),
				std::min({ p0.y, p1.y, p2.y }),
				std::min({ p0.z, p1.z, p2.z })
			);

			Vec3 maxPos = Vec3(
				std::max({ p0.x, p1.x, p2.x }),
				std::max({ p0.y, p1.y, p2.y }),
				std::max({ p0.z, p1.z, p2.z })
			);

			Vec3 Out;

			float Denom0 = (maxPos.x - minPos.x);
			float Denom1 = (maxPos.y - minPos.y);
			float Denom2 = (maxPos.z - minPos.z);

			Denom0 = Denom0 + (FLOAT_LOWEST * (Denom0 == 0.0f));
			Denom1 = Denom0 + (FLOAT_LOWEST * (Denom0 == 0.0f));
			Denom2 = Denom0 + (FLOAT_LOWEST * (Denom0 == 0.0f));

			Out.x = (InterpedVal.x - minPos.x) / Denom0;
			Out.y = (InterpedVal.y - minPos.y) / Denom1;
			Out.z = (InterpedVal.z - minPos.z) / Denom2;

			Out.x = std::clamp(Out.x, 0.0f, 1.0f);
			Out.y = std::clamp(Out.y, 0.0f, 1.0f);
			Out.z = std::clamp(Out.z, 0.0f, 1.0f);


			return Out;
		}

		float MapInterpolatedfloatToGray(float p0, float p1, float p2, float Val)
		{
			float min = std::min({ p0, p1, p2 });
			float max = std::max({ p0, p1, p2 });


			return (Val - min) / (max - min);

		}
	}


	__inline bool ShouldCulltriangle(const Vec3& WorldSpacePos, Vec3 FaceNormal, const Vec3& CamPos)
	{
		//if ((TriNormal.Dot(WorldSpaceTri.Points[0] - Cam->Pos) < 0.0f) || !TerraPGE::DoCull || !MeshToRender->BackfaceCulling) // backface culling
		float facing = FaceNormal.Dot(WorldSpacePos - CamPos);
		return facing >= 0.0f;
	}


	__inline bool ShouldCulltriangle(const Vec3& ViewSpacePos, const Vec3& FaceNormal, const Matrix& ViewMatrix)
	{
		Vec3 viewPos = ViewSpacePos * ViewMatrix;
		Vec3 normalVS = FaceNormal * ViewMatrix;
		float facing = normalVS.Dot(viewPos);
		return facing >= 0.0f;
	}


	__inline Triangle TransformToWorld(const Triangle& Tri, const Matrix& Model)
	{
		Triangle out = Tri;
		out.ApplyMatrix(Model);

		for (int i = 0; i < 3; ++i)
			out.WorldSpaceVerts.Points[i] = out.Points.Points[i];

		return out;
	}


	__inline Triangle ProjectToViewSpace(const Triangle* Tri, Camera* Cam)
	{
		// 3d Space -> Viewed Space
		Triangle Result = *Tri;
		Result.ApplyMatrix(Cam->GetViewMatrix());

		return Result;
	}


	std::vector<Triangle> Clipping(const Triangle* Tri, const int sx, const int sy)
	{
		Triangle Clipped[2];

		std::vector<Triangle> ListTris;
		ListTris.push_back(*Tri);
		int NewTris = 1;

		for (int p = 0; p < 4; p++)
		{
			int NewTrisToAdd = 0;
			while (NewTris > 0)
			{
				Triangle Test = ListTris.front();
				ListTris.erase(ListTris.begin());
				NewTris--;

				switch (p)
				{
					case 0:
					{
						NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
						break;
					}
					case 1:
					{
						NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
						break;
					}
					case 2:
					{
						NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
						break;
					}
					case 3:
					{
						NewTrisToAdd = Test.ClipAgainstPlane({ (float)sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Renderer::DebugClip);
						break;
					}
				}

				for (int w = 0; w < NewTrisToAdd; w++)
				{
					ListTris.push_back(Clipped[w]);
				}
			}
			NewTris = (int)ListTris.size();
		}

		return ListTris;
	}


	std::vector<Triangle> ClippingClipSpace(const Triangle* Tri)
	{
		std::vector<Triangle> Tris;
		Tris.push_back(*Tri);

		Vec4 planes[6] = {
			{  0,  0,  1,  0 }, // z >= 0
			{  1,  0,  0,  1 }, // x >= -w
			{ -1,  0,  0,  1 }, // x <=  w
			{  0,  1,  0,  1 }, // y >= -w
			{  0, -1,  0,  1 }, // y <=  w
			{  0,  0, -1,  1 }  // z <=  w
		};

		for (int p = 0; p < 6; ++p)
		{
			std::vector<Triangle> Next;

			for (const Triangle& T : Tris)
			{
				Triangle O1, O2;
				int count = T.ClipAgainstPlane(planes[p], O1, O2, DebugClip);

				if (count >= 1)
					Next.push_back(O1);
				if (count == 2)
					Next.push_back(O2);
			}

			Tris.swap(Next);

			if (Tris.empty())
				break;
		}

		return Tris;
	}


	std::vector<Triangle> VertexShader(Camera* Cam, Renderable* Object)
	{
		std::vector<Triangle> ClipSpaceTris = {};
		ClipSpaceTris.reserve(Object->mesh->Triangles.size());

		Vec3 NormPos = Vec3(0, 0, 0);
		Vec3 NormDir = Vec3(0, 0, 0);
		Vec3 TriNormal;

		static const float INV_3 = 1.0f / 3.0f;

		for (const Triangle& Tri : Object->mesh->Triangles)
		{
			// 3D Space / World Space
			Triangle WorldSpaceTri = TransformToWorld(Tri, Object->Transform.GetWorldMatrix());

			//TODO Fix this whole damn thing calc normals at mesh gen or at vertex shade time fuck the rest
			TriNormal = Tri.FaceNormal;
			if (Object->mesh->Normals.size() == 0)
			{
				NormPos = ((Tri.Points.Points[0] + Tri.Points.Points[1] + Tri.Points.Points[2]) * INV_3);
				NormDir = (Tri.Points.Points[1] - Tri.Points.Points[0]).GetVec3().CrossNormalized((Tri.Points.Points[2] - Tri.Points.Points[0])).Normalized();
				TriNormal = -(WorldSpaceTri.Points.Points[1] - WorldSpaceTri.Points.Points[0]).GetVec3().CrossNormalized((WorldSpaceTri.Points.Points[2] - WorldSpaceTri.Points.Points[0])).Normalized(); // this line and the if statement is used for culling

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= Object->Transform.Normal;
				}

				WorldSpaceTri.NormalPositions[0] = NormPos;
				WorldSpaceTri.NormalPositions[1] = Tri.Points.Points[0];
				WorldSpaceTri.NormalPositions[2] = Tri.Points.Points[1];
				WorldSpaceTri.NormalPositions[3] = Tri.Points.Points[2];
				WorldSpaceTri.NormDirections[0] = NormDir;
			}
			else
			{
				TriNormal *= Object->Transform.Normal;
				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= Object->Transform.Normal;
				}

				NormDir = WorldSpaceTri.FaceNormal;
				NormPos = WorldSpaceTri.NormalPositions[0];
			}

			if (!ShouldCulltriangle(WorldSpaceTri.Points.Points[0], TriNormal, Cam->GetWorldPosition()) || !DoCull) // backface culling
			{
				WorldSpaceTri.FaceNormal = TriNormal;
				Triangle ViewSpaceTri = ProjectToViewSpace(&WorldSpaceTri, Cam);

				ViewSpaceTri.ApplyMatrix(Cam->GetProjectionMatrix());

				ClipSpaceTris.push_back(ViewSpaceTri);
			}
		}
		return ClipSpaceTris;
	}


	template<typename T>
	void __fastcall BaryCentricRasterizer(float* FrameBuffer, float* DepthBuffer, const int ScreenWidth, const int ScreenHeight, float* ShadowMap, const int ShadowMapWidth, const int ShadowMapHeight, T&& Shader, ShaderArgs* BaseArgs1)
	{
		ShaderArgs* BaseArgs = DEBUG_NEW ShaderArgs(BaseArgs1);

		// Allocating now reduces copying
		BaseArgs->PrepareFragmentShader();

		Triangle* ScreenSpaceTri = BaseArgs->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		//Matrix Vp = BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX) * BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_PROJ_MATRIX);
		LightObject** Lights = BaseArgs->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);
		ShaderTypes* ShaderType = BaseArgs->FindShaderResourcePtr<ShaderTypes*>(TPGE_SHDR_TYPE);
		size_t LightCount = BaseArgs->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		bool HasLight = LightCount > 0;
		Color* FragmentColor = BaseArgs->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

		bool MidPixel = false;

		BaryCoords Interp;

		// Screen Space
		Vec4 v0 = ScreenSpaceTri->Points.Points[0];
		Vec4 v1 = ScreenSpaceTri->Points.Points[1];
		Vec4 v2 = ScreenSpaceTri->Points.Points[2];

		// Extract per-vertex attributes
		Vec3 uvw0 = ScreenSpaceTri->TexCoords[0].AsVec3(), uvw1 = ScreenSpaceTri->TexCoords[1].AsVec3(), uvw2 = ScreenSpaceTri->TexCoords[2].AsVec3();
		Vec3 n0 = ScreenSpaceTri->FaceNormal, n1 = ScreenSpaceTri->FaceNormal, n2 = ScreenSpaceTri->FaceNormal;

		Vec4 world0 = ScreenSpaceTri->WorldSpaceVerts.Points[0];
		Vec4 world1 = ScreenSpaceTri->WorldSpaceVerts.Points[1];
		Vec4 world2 = ScreenSpaceTri->WorldSpaceVerts.Points[2];

		const float v1y_Sub_v2y = v1.y - v2.y;
		const float v2y_Sub_v0y = v2.y - v0.y;
		const float v0x_Sub_v2x = v0.x - v2.x;
		const float v2x_Sub_v1x = v2.x - v1.x;

		// Triangle bounding box
		int minX = std::max(0, (int)std::floor(std::min({ v0.x, v1.x, v2.x })));
		int maxX = std::min(ScreenWidth - 1, (int)std::ceil(std::max({ v0.x, v1.x, v2.x })));
		int minY = std::max(0, (int)std::floor(std::min({ v0.y, v1.y, v2.y })));
		int maxY = std::min(ScreenHeight - 1, (int)std::ceil(std::max({ v0.y, v1.y, v2.y })));

		// Hacky way to tell tri winding
		float triArea = (v1y_Sub_v2y) * (v0x_Sub_v2x)+(v2.x - v1.x) * (v0.y - v2.y); 		// Signed area of tri in screen space
		bool ccw = triArea > 0.0f;
		if (triArea == 0.0f) return; // Degenerate triangle
		float invArea = 1 / triArea;

		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				int idx = ContIdx(x, y, ScreenWidth);
				MidPixel = (x * 2 == ScreenWidth && y * 2 == ScreenHeight);
				// Compute barycentric coordinates
				float alpha = (v1y_Sub_v2y * ((x + 0.5f) - v2.x) + (v2x_Sub_v1x) * ((y + 0.5f) - v2.y)) * invArea;
				float beta = (v2y_Sub_v0y * ((x + 0.5f) - v2.x) + (v0x_Sub_v2x) * ((y + 0.5f) - v2.y)) * invArea;
				float gamma = 1.0f - alpha - beta;

				//				if (triArea < 0.0f)
				//				{
				//					triArea = -triArea;
				//					alpha = -alpha;
				//					beta = -beta;
				//					gamma = -gamma;
				//				}

				if (alpha < 0 || beta < 0 || gamma < 0) continue;

				if (false)
				{
					Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, alpha, beta, gamma);
					continue;
				}

				Interp.SetBary(alpha, beta, gamma);
				float Depth = Interp.PerspectiveCorrectInterpolate(v0.z, v1.z, v2.z, v0.w, v1.w, v2.w);

				if (Depth < DepthBuffer[idx] || SkipDepthTesting)
				{
					DepthBuffer[idx] = Depth;

					if (DebugDepthBuffer)
					{
						if (MidPixel)
						{
							TestDepth = Depth;
						}

						Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
						continue;
					}

					float ShadowMapDepth = 0.0f;
					float ShadowDepth = 0.0f;

					Vec3 InterpolatedWorldPos;
					InterpolatedWorldPos.x = Interp.PerspectiveCorrectInterpolate(world0.x, world1.x, world2.x, v0.w, v1.w, v2.w);
					InterpolatedWorldPos.y = Interp.PerspectiveCorrectInterpolate(world0.y, world1.y, world2.y, v0.w, v1.w, v2.w);
					InterpolatedWorldPos.z = Interp.PerspectiveCorrectInterpolate(world0.z, world1.z, world2.z, v0.w, v1.w, v2.w);


					if (false)
					{
						Vec3 Color = RasterUtils::MapInterpedVec3ToColor(world0, world1, world2, InterpolatedWorldPos);
						Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, Color.x, Color.y, Color.z);
						continue;
					}

					if (Renderer::DoShadows && HasLight)
					{
						bool isInShadow = Lights[0]->SampleShadowMap(ShadowMap, ShadowMapWidth, ShadowMapHeight, InterpolatedWorldPos, &ShadowDepth, &ShadowMapDepth);

						if (MidPixel && (DebugShadows || DebugShadowValue || DebugShadows))
						{
							TestDepth = ShadowDepth;
							TestDepthMapped = ShadowMapDepth;
						}

						if (DebugShadowValue)
						{
							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, ShadowDepth, ShadowDepth, ShadowDepth);
							continue;
						}
						else if (DebugShadowMap)
						{
							ShadowMapDepth = std::clamp(ShadowMapDepth, 0.0f, 1.0f);
							float ColorVal = (ShadowMapDepth);
							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, ColorVal, ColorVal, ColorVal);
							continue;
						}

						if (isInShadow)
						{
							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);

							if (DebugShadows)
							{
								Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, 255.0f, 0.0f, 0.0f);
								continue;
							}
						}
						else
						{
							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
						}
					}

					if (WireFrame)
					{
						Vec3 BaryCoords = Vec3(alpha, beta, gamma);
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords);
						TerraPGE::EngineShaders::DebugShaders::Shader_WireFrame(BaseArgs);
						Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
						continue;
					}

					if (*ShaderType != ShaderTypes::SHADER_FRAGMENT || ScreenSpaceTri->OverrideTextureColor || !Renderer::DoLighting)
					{
						// Debug dispatching
						if (ScreenSpaceTri->OverrideTextureColor)
						{
							BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Col.x, ScreenSpaceTri->Col.y, ScreenSpaceTri->Col.z));
						}
						else if (ScreenSpaceTri->Material->HasUsableTexture())
						{
							Vec3 uvw;
							uvw.x = Interp.PerspectiveCorrectInterpolate(uvw0.x, uvw1.x, uvw2.x, v0.w, v1.w, v2.w);
							uvw.y = Interp.PerspectiveCorrectInterpolate(uvw0.y, uvw1.y, uvw2.y, v0.w, v1.w, v2.w);
							uvw.z = Interp.PerspectiveCorrectInterpolate(uvw0.z, uvw1.z, uvw2.z, v0.w, v1.w, v2.w);

							BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { uvw.x / uvw.z, uvw.y / uvw.z, uvw.z });
							EngineShaders::Shader_Sample_Texture(BaseArgs);
							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
							continue;
						}
						else
						{
							BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Material->AmbientColor.x, ScreenSpaceTri->Material->AmbientColor.y, ScreenSpaceTri->Material->AmbientColor.z));
						}
					}
					else
					{
						// Fragment Shader dispatch
						float w = uvw0.z;
						Vec3 uvw;
						uvw.x = Interp.PerspectiveCorrectInterpolate(uvw0.x, uvw1.x, uvw2.x, v0.w, v1.w, v2.w);
						uvw.y = Interp.PerspectiveCorrectInterpolate(uvw0.y, uvw1.y, uvw2.y, v0.w, v1.w, v2.w);
						uvw.z = Interp.PerspectiveCorrectInterpolate(uvw0.z, uvw1.z, uvw2.z, v0.w, v1.w, v2.w);

						// Set up some shader args and call fragment shader
						Vec3 BaryCoords = Vec3(alpha, beta, gamma);
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_POS, InterpolatedWorldPos);
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, ScreenSpaceTri->FaceNormal);
						BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { uvw.x / uvw.z, uvw.y / uvw.z, uvw.z });
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords);
						BaseArgs->EditShaderDataValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)x, (float)y));
						Shader(BaseArgs);
					}

					// Set pixel in pixel buffer
					if (!UseHDR)
					{
						FragmentColor->R = std::clamp<float>(FragmentColor->R, 0.0f, 1.0f);
						FragmentColor->G = std::clamp<float>(FragmentColor->G, 0.0f, 1.0f);
						FragmentColor->B = std::clamp<float>(FragmentColor->B, 0.0f, 1.0f);
					}

					Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);

					//COLORREF PixelClr = RGB(PixelRoundFloor(FragmentColor->R), PixelRoundFloor(FragmentColor->G), PixelRoundFloor(FragmentColor->B));
					//Gdi->QuickSetPixel(x, y, PixelClr);
				}
			}
		}

		BaseArgs->Delete();
	}


	// input tri is clip space
	void __fastcall BaryCentricRasterizerDepth(Triangle* ClipSpaceTri, float* Buffer, const SIZE_T BufferWidth, const SIZE_T BufferHeight)
	{
		float z0 = ClipSpaceTri->Points.Points[0].z;
		float z1 = ClipSpaceTri->Points.Points[1].z;
		float z2 = ClipSpaceTri->Points.Points[2].z;

		ClipSpaceTri->ApplyPerspectiveDivide();
		ClipSpaceTri->Scale(Vec3((float)((BufferWidth - 1) * 0.5f), (float)((BufferHeight - 1) * 0.5f), 1.0f));
		ClipSpaceTri->Translate(Vec3((float)((BufferWidth - 1) * 0.5f), (float)((BufferHeight - 1) * 0.5f), 0.0f));

		// Screen Space
		Vec4 v0 = ClipSpaceTri->Points.Points[0];
		Vec4 v1 = ClipSpaceTri->Points.Points[1];
		Vec4 v2 = ClipSpaceTri->Points.Points[2];

		const float v1y_Sub_v2y = v1.y - v2.y;
		const float v2y_Sub_v0y = v2.y - v0.y;
		const float v0x_Sub_v2x = v0.x - v2.x;
		const float v2x_Sub_v1x = v2.x - v1.x;

		BaryCoords Interp;

		// --- 2. Compute triangle bounding box ---
		int minX = std::max(0, (int)std::floor(std::min({ v0.x, v1.x, v2.x })));
		int maxX = std::min(BufferWidth - 1, (SIZE_T)std::ceil(std::max({ v0.x, v1.x, v2.x })));
		int minY = std::max(0, (int)std::floor(std::min({ v0.y, v1.y, v2.y })));
		int maxY = std::min(BufferHeight - 1, (SIZE_T)std::ceil(std::max({ v0.y, v1.y, v2.y })));

		float denom = (v1y_Sub_v2y) * (v0x_Sub_v2x)+(v2.x - v1.x) * (v0.y - v2.y);
		if (denom == 0.0f) return; // Degenerate triangle
		float invDenom = 1 / denom;

		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				// Compute barycentric coordinates
				float alpha = (v1y_Sub_v2y * ((x + 0.5f) - v2.x) + (v2x_Sub_v1x) * ((y + 0.5f) - v2.y)) * invDenom;
				float beta = (v2y_Sub_v0y * ((x + 0.5f) - v2.x) + (v0x_Sub_v2x) * ((y + 0.5f) - v2.y)) * invDenom;
				float gamma = 1.0f - alpha - beta;

				if (alpha < 0 || beta < 0 || gamma < 0) continue;

				Interp.SetBary(alpha, beta, gamma);

				// this is correct for non ortho projection TODO make a switch for this in my main renderer and here 
				float Depth = Interp.PerspectiveCorrectInterpolate(v0.z, v1.z, v2.z, v0.w, v1.w, v2.w);

				if (Depth < 0.0f)
					continue;

				int idx = ContIdx(x, y, BufferWidth);

				if (Depth < Buffer[idx])
				{
					Buffer[idx] = Depth;
				}
			}
		}
	}


	namespace Multithreaded
	{
		struct RasterizerTile
		{
			float* Dst;
			int DstWidth;
			int MinY;
			int MaxY;
			int MinX;
			int MaxX;
		};


		void __inline RasterizeDepthTile(RasterizerTile Tile, const Vec4 v0, const Vec4 v1, const Vec4 v2, const float v1y_Sub_v2y, const float v2y_Sub_v0y, const float v0x_Sub_v2x, const float v2x_Sub_v1x, const float invDenom)
		{
			for (int y = Tile.MinY; y <= Tile.MaxY; y++)
			{
				for (int x = Tile.MinX; x <= Tile.MaxX; x++)
				{
					BaryCoords Interp;

					// Compute barycentric coordinates
					float alpha = (v1y_Sub_v2y * ((x + 0.5f) - v2.x) + (v2x_Sub_v1x) * ((y + 0.5f) - v2.y)) * invDenom;
					float beta = (v2y_Sub_v0y * ((x + 0.5f) - v2.x) + (v0x_Sub_v2x) * ((y + 0.5f) - v2.y)) * invDenom;
					float gamma = 1.0f - alpha - beta;

					if (alpha < 0 || beta < 0 || gamma < 0) continue;

					Interp.SetBary(alpha, beta, gamma);

					// this is correct for non ortho projection TODO make a switch for this in my main renderer and here 
					float Depth = Interp.PerspectiveCorrectInterpolate(v0.z, v1.z, v2.z, v0.w, v1.w, v2.w);

					if (Depth < 0.0f)
						continue;

					int idx = ContIdx(x, y, Tile.DstWidth);

					if (Depth < Tile.Dst[idx])
					{
						Tile.Dst[idx] = Depth;
					}
				}
			}
		}


		// not worth the overhead
		void __fastcall BaryCentricRasterizerDepth(Triangle* ClipSpaceTri, float* Buffer, const SIZE_T BufferWidth, const SIZE_T BufferHeight)
		{
			float z0 = ClipSpaceTri->Points.Points[0].z;
			float z1 = ClipSpaceTri->Points.Points[1].z;
			float z2 = ClipSpaceTri->Points.Points[2].z;

			ClipSpaceTri->ApplyPerspectiveDivide();
			ClipSpaceTri->Scale(Vec3((float)((BufferWidth - 1) * 0.5f), (float)((BufferHeight - 1) * 0.5f), 1.0f));
			ClipSpaceTri->Translate(Vec3((float)((BufferWidth - 1) * 0.5f), (float)((BufferHeight - 1) * 0.5f), 0.0f));

			// Screen Space
			Vec4 v0 = ClipSpaceTri->Points.Points[0];
			Vec4 v1 = ClipSpaceTri->Points.Points[1];
			Vec4 v2 = ClipSpaceTri->Points.Points[2];

			const float v1y_Sub_v2y = v1.y - v2.y;
			const float v2y_Sub_v0y = v2.y - v0.y;
			const float v0x_Sub_v2x = v0.x - v2.x;
			const float v2x_Sub_v1x = v2.x - v1.x;

			BaryCoords Interp;

			// --- 2. Compute triangle bounding box ---
			int minX = std::max(0, (int)std::floor(std::min({ v0.x, v1.x, v2.x })));
			int maxX = std::min(BufferWidth - 1, (SIZE_T)std::ceil(std::max({ v0.x, v1.x, v2.x })));
			int minY = std::max(0, (int)std::floor(std::min({ v0.y, v1.y, v2.y })));
			int maxY = std::min(BufferHeight - 1, (SIZE_T)std::ceil(std::max({ v0.y, v1.y, v2.y })));

			const float denom = (v1y_Sub_v2y) * (v0x_Sub_v2x)+(v2.x - v1.x) * (v0.y - v2.y);
			if (denom == 0.0f) return; // Degenerate triangle

			const float invDenom = 1/denom;

			const int tileSize = 32;

			for (int tileY = minY; tileY <= maxY; tileY += tileSize)
			{
				for (int tileX = minX; tileX <= maxX; tileX += tileSize)
				{
					int tileMaxY = std::min(tileY + tileSize - 1, maxY);
					int tileMaxX = std::min(tileX + tileSize - 1, maxX);
					RasterizerTile Tile{};
					Tile.Dst = Buffer;
					Tile.DstWidth = BufferWidth;
					Tile.MinX = tileX;
					Tile.MaxX = tileMaxX;
					Tile.MinY = tileY;
					Tile.MaxY = tileMaxY;



					Core::ThreadPool.EnqueueTask([Tile, &v2, &v1y_Sub_v2y, &v2y_Sub_v0y, &v0x_Sub_v2x, &v2x_Sub_v1x, &v0, &v1, &invDenom]()
						{
							RasterizeDepthTile(Tile, v0, v1, v2, v1y_Sub_v2y, v2y_Sub_v0y, v0x_Sub_v2x, v2x_Sub_v1x, invDenom);
						});
				}
			}

			Core::ThreadPool.WaitUntilAllTasksFinished();
		}
	

		/*
		void __inline RasterizeTile(RasterizerTile Tile, const Vec4 v0, const Vec4 v1, const Vec4 v2, const float v1y_Sub_v2y, const float v2y_Sub_v0y, const float v0x_Sub_v2x, const float v2x_Sub_v1x, const float invArea)
		{
			bool MidPixel = false;

			for (int y = Tile.MinY; y <= Tile.MaxY; y++)
			{
				for (int x = Tile.MinX; x <= Tile.MaxX; x++)
				{
					int idx = ContIdx(x, y, Tile.DstWidth);
					MidPixel = (x * 2 == Tile.DstWidth && y * 2 == Tile.DstHeight);
					// Compute barycentric coordinates
					float alpha = (v1y_Sub_v2y * ((x + 0.5f) - v2.x) + (v2x_Sub_v1x) * ((y + 0.5f) - v2.y)) * invArea;
					float beta = (v2y_Sub_v0y * ((x + 0.5f) - v2.x) + (v0x_Sub_v2x) * ((y + 0.5f) - v2.y)) * invArea;
					float gamma = 1.0f - alpha - beta;

					//				if (triArea < 0.0f)
					//				{
					//					triArea = -triArea;
					//					alpha = -alpha;
					//					beta = -beta;
					//					gamma = -gamma;
					//				}

					if (alpha < 0 || beta < 0 || gamma < 0) continue;

					if (false)
					{
						Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, Tile.DstWidth, alpha, beta, gamma);
						continue;
					}

					Interp.SetBary(alpha, beta, gamma);
					float Depth = Interp.PerspectiveCorrectInterpolate(v0.z, v1.z, v2.z, v0.w, v1.w, v2.w);

					if (Depth < DepthBuffer[idx] || SkipDepthTesting)
					{
						DepthBuffer[idx] = Depth;

						if (DebugDepthBuffer)
						{
							if (MidPixel)
							{
								TestDepth = Depth;
							}

							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, Tile.DstWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
							continue;
						}

						float ShadowMapDepth = 0.0f;
						float ShadowDepth = 0.0f;

						Vec3 InterpolatedWorldPos;
						InterpolatedWorldPos.x = Interp.PerspectiveCorrectInterpolate(world0.x, world1.x, world2.x, v0.w, v1.w, v2.w);
						InterpolatedWorldPos.y = Interp.PerspectiveCorrectInterpolate(world0.y, world1.y, world2.y, v0.w, v1.w, v2.w);
						InterpolatedWorldPos.z = Interp.PerspectiveCorrectInterpolate(world0.z, world1.z, world2.z, v0.w, v1.w, v2.w);


						if (false)
						{
							Vec3 Color = RasterUtils::MapInterpedVec3ToColor(world0, world1, world2, InterpolatedWorldPos);
							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, Color.x, Color.y, Color.z);
							continue;
						}

						if (Renderer::DoShadows && HasLight)
						{
							bool isInShadow = Lights[0]->SampleShadowMap(ShadowMap, ShadowMapWidth, ShadowMapHeight, InterpolatedWorldPos, &ShadowDepth, &ShadowMapDepth);

							if (MidPixel && (DebugShadows || DebugShadowValue || DebugShadows))
							{
								TestDepth = ShadowDepth;
								TestDepthMapped = ShadowMapDepth;
							}

							if (DebugShadowValue)
							{
								Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, ShadowDepth, ShadowDepth, ShadowDepth);
								continue;
							}
							else if (DebugShadowMap)
							{
								ShadowMapDepth = std::clamp(ShadowMapDepth, 0.0f, 1.0f);
								float ColorVal = (ShadowMapDepth);
								Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, ColorVal, ColorVal, ColorVal);
								continue;
							}

							if (isInShadow)
							{
								BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);

								if (DebugShadows)
								{
									Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, 255.0f, 0.0f, 0.0f);
									continue;
								}
							}
							else
							{
								BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
							}
						}

						if (WireFrame)
						{
							Vec3 BaryCoords = Vec3(alpha, beta, gamma);
							BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords);
							TerraPGE::EngineShaders::DebugShaders::Shader_WireFrame(BaseArgs);
							Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
							continue;
						}

						if (*ShaderType != ShaderTypes::SHADER_FRAGMENT || ScreenSpaceTri->OverrideTextureColor || !Renderer::DoLighting)
						{
							// Debug dispatching
							if (ScreenSpaceTri->OverrideTextureColor)
							{
								BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Col.x, ScreenSpaceTri->Col.y, ScreenSpaceTri->Col.z));
							}
							else if (ScreenSpaceTri->Material->HasUsableTexture())
							{
								Vec3 uvw;
								uvw.x = Interp.PerspectiveCorrectInterpolate(uvw0.x, uvw1.x, uvw2.x, v0.w, v1.w, v2.w);
								uvw.y = Interp.PerspectiveCorrectInterpolate(uvw0.y, uvw1.y, uvw2.y, v0.w, v1.w, v2.w);
								uvw.z = Interp.PerspectiveCorrectInterpolate(uvw0.z, uvw1.z, uvw2.z, v0.w, v1.w, v2.w);

								BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { uvw.x / uvw.z, uvw.y / uvw.z, uvw.z });
								EngineShaders::Shader_Sample_Texture(BaseArgs);
								Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);
								continue;
							}
							else
							{
								BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Material->AmbientColor.x, ScreenSpaceTri->Material->AmbientColor.y, ScreenSpaceTri->Material->AmbientColor.z));
							}
						}
						else
						{
							// Fragment Shader dispatch
							float w = uvw0.z;
							Vec3 uvw;
							uvw.x = Interp.PerspectiveCorrectInterpolate(uvw0.x, uvw1.x, uvw2.x, v0.w, v1.w, v2.w);
							uvw.y = Interp.PerspectiveCorrectInterpolate(uvw0.y, uvw1.y, uvw2.y, v0.w, v1.w, v2.w);
							uvw.z = Interp.PerspectiveCorrectInterpolate(uvw0.z, uvw1.z, uvw2.z, v0.w, v1.w, v2.w);

							// Set up some shader args and call fragment shader
							Vec3 BaryCoords = Vec3(alpha, beta, gamma);
							BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_POS, InterpolatedWorldPos);
							BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, ScreenSpaceTri->FaceNormal);
							BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { uvw.x / uvw.z, uvw.y / uvw.z, uvw.z });
							BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords);
							BaseArgs->EditShaderDataValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)x, (float)y));
							Shader(BaseArgs);
						}

						// Set pixel in pixel buffer
						if (!UseHDR)
						{
							FragmentColor->R = std::clamp<float>(FragmentColor->R, 0.0f, 1.0f);
							FragmentColor->G = std::clamp<float>(FragmentColor->G, 0.0f, 1.0f);
							FragmentColor->B = std::clamp<float>(FragmentColor->B, 0.0f, 1.0f);
						}

						Renderer::RenderingCore::SetPixelFrameBuffer(FrameBuffer, x, y, ScreenWidth, FragmentColor->R, FragmentColor->G, FragmentColor->B);

						//COLORREF PixelClr = RGB(PixelRoundFloor(FragmentColor->R), PixelRoundFloor(FragmentColor->G), PixelRoundFloor(FragmentColor->B));
						//Gdi->QuickSetPixel(x, y, PixelClr);
					}
				}
			}
		}



		template<typename T>
		void __fastcall BaryCentricRasterizer(float* FrameBuffer, float* DepthBuffer, const int ScreenWidth, const int ScreenHeight, float* ShadowMap, const int ShadowMapWidth, const int ShadowMapHeight, T&& Shader, ShaderArgs* BaseArgs1)
		{
			ShaderArgs* BaseArgs = DEBUG_NEW ShaderArgs(BaseArgs1);

			// Allocating now reduces copying
			BaseArgs->PrepareFragmentShader();

			Triangle* ScreenSpaceTri = BaseArgs->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
			//Matrix Vp = BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX) * BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_PROJ_MATRIX);
			LightObject** Lights = BaseArgs->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);
			ShaderTypes* ShaderType = BaseArgs->FindShaderResourcePtr<ShaderTypes*>(TPGE_SHDR_TYPE);
			size_t LightCount = BaseArgs->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
			bool HasLight = LightCount > 0;
			Color* FragmentColor = BaseArgs->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

			bool MidPixel = false;

			BaryCoords Interp;

			// Screen Space
			Vec4 v0 = ScreenSpaceTri->Points.Points[0];
			Vec4 v1 = ScreenSpaceTri->Points.Points[1];
			Vec4 v2 = ScreenSpaceTri->Points.Points[2];

			// Extract per-vertex attributes
			Vec3 uvw0 = ScreenSpaceTri->TexCoords[0].AsVec3(), uvw1 = ScreenSpaceTri->TexCoords[1].AsVec3(), uvw2 = ScreenSpaceTri->TexCoords[2].AsVec3();
			Vec3 n0 = ScreenSpaceTri->FaceNormal, n1 = ScreenSpaceTri->FaceNormal, n2 = ScreenSpaceTri->FaceNormal;

			Vec4 world0 = ScreenSpaceTri->WorldSpaceVerts.Points[0];
			Vec4 world1 = ScreenSpaceTri->WorldSpaceVerts.Points[1];
			Vec4 world2 = ScreenSpaceTri->WorldSpaceVerts.Points[2];

			const float v1y_Sub_v2y = v1.y - v2.y;
			const float v2y_Sub_v0y = v2.y - v0.y;
			const float v0x_Sub_v2x = v0.x - v2.x;
			const float v2x_Sub_v1x = v2.x - v1.x;

			// Triangle bounding box
			int minX = std::max(0, (int)std::floor(std::min({ v0.x, v1.x, v2.x })));
			int maxX = std::min(ScreenWidth - 1, (int)std::ceil(std::max({ v0.x, v1.x, v2.x })));
			int minY = std::max(0, (int)std::floor(std::min({ v0.y, v1.y, v2.y })));
			int maxY = std::min(ScreenHeight - 1, (int)std::ceil(std::max({ v0.y, v1.y, v2.y })));

			// Hacky way to tell tri winding
			float triArea = (v1y_Sub_v2y) * (v0x_Sub_v2x)+(v2.x - v1.x) * (v0.y - v2.y); 		// Signed area of tri in screen space
			bool ccw = triArea > 0.0f;
			if (triArea == 0.0f) return; // Degenerate triangle
			float invArea = 1 / triArea;

			const int tileSize = 32;

			for (int tileY = minY; tileY <= maxY; tileY += tileSize)
			{
				for (int tileX = minX; tileX <= maxX; tileX += tileSize)
				{
					int tileMaxY = std::min(tileY + tileSize - 1, maxY);
					int tileMaxX = std::min(tileX + tileSize - 1, maxX);
					RasterizerTile Tile{};
					Tile.Dst = Buffer;
					Tile.DstWidth = BufferWidth;
					Tile.MinX = tileX;
					Tile.MaxX = tileMaxX;
					Tile.MinY = tileY;
					Tile.MaxY = tileMaxY;

					Core::ThreadPool.EnqueueTask([Tile, &v2, &v1y_Sub_v2y, &v2y_Sub_v0y, &v0x_Sub_v2x, &v2x_Sub_v1x, &v0, &v1, &invDenom]()
						{
							RasterizeDepthTile(Tile, v0, v1, v2, v1y_Sub_v2y, v2y_Sub_v0y, v0x_Sub_v2x, v2x_Sub_v1x, invDenom);
						});
				}
			}
			BaseArgs->Delete();
		}
		*/
	}
}