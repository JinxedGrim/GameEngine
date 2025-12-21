#pragma once
#include "EngineCore.h"

namespace TerraPGE::Renderer
{
	float ShadowMapBias = FLOAT_LOWEST;
	bool DebugDepthBuffer = false;
	bool DebugShadows = false;
	bool DebugShadowMap = false;
	bool SkipDepthTesting = false;
	float TestClipZ = 1.0f;
	float TestNdcZ = 1.0f;
	float TestClipW = 1.0f;
	float TestNdcW = 1.0f;
	float TestDepth = 1.0f;
	bool ConvertToSRGB = true;

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

		float InvW0;
		float InvW1;
		float InvW2;

		float LerpInvW;

		void AddBary(float A, float B, float G)
		{
			this->Alpha = A;
			this->Beta = B;
			this->Gamma = G;
		}

		void AddWFromTri(const Triangle* Tri)
		{
			this->InvW0 = 1 / Tri->Points[0].w;
			this->InvW1 = 1 / Tri->Points[1].w;
			this->InvW2 = 1 / Tri->Points[2].w;
		}

		void CallLerpedInvW()
		{
			this->LerpInvW = InvW0 * Alpha + InvW1 * Beta + InvW2 * Gamma;
		}

		float Interpolate(float Val0, float Val1, float Val2)
		{
			// Undivide && interpolate
			float vOverW =
				Val0 * this->InvW0 * this->Alpha +
				Val1 * this->InvW1 * this->Beta +
				Val2 * this->InvW2 * this->Gamma;

			// re-apply perspective
			return vOverW / this->LerpInvW;
		}

		Vec3 InterpolateVec3(const Vec3& V0, const Vec3& V1, const Vec3& V2)
		{
			// Undivide && interpolate
			float vXOverW =
				(V0.x * this->InvW0 * this->Alpha +
				V0.x * this->InvW1 * this->Beta +
				V0.x * this->InvW2 * this->Gamma) / LerpInvW;

			float vYOverW =
				(V1.y * this->InvW0 * this->Alpha +
				V1.y * this->InvW1 * this->Beta +
				V1.y * this->InvW2 * this->Gamma) / LerpInvW;

			float vZOverW =
				(V2.z * this->InvW0 * this->Alpha +
				V2.z * this->InvW1 * this->Beta +
				V2.z * this->InvW2 * this->Gamma) / LerpInvW;

			// re-apply perspective
			return Vec3(vXOverW, vYOverW, vZOverW);
		}
	};


	template<typename T>
	void __fastcall BaryCentricRasterizer(float* DepthBuffer, int ScreenWidth, int ScreenHeight, GdiPP* Gdi, T&& Shader, ShaderArgs* BaseArgs1)
	{
		ShaderArgs* BaseArgs = DEBUG_NEW ShaderArgs(BaseArgs1);

		// Allocating now reduces copying
		BaseArgs->PrepareFragmentShader();

		Triangle* ScreenSpaceTri = BaseArgs->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		float FarSubNear = Core::FFAR - Core::FNEAR;
		float FarNear = Core::FFAR * Core::FNEAR;
		//Matrix Vp = BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX) * BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_PROJ_MATRIX);
		LightObject** Lights = BaseArgs->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);
		ShaderTypes* ShaderType = BaseArgs->FindShaderResourcePtr<ShaderTypes*>(TPGE_SHDR_TYPE);
		size_t LightCount = BaseArgs->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
		bool HasLight = LightCount > 0;

		BaryCoords Interp;
		Interp.AddWFromTri(ScreenSpaceTri);

		// Screen Space
		Vec4 v0 = ScreenSpaceTri->Points[0];
		Vec4 v1 = ScreenSpaceTri->Points[1];
		Vec4 v2 = ScreenSpaceTri->Points[2];

		// Extract per-vertex attributes
		Vec3 uvw0 = ScreenSpaceTri->TexCoords[0].AsVec3(), uvw1 = ScreenSpaceTri->TexCoords[1].AsVec3(), uvw2 = ScreenSpaceTri->TexCoords[2].AsVec3();
		Vec3 n0 = ScreenSpaceTri->FaceNormal, n1 = ScreenSpaceTri->FaceNormal, n2 = ScreenSpaceTri->FaceNormal;

		Vec4 world0 = ScreenSpaceTri->WorldSpaceVerts[0];
		Vec4 world1 = ScreenSpaceTri->WorldSpaceVerts[1];
		Vec4 world2 = ScreenSpaceTri->WorldSpaceVerts[2];

		// --- 2. Compute triangle bounding box ---
		int minX = std::max(0, (int)std::floor(std::min({ v0.x, v1.x, v2.x })));
		int maxX = std::min(Core::sx - 1, (int)std::ceil(std::max({ v0.x, v1.x, v2.x })));
		int minY = std::max(0, (int)std::floor(std::min({ v0.y, v1.y, v2.y })));
		int maxY = std::min(Core::sy - 1, (int)std::ceil(std::max({ v0.y, v1.y, v2.y })));

		float denom = (v1.y - v2.y) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.y - v2.y);
		if (denom == 0.0f) return; // Degenerate triangle

		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				// Compute barycentric coordinates
				float alpha = ((v1.y - v2.y) * (x - v2.x) + (v2.x - v1.x) * (y - v2.y)) / denom;
				float beta = ((v2.y - v0.y) * (x - v2.x) + (v0.x - v2.x) * (y - v2.y)) / denom;
				float gamma = 1.0f - alpha - beta;

				// --- Check if pixel is inside triangle ---
				if (alpha < 0 || beta < 0 || gamma < 0)
					continue;

				Interp.AddBary(alpha, beta, gamma);
				Interp.CallLerpedInvW();

				float Depth = Interp.Interpolate(v0.z, v1.z, v2.z);

				//float Depth1 = RemapNonLinearDepth(Depth, TerraPGE::Core::FNEAR, TerraPGE::Core::FFAR);

				if (x == ScreenWidth / 2 && y == ScreenHeight / 2)
				{
					TestDepth = Depth;
					TestNdcZ = Depth;
					TestClipW = ScreenSpaceTri->ClipSpaceVerts[0].w;
				}

				// depth test (assumes DepthBuffer init = 1.0f and smaller means closer)
				int idx = y * Core::sx + x;
				if (Depth < DepthBuffer[idx] || SkipDepthTesting)
				{
					Vec3 InterpolatedWorldPos = Interp.InterpolateVec3(world0, world1, world2);

					DepthBuffer[idx] = Depth;

					float w = uvw0.z;
					Vec3 uvw = Interp.InterpolateVec3(uvw0, uvw1, uvw2);
					//uvw.w = w;

					Vec3 InterpolatedNormal = Interp.InterpolateVec3(n0, n1, n2);
					InterpolatedNormal.Normalize();

					// Shade
					float ShadowValue = 0.0f;
					float ShadowDepth = 0.0f;
					int MapIdx = 0;

					if (Core::DoShadows && HasLight)
					{
						// do for all lights
						//for (int lightIdx = 0; lightIdx < LightCount; lightIdx++)
						//{
						//	Vec4 ShadowNdcPos = Lights[0]->CalcNdc(Vec4(InterpolatedPos, 1.0f));
						//	ShadowNdcPos.CorrectPerspective();

						//	ShadowNdcPos *= (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
						//	ShadowNdcPos += (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

						//	MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, Core::ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, Core::ShadowMapHeight - 1), Core::ShadowMapWidth);

						//	ShadowValue = Core::ShadowMap[MapIdx] + ShadowMapBias;
						//	ShadowDepth = (((FarNear / (Core::FFAR - ShadowNdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

						//	if (ShadowDepth > ShadowValue)
						//	{
						//		BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);
						//	}
						//	else
						//	{
						//		BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
						//	}
						//}
					}

					// Debug depth buffer (Grayscale the pixel * depth val)
					if (DebugDepthBuffer || DebugShadowMap)
					{
						// I plan to add a way to visualize the shadow map here
						float Val = Depth;
						if (DebugShadowMap)
						{
							if (ShadowDepth > ShadowValue)
							{
								Val = 0.3f;
							}
							else
							{
								Val = Core::ShadowMap[MapIdx];
							}
						}

						float ColorVal = (255.0f * Val);

						ColorVal = std::clamp(ColorVal, 0.0f, 255.0f);
						
						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ColorVal, ColorVal, ColorVal));
					}
					else if (*ShaderType != ShaderTypes::SHADER_FRAGMENT || ScreenSpaceTri->OverrideTextureColor || !Core::DoLighting)
					{
						// This entire else if is mainly for debugging clipping
						if (ScreenSpaceTri->OverrideTextureColor)
						{
							BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Col.x, ScreenSpaceTri->Col.y, ScreenSpaceTri->Col.z));
						}
						else if (ScreenSpaceTri->Material->HasUsableTexture())
						{
							Vec3 TexturCol = ScreenSpaceTri->Material->Textures.at(0)->GetPixelColor(uvw.x, 1.0f - uvw.y).GetRGB();
							BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(TexturCol.x, TexturCol.y, TexturCol.z));
						}
						else
						{
							BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(ScreenSpaceTri->Material->AmbientColor.x, ScreenSpaceTri->Material->AmbientColor.y, ScreenSpaceTri->Material->AmbientColor.z));
						}
					}
					else
					{
						// Set up some shader args and call fragment shader=
						Vec3 BaryCoords = Vec3(alpha, beta, gamma);
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_POS, InterpolatedWorldPos);
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, InterpolatedNormal);
						BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { uvw.x / uvw.z, uvw.y / uvw.z, uvw.z });
						BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords);
						BaseArgs->EditShaderDataValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)x, (float)y));
						Shader(BaseArgs);
					}

					// Set pixel in pixel buffer
					Color* FragmentColor = BaseArgs->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);

					if (ConvertToSRGB)
					{
						FragmentColor->Normalize();
						FragmentColor->TosRGB();
						FragmentColor->Denormalize();
					}

					COLORREF PixelClr = RGB(PixelRoundFloor(FragmentColor->R), PixelRoundFloor(FragmentColor->G), PixelRoundFloor(FragmentColor->B));
					Gdi->QuickSetPixel(x, y, PixelClr);
				}
			}
		}

		BaseArgs->Delete();
	}


	void __fastcall BaryCentricRasterizerDepth(Triangle* ScreenSpaceTri, float* Buffer, const SIZE_T BufferWidth, const SIZE_T BufferHeight, const Matrix& Vp)
	{
		// Extract per-vertex attributes
		Vec3 uvw0 = ScreenSpaceTri->TexCoords[0].AsVec3(), uvw1 = ScreenSpaceTri->TexCoords[1].AsVec3(), uvw2 = ScreenSpaceTri->TexCoords[2].AsVec3();
		Vec3 n0 = ScreenSpaceTri->FaceNormal, n1 = ScreenSpaceTri->FaceNormal, n2 = ScreenSpaceTri->FaceNormal;

		// --- 2. Compute triangle bounding box ---
		int minX = std::max(0, (int)std::floor(std::min({ ScreenSpaceTri->Points[0].x, ScreenSpaceTri->Points[1].x, ScreenSpaceTri->Points[2].x})));
		int maxX = std::min(BufferWidth - 1, (SIZE_T)std::ceil(std::max({ ScreenSpaceTri->Points[0].x, ScreenSpaceTri->Points[1].x, ScreenSpaceTri->Points[2].x })));
		int minY = std::max(0, (int)std::floor(std::min({ ScreenSpaceTri->Points[0].y, ScreenSpaceTri->Points[1].y, ScreenSpaceTri->Points[2].y })));
		int maxY = std::min(BufferHeight - 1, (SIZE_T)std::ceil(std::max({ ScreenSpaceTri->Points[0].y, ScreenSpaceTri->Points[1].y, ScreenSpaceTri->Points[2].y })));

		float denom = (ScreenSpaceTri->Points[1].y - ScreenSpaceTri->Points[2].y) * (ScreenSpaceTri->Points[0].x - ScreenSpaceTri->Points[2].x) + (ScreenSpaceTri->Points[2].x - ScreenSpaceTri->Points[1].x) * (ScreenSpaceTri->Points[0].y - ScreenSpaceTri->Points[2].y);
		if (denom == 0.0f) return; // Degenerate triangle

		for (int y = minY; y <= maxY; y++)
		{
			for (int x = minX; x <= maxX; x++)
			{
				// Compute barycentric coordinates
				float alpha = ((ScreenSpaceTri->Points[1].y - ScreenSpaceTri->Points[2].y) * (x - ScreenSpaceTri->Points[2].x) + (ScreenSpaceTri->Points[2].x - ScreenSpaceTri->Points[1].x) * (y - ScreenSpaceTri->Points[2].y)) / denom;
				float beta = ((ScreenSpaceTri->Points[2].y - ScreenSpaceTri->Points[0].y) * (x - ScreenSpaceTri->Points[2].x) + (ScreenSpaceTri->Points[0].x - ScreenSpaceTri->Points[2].x) * (y - ScreenSpaceTri->Points[2].y)) / denom;
				float gamma = 1.0f - alpha - beta;

				// --- 5. Check if pixel is inside triangle ---
				if (alpha < 0 || beta < 0 || gamma < 0) continue;

				// --- 6. Interpolate depth ---
				float Depth = alpha * ScreenSpaceTri->Points[0].z + beta * ScreenSpaceTri->Points[1].z + gamma * ScreenSpaceTri->Points[2].z;

				int idx = y * BufferWidth + x;
				if (Depth < Buffer[idx])
				{
					Buffer[idx] = Depth;
				}
			}
		}
	}


	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	template<typename T>
	void __fastcall RasterizeTriangleThreaded(float* DepthBuffer, GdiPP& Gdi, T&& Shader, ShaderArgs* Args)
	{
		// TODO Maybe take in shader args as a base then create copies and only distribute the dependent vars as free on delete
		// Setup variables
		Triangle* Tri = Args->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
		float FarSubNear = Core::FFAR - Core::FNEAR;
		float FarNear = Core::FFAR * Core::FNEAR;
		Matrix Vp = Args->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX) * Args->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_PROJ_MATRIX);
		Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_COLOR, Vec3(), 0);
		LightObject** Lights = Args->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);
		ShaderTypes ShaderType = Args->FindShaderResourceValue<ShaderTypes>(TPGE_SHDR_TYPE);

		int x1 = PixelRound(Tri->Points[0].x);
		int y1 = PixelRound(Tri->Points[0].y);
		int x2 = PixelRound(Tri->Points[1].x);
		int y2 = PixelRound(Tri->Points[1].y);
		int x3 = PixelRound(Tri->Points[2].x);
		int y3 = PixelRound(Tri->Points[2].y);
		float u1 = 0.0f;
		float u2 = 0.0f;
		float u3 = 0.0f;
		float w1 = 0.0f;
		float w2 = 0.0f;
		float w3 = 0.0f;
		float v1 = 0.0f;
		float v2 = 0.0f;
		float v3 = 0.0f;

		if (Tri->Material->HasUsableTexture())
		{
			u1 = Tri->TexCoords[0].u;
			u2 = Tri->TexCoords[1].u;
			u3 = Tri->TexCoords[2].u;
			v1 = Tri->TexCoords[0].v;
			v2 = Tri->TexCoords[1].v;
			v3 = Tri->TexCoords[2].v;
			w1 = Tri->TexCoords[0].w;
			w2 = Tri->TexCoords[1].w;
			w3 = Tri->TexCoords[2].w;
		}

		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			du1_step = 0, dv1_step = 0,
			du2_step = 0, dv2_step = 0,
			dw1_step = 0, dw2_step = 0;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy2) du2_step = du2 / (float)abs(dy2);
		if (dy2) dv2_step = dv2 / (float)abs(dy2);
		if (dy2) dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (int)((float)(i - y1) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
					{
						for (int j = ax; j < bx; j++)
						{
							tex_u = (1.0f - t) * tex_su + t * tex_eu;
							tex_v = (1.0f - t) * tex_sv + t * tex_ev;
							tex_w = (1.0f - t) * tex_sw + t * tex_ew;
							t += tstep;

							int idx = ContIdx(j, i, Core::sx);

							// Calculate the barycentric coordinates which we use for interpolation
							Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
							// Use Barycentric coords to interpolate our world position
							Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

							// World Pos -> Clipped Space
							Vec4 NdcPos = InterpolatedPos * Vp;
							// Clipped -> Ndc
							NdcPos.CorrectPerspective();

							// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
							// this line also normalizes that value to [0, 1]
							float Depth = (((FarNear / (Core::FFAR - NdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							// Depth test
							if (Depth < DepthBuffer[idx])
								DepthBuffer[idx] = Depth;
							else
								continue; // Don't draw pixel

							float ShadowValue = 0.0f;
							float ShadowDepth = 0.0f;
							int MapIdx = 0;

							if (Core::DoShadows)
							{
								// Make this better TODO
								Vec4 ShadowNdcPos = InterpolatedPos * Lights[0]->VpMatrices[0];

								ShadowNdcPos.CorrectPerspective();

								ShadowNdcPos *= (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
								ShadowNdcPos += (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

								MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, Core::ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, Core::ShadowMapHeight - 1), Core::ShadowMapWidth);

								ShadowValue = Core::ShadowMap[MapIdx] + ShadowMapBias;
								ShadowDepth = (((FarNear / (Core::FFAR - ShadowNdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

								if (ShadowDepth > ShadowValue)
								{
									Args->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);
								}
								else
								{
									Args->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
								}
							}

							// Debug depth buffer (Grayscale the pixel * depth val)
							if (DebugDepthBuffer || DebugShadowMap)
							{
								// I plan to add a way to visualize the shadow map here
								float Val = Depth;
								if (DebugShadowMap)
								{
									if (ShadowDepth > ShadowValue)
									{
										Val = 0.3f;
									}
									else
									{
										Val = Core::ShadowMap[MapIdx];
									}
								}

								Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(255.0f * Val, 255.0f * Val, 255.0f * Val));
							}
							else if (ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !Core::DoLighting)
							{
								// This entire else if is mainly for debugging clipping
								if (Tri->OverrideTextureColor)
								{
									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Col.x, Tri->Col.y, Tri->Col.z));
								}
								else if (Tri->Material->HasUsableTexture())
								{
									Vec3 TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();

									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(TexturCol.x, TexturCol.y, TexturCol.z));
								}
								else
								{
									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Material->AmbientColor.x, Tri->Material->AmbientColor.y, Tri->Material->AmbientColor.z));
								}
							}
							else
							{
								Vec3 InterpolatedNormal = ((Tri->FaceNormal * BaryCoords.x) + (Tri->FaceNormal * BaryCoords.y) + (Tri->FaceNormal * BaryCoords.z)).Normalized();

								// Set up some shader args and call fragment shader=
								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_POS, InterpolatedPos, sizeof(void*));
								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, InterpolatedNormal, sizeof(void*));
								Args->AddShaderDataByValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { tex_u / tex_w, tex_v / tex_w, tex_w }, sizeof(void*));
								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords, sizeof(void*));
								Args->AddShaderDataByValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)j, (float)i), sizeof(void*));
								Shader(Args);
							}

							// Set pixel in pixel buffer
							COLORREF PixelClr = RGB(PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->R), PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->G), PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->B));
							Gdi.QuickSetPixel(j, i, PixelClr);
							// TODO Maybe delete new version of shader args here (if i make copies and they only delete selected vars we're fine?)
						}
					};
				Core::ThreadPool.EnqueueTask(Func);
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1) dax_step = dx1 / (float)abs(dy1);
		if (dy2) dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1) du1_step = du1 / (float)abs(dy1);
		if (dy1) dv1_step = dv1 / (float)abs(dy1);
		if (dy1) dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (int)((float)(i - y2) * dax_step);
				int bx = x1 + (int)((float)(i - y1) * dbx_step);

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				auto Func = [=, &Gdi]() mutable
					{
						for (int j = ax; j < bx; j++)
						{
							tex_u = (1.0f - t) * tex_su + t * tex_eu;
							tex_v = (1.0f - t) * tex_sv + t * tex_ev;
							tex_w = (1.0f - t) * tex_sw + t * tex_ew;
							t += tstep;

							int idx = ContIdx(j, i, Core::sx);

							// Calculate the barycentric coordinates which we use for interpolation
							Vec3 BaryCoords = CalculateBarycentricCoordinatesScreenSpace(Vec2((float)j, (float)i), Vec2((float)Tri->Points[0].x, (float)Tri->Points[0].y), Vec2((float)Tri->Points[1].x, (float)Tri->Points[1].y), Vec2((float)Tri->Points[2].x, (float)Tri->Points[2].y));
							// Use Barycentric coords to interpolate our world position
							Vec4 InterpolatedPos = Vec4((Tri->WorldSpaceVerts[0] * BaryCoords.x) + (Tri->WorldSpaceVerts[1] * BaryCoords.y) + (Tri->WorldSpaceVerts[2] * BaryCoords.z), 1.0f);

							// World Pos -> Clipped Space
							Vec4 NdcPos = InterpolatedPos * Vp;
							// Clipped -> Ndc
							NdcPos.CorrectPerspective();

							// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
							// this line also normalizes that value to [0, 1]
							float Depth = (((FarNear / (Core::FFAR - NdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

							// Depth test
							if (Depth < DepthBuffer[idx])
								DepthBuffer[idx] = Depth;
							else
								continue; // Don't draw pixel

							float ShadowValue = 0.0f;
							float ShadowDepth = 0.0f;
							int MapIdx = 0;

							if (Core::DoShadows)
							{
								Vec4 ShadowNdcPos = Lights[0]->CalcNdc(InterpolatedPos);
								ShadowNdcPos.CorrectPerspective();

								ShadowNdcPos *= (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
								ShadowNdcPos += (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

								MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, Core::ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, Core::ShadowMapHeight - 1), Core::ShadowMapWidth);

								ShadowValue = Core::ShadowMap[MapIdx] + ShadowMapBias;
								ShadowDepth = (((FarNear / (Core::FFAR - ShadowNdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

								if (ShadowDepth > ShadowValue)
								{
									Args->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);
								}
								else
								{
									Args->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
								}
							}

							// Debug depth buffer (Grayscale the pixel * depth val)
							if (DebugDepthBuffer || DebugShadowMap)
							{
								// I plan to add a way to visualize the shadow map here
								float Val = Depth;
								if (DebugShadowMap)
								{
									if (ShadowDepth > ShadowValue)
									{
										Val = 0.3f;
									}
									else
									{
										Val = Core::ShadowMap[MapIdx];
									}
								}

								Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(255.0f * Val, 255.0f * Val, 255.0f * Val));
							}
							else if (ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !Core::DoLighting)
							{
								// This entire else if is mainly for debugging clipping
								if (Tri->OverrideTextureColor)
								{
									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Col.x, Tri->Col.y, Tri->Col.z));
								}
								else if (Tri->Material->HasUsableTexture())
								{
									Vec3 TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(TexturCol.x, TexturCol.y, TexturCol.z));
								}
								else
								{
									Args->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Material->AmbientColor.x, Tri->Material->AmbientColor.y, Tri->Material->AmbientColor.z));
								}
							}
							else
							{
								Vec3 InterpolatedNormal = ((Tri->FaceNormal * BaryCoords.x) + (Tri->FaceNormal * BaryCoords.y) + (Tri->FaceNormal * BaryCoords.z)).Normalized();

								// Set up some shader args and call fragment shader=

								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_POS, InterpolatedPos, sizeof(void*));
								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, InterpolatedNormal, sizeof(void*));
								Args->AddShaderDataByValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { tex_u / tex_w, tex_v / tex_w, tex_w }, sizeof(void*));
								Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, BaryCoords, sizeof(void*));
								Args->AddShaderDataByValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)j, (float)i), sizeof(void*));
								Shader(Args);
							}

							// Set pixel in pixel buffer
							COLORREF PixelClr = RGB(PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->R), PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->G), PixelRoundFloor(Args->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR)->B));
							Gdi.QuickSetPixel(j, i, PixelClr);
						}
					};

				Core::ThreadPool.EnqueueTask(Func);
				Core::ThreadPool.EnqueueTask(Func);
			}
		}

		Core::ThreadPool.WaitUntilAllTasksFinished();
	}



	// Adapted from: https://www.avrfreaks.net/sites/default/files/triangles.c
	//template<typename T>
	//void __fastcall RasterizeTriangle(float* DepthBuffer, GdiPP* Gdi, T&& Shader, ShaderArgs* BaseArgs1)
	//{
	//	// TODO Maybe take in shader args as a base then create copies and only distribute the dependent vars as free on delete
	//	// Setup variables

	//	ShaderArgs* BaseArgs = DEBUG_NEW ShaderArgs(BaseArgs1);

	//	Triangle* Tri = BaseArgs->FindShaderResourcePtr<Triangle*>(TPGE_SHDR_TRI);
	//	float FarSubNear = Core::FFAR - Core::FNEAR;
	//	float FarNear = Core::FFAR * Core::FNEAR;
	//	Matrix Vp = BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX) * BaseArgs->FindShaderResourceValue<Matrix>(TPGE_SHDR_CAMERA_PROJ_MATRIX);
	//	LightObject** Lights = BaseArgs->FindShaderResourcePtr<LightObject**>(TPGE_SHDR_LIGHT_OBJECTS);
	//	ShaderTypes* ShaderType = BaseArgs->FindShaderResourcePtr<ShaderTypes*>(TPGE_SHDR_TYPE);
	//	size_t LightCount = BaseArgs->FindShaderResourceValue<size_t>(TPGE_SHDR_LIGHT_COUNT);
	//	bool HasLight = LightCount > 0;

	//	// Allocating now reduces copying
	//    BaseArgs->PrepareFragmentShader();

	//	int x1 = PixelRound(Tri->Points[0].x);
	//	int y1 = PixelRound(Tri->Points[0].y);
	//	int x2 = PixelRound(Tri->Points[1].x);
	//	int y2 = PixelRound(Tri->Points[1].y);
	//	int x3 = PixelRound(Tri->Points[2].x);
	//	int y3 = PixelRound(Tri->Points[2].y);
	//	float u1 = 0.0f;
	//	float u2 = 0.0f;
	//	float u3 = 0.0f;
	//	float v1 = 0.0f;
	//	float v2 = 0.0f;
	//	float v3 = 0.0f;
	//	float w1 = 0.0f;
	//	float w2 = 0.0f;
	//	float w3 = 0.0f;

	//	if (Tri->Material->HasUsableTexture())
	//	{
	//		u1 = Tri->TexCoords[0].u;
	//		u2 = Tri->TexCoords[1].u;
	//		u3 = Tri->TexCoords[2].u;
	//		v1 = Tri->TexCoords[0].v;
	//		v2 = Tri->TexCoords[1].v;
	//		v3 = Tri->TexCoords[2].v;
	//		w1 = Tri->TexCoords[0].w;
	//		w2 = Tri->TexCoords[1].w;
	//		w3 = Tri->TexCoords[2].w;
	//	}

	//	if (y2 < y1)
	//	{
	//		std::swap(y1, y2);
	//		std::swap(x1, x2);
	//		std::swap(u1, u2);
	//		std::swap(v1, v2);
	//		std::swap(w1, w2);
	//	}

	//	if (y3 < y1)
	//	{
	//		std::swap(y1, y3);
	//		std::swap(x1, x3);
	//		std::swap(u1, u3);
	//		std::swap(v1, v3);
	//		std::swap(w1, w3);
	//	}

	//	if (y3 < y2)
	//	{
	//		std::swap(y2, y3);
	//		std::swap(x2, x3);
	//		std::swap(u2, u3);
	//		std::swap(v2, v3);
	//		std::swap(w2, w3);
	//	}

	//	int dy1 = y2 - y1;
	//	int dx1 = x2 - x1;
	//	float dv1 = v2 - v1;
	//	float du1 = u2 - u1;
	//	float dw1 = w2 - w1;

	//	int dy2 = y3 - y1;
	//	int dx2 = x3 - x1;
	//	float dv2 = v3 - v1;
	//	float du2 = u3 - u1;
	//	float dw2 = w3 - w1;

	//	float tex_u, tex_v, tex_w;

	//	float dax_step = 0, dbx_step = 0,
	//		du1_step = 0, dv1_step = 0,
	//		du2_step = 0, dv2_step = 0,
	//		dw1_step = 0, dw2_step = 0;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	if (dy1) du1_step = du1 / (float)abs(dy1);
	//	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	//	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	//	if (dy2) du2_step = du2 / (float)abs(dy2);
	//	if (dy2) dv2_step = dv2 / (float)abs(dy2);
	//	if (dy2) dw2_step = dw2 / (float)abs(dy2);

	//	if (dy1)
	//	{
	//		for (int i = y1; i <= y2; i++)
	//		{
	//			int ax = x1 + (int)((float)(i - y1) * dax_step);
	//			int bx = x1 + (int)((float)(i - y1) * dbx_step);

	//			float tex_su = u1 + (float)(i - y1) * du1_step;
	//			float tex_sv = v1 + (float)(i - y1) * dv1_step;
	//			float tex_sw = w1 + (float)(i - y1) * dw1_step;

	//			float tex_eu = u1 + (float)(i - y1) * du2_step;
	//			float tex_ev = v1 + (float)(i - y1) * dv2_step;
	//			float tex_ew = w1 + (float)(i - y1) * dw2_step;

	//			if (ax > bx)
	//			{
	//				std::swap(ax, bx);
	//				std::swap(tex_su, tex_eu);
	//				std::swap(tex_sv, tex_ev);
	//				std::swap(tex_sw, tex_ew);
	//			}

	//			tex_u = tex_su;
	//			tex_v = tex_sv;
	//			tex_w = tex_sw;

	//			float tstep = 1.0f / ((float)(bx - ax));
	//			float t = 0.0f;

	//			for (int j = ax; j < bx; j++)
	//			{
	//				tex_u = (1.0f - t) * tex_su + t * tex_eu;
	//				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
	//				tex_w = (1.0f - t) * tex_sw + t * tex_ew;
	//				t += tstep;

	//				int idx = ContIdx(j, i, Core::sx);

	//				FragmentInfo FragInfo = CalculateFragmentInfo(Vec2(j, i), Tri, &Vp, FarNear, FarSubNear);

	//				// Depth test
	//				if (FragInfo.Depth < DepthBuffer[idx])
	//					DepthBuffer[idx] = FragInfo.Depth;
	//				else
	//					continue; // Don't draw pixel

	//				float ShadowValue = 0.0f;
	//				float ShadowDepth = 0.0f;
	//				int MapIdx = 0;

	//				if (Core::DoShadows && HasLight)
	//				{
	//					// do for all lights
	//					for (int lightIdx = 0; lightIdx < LightCount; lightIdx++)
	//					{
	//						Vec4 ShadowNdcPos = Lights[0]->CalcNdc(FragInfo.InterpolatedPos);
	//						ShadowNdcPos.CorrectPerspective();

	//						ShadowNdcPos *= (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
	//						ShadowNdcPos += (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

	//						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, Core::ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, Core::ShadowMapHeight - 1), Core::ShadowMapWidth);

	//						ShadowValue = Core::ShadowMap[MapIdx] + ShadowMapBias;
	//						ShadowDepth = (((FarNear / (Core::FFAR - ShadowNdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

	//						if (ShadowDepth > ShadowValue)
	//						{
	//							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);
	//						}
	//						else
	//						{
	//							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
	//						}
	//					}
	//				}

	//				// Debug depth buffer (Grayscale the pixel * depth val)
	//				if (DebugDepthBuffer || DebugShadowMap)
	//				{
	//					// I plan to add a way to visualize the shadow map here
	//					float Val = FragInfo.Depth;
	//					if (DebugShadowMap)
	//					{
	//						if (ShadowDepth > ShadowValue)
	//						{
	//							Val = 0.3f;
	//						}
	//						else
	//						{
	//							Val = Core::ShadowMap[MapIdx];
	//						}
	//					}

	//					BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(255.0f * Val, 255.0f * Val, 255.0f * Val));
	//				}
	//				else if (*ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !Core::DoLighting)
	//				{
	//					// This entire else if is mainly for debugging clipping
	//					if (Tri->OverrideTextureColor)
	//					{
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Col.x, Tri->Col.y, Tri->Col.z));
	//					}
	//					else if (Tri->Material->HasUsableTexture())
	//					{
	//						Vec3 TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(TexturCol.x, TexturCol.y, TexturCol.z));
	//					}
	//					else
	//					{
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Material->AmbientColor.x, Tri->Material->AmbientColor.y, Tri->Material->AmbientColor.z));
	//					}
	//				}
	//				else
	//				{
	//					Vec3 InterpolatedNormal = ((Tri->FaceNormal * FragInfo.BaryCoords.x) + (Tri->FaceNormal * FragInfo.BaryCoords.y) + (Tri->FaceNormal * FragInfo.BaryCoords.z)).Normalized();

	//					// Set up some shader args and call fragment shader=
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_POS, FragInfo.InterpolatedPos);
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, InterpolatedNormal);
	//					BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { tex_u / tex_w, tex_v / tex_w, tex_w });
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, FragInfo.BaryCoords);
	//					BaseArgs->EditShaderDataValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)j, (float)i));
	//					Shader(BaseArgs);
	//				}

	//				// Set pixel in pixel buffer
	//				Color* FragmentColor = BaseArgs->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);
	//				COLORREF PixelClr = RGB(PixelRoundFloor(FragmentColor->R), PixelRoundFloor(FragmentColor->G), PixelRoundFloor(FragmentColor->B));
	//				Gdi->QuickSetPixel(j, i, PixelClr);
	//			}
	//		}
	//	}

	//	dy1 = y3 - y2;
	//	dx1 = x3 - x2;
	//	dv1 = v3 - v2;
	//	du1 = u3 - u2;
	//	dw1 = w3 - w2;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	du1_step = 0, dv1_step = 0;
	//	if (dy1) du1_step = du1 / (float)abs(dy1);
	//	if (dy1) dv1_step = dv1 / (float)abs(dy1);
	//	if (dy1) dw1_step = dw1 / (float)abs(dy1);

	//	if (dy1)
	//	{
	//		for (int i = y2; i <= y3; i++)
	//		{
	//			int ax = x2 + (int)((float)(i - y2) * dax_step);
	//			int bx = x1 + (int)((float)(i - y1) * dbx_step);

	//			float tex_su = u2 + (float)(i - y2) * du1_step;
	//			float tex_sv = v2 + (float)(i - y2) * dv1_step;
	//			float tex_sw = w2 + (float)(i - y2) * dw1_step;

	//			float tex_eu = u1 + (float)(i - y1) * du2_step;
	//			float tex_ev = v1 + (float)(i - y1) * dv2_step;
	//			float tex_ew = w1 + (float)(i - y1) * dw2_step;

	//			if (ax > bx)
	//			{
	//				std::swap(ax, bx);
	//				std::swap(tex_su, tex_eu);
	//				std::swap(tex_sv, tex_ev);
	//				std::swap(tex_sw, tex_ew);
	//			}

	//			tex_u = tex_su;
	//			tex_v = tex_sv;
	//			tex_w = tex_sw;

	//			float tstep = 1.0f / ((float)(bx - ax));
	//			float t = 0.0f;

	//			for (int j = ax; j < bx; j++)
	//			{
	//				tex_u = (1.0f - t) * tex_su + t * tex_eu;
	//				tex_v = (1.0f - t) * tex_sv + t * tex_ev;
	//				tex_w = (1.0f - t) * tex_sw + t * tex_ew;
	//				t += tstep;

	//				int idx = ContIdx(j, i, Core::sx);

	//				FragmentInfo FragInfo = CalculateFragmentInfo(Vec2(j, i), Tri, &Vp, FarNear, FarSubNear);
	//				
	//				// Depth test
	//				if (FragInfo.Depth < DepthBuffer[idx])
	//					DepthBuffer[idx] = FragInfo.Depth;
	//				else
	//					continue;

	//				float ShadowValue = 0.0f;
	//				float ShadowDepth = 0.0f;
	//				int MapIdx = 0;

	//				if (Core::DoShadows && HasLight)
	//				{
	//					// do for all lights
	//					for (int lightIdx = 0; lightIdx < LightCount; lightIdx++)
	//					{
	//						Vec4 ShadowNdcPos = Lights[0]->CalcNdc(FragInfo.InterpolatedPos);
	//						ShadowNdcPos.CorrectPerspective();

	//						ShadowNdcPos *= (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
	//						ShadowNdcPos += (Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

	//						MapIdx = ContIdx(PixelRoundMinMax(ShadowNdcPos.x, 0, Core::ShadowMapWidth - 1), PixelRoundMinMax(ShadowNdcPos.y, 0, Core::ShadowMapHeight - 1), Core::ShadowMapWidth);

	//						ShadowValue = Core::ShadowMap[MapIdx] + ShadowMapBias;
	//						ShadowDepth = (((FarNear / (Core::FFAR - ShadowNdcPos.z * FarSubNear) - Core::FNEAR) / FarSubNear) + 1.0f) / 2.0f;

	//						if (ShadowDepth > ShadowValue)
	//						{
	//							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, true);
	//						}
	//						else
	//						{
	//							BaseArgs->EditShaderDataValue(TPGE_SHDR_IS_IN_SHADOW, false);
	//						}
	//					}
	//				}

	//				// Debug depth buffer (Grayscale the pixel * depth val)
	//				if (DebugDepthBuffer || DebugShadowMap)
	//				{
	//					// I plan to add a way to visualize the shadow map here
	//					float Val = FragInfo.Depth;
	//					if (DebugShadowMap)
	//					{
	//						if (ShadowDepth > ShadowValue)
	//						{
	//							Val = 0.3f;
	//						}
	//						else
	//						{
	//							Val = Core::ShadowMap[MapIdx];
	//						}
	//					}

	//					BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(255.0f * Val, 255.0f * Val, 255.0f * Val));
	//				}

	//				else if (*ShaderType != ShaderTypes::SHADER_FRAGMENT || Tri->OverrideTextureColor || !Core::DoLighting)
	//				{
	//					// This entire else if is mainly for debugging clipping
	//					if (Tri->OverrideTextureColor)
	//					{
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Col.x, Tri->Col.y, Tri->Col.z));
	//					}
	//					else if (Tri->Material->HasUsableTexture())
	//					{
	//						Vec3 TexturCol = Tri->Material->Textures.at(0)->GetPixelColor(tex_u, 1.0f - tex_v).GetRGB();
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(TexturCol.x, TexturCol.y, TexturCol.z));
	//					}
	//					else
	//					{
	//						BaseArgs->EditShaderDataValue<Color>(TPGE_SHDR_FRAG_COLOR, Color(Tri->Material->AmbientColor.x, Tri->Material->AmbientColor.y, Tri->Material->AmbientColor.z));
	//					}
	//				}
	//				else
	//				{
	//					Vec3 InterpolatedNormal = ((Tri->FaceNormal * FragInfo.BaryCoords.x) + (Tri->FaceNormal * FragInfo.BaryCoords.y) + (Tri->FaceNormal * FragInfo.BaryCoords.z)).Normalized();

	//					// Set up some shader args and call fragment shader=
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_POS, FragInfo.InterpolatedPos);
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_NORMAL, InterpolatedNormal);
	//					BaseArgs->EditShaderDataValue<TextureCoords>(TPGE_SHDR_TEX_UVW, { tex_u / tex_w, tex_v / tex_w, tex_w });
	//					BaseArgs->EditShaderDataValue<Vec3>(TPGE_SHDR_FRAG_BARY_COORDS, FragInfo.BaryCoords);
	//					BaseArgs->EditShaderDataValue<Vec2>(TPGE_SHDR_PIXEL_COORDS, Vec2((float)j, (float)i));
	//					Shader(BaseArgs);
	//				}

	//				// Set pixel in pixel buffer
	//				Color* FragmentColor = BaseArgs->FindShaderResourcePtr<Color*>(TPGE_SHDR_FRAG_COLOR);
	//				COLORREF PixelClr = RGB(PixelRoundFloor(FragmentColor->R), PixelRoundFloor(FragmentColor->G), PixelRoundFloor(FragmentColor->B));
	//				Gdi->QuickSetPixel(j, i, PixelClr);
	//			}
	//		}
	//	}

	//	BaseArgs->Delete();
	//}


	//// Render process but writes to buffer only
	//void __fastcall RasterizeTriangleDepthToBuffer(Triangle* Tri, float* Buffer, SIZE_T BufferWidth, SIZE_T BufferHeight, const Matrix& ViewProjection)
	//{
	//	float FarSubNear = Core::FFAR - Core::FNEAR;
	//	float FarNear = Core::FFAR * Core::FNEAR;

	//	int x1 = PixelRound(Tri->Points[0].x);
	//	int y1 = PixelRound(Tri->Points[0].y);
	//	int x2 = PixelRound(Tri->Points[1].x);
	//	int y2 = PixelRound(Tri->Points[1].y);
	//	int x3 = PixelRound(Tri->Points[2].x);
	//	int y3 = PixelRound(Tri->Points[2].y);

	//	if (y2 < y1)
	//	{
	//		std::swap(y1, y2);
	//		std::swap(x1, x2);
	//	}

	//	if (y3 < y1)
	//	{
	//		std::swap(y1, y3);
	//		std::swap(x1, x3);
	//	}

	//	if (y3 < y2)
	//	{
	//		std::swap(y2, y3);
	//		std::swap(x2, x3);
	//	}

	//	int dy1 = y2 - y1;
	//	int dx1 = x2 - x1;

	//	int dy2 = y3 - y1;
	//	int dx2 = x3 - x1;

	//	float dax_step = 0, dbx_step = 0;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	if (dy1)
	//	{
	//		for (int i = y1; i <= y2; i++)
	//		{
	//			int ax = x1 + (int)((float)(i - y1) * dax_step);
	//			int bx = x1 + (int)((float)(i - y1) * dbx_step);

	//			if (ax > bx)
	//			{
	//				std::swap(ax, bx);
	//			}

	//			float tstep = 1.0f / ((float)(bx - ax));
	//			float t = 0.0f;

	//			for (int j = ax; j < bx; j++)
	//			{
	//				t += tstep;
	//				SIZE_T Idx = ContIdx(j, i, BufferWidth);

	//				FragmentInfo FragInfo = CalculateFragmentInfo(Vec2(j, i), Tri, &ViewProjection, FarNear, FarSubNear);

	//				// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
	//				// this line also normalizes that value to [0, 1]
	//				if (FragInfo.Depth < Buffer[Idx])
	//					Buffer[Idx] = FragInfo.Depth;
	//				else
	//					continue;
	//			}

	//		}
	//	}

	//	dy1 = y3 - y2;
	//	dx1 = x3 - x2;

	//	if (dy1) dax_step = dx1 / (float)abs(dy1);
	//	if (dy2) dbx_step = dx2 / (float)abs(dy2);

	//	if (dy1)
	//	{
	//		for (int i = y2; i <= y3; i++)
	//		{
	//			int ax = x2 + (int)((float)(i - y2) * dax_step);
	//			int bx = x1 + (int)((float)(i - y1) * dbx_step);

	//			if (ax > bx)
	//			{
	//				std::swap(ax, bx);
	//			}

	//			float tstep = 1.0f / ((float)(bx - ax));
	//			float t = 0.0f;

	//			for (int j = ax; j < bx; j++)
	//			{
	//				t += tstep;

	//				t += tstep;
	//				SIZE_T Idx = ContIdx(j, i, BufferWidth);

	//				FragmentInfo FragInfo = CalculateFragmentInfo(Vec2(j, i), Tri, &ViewProjection, FarNear, FarSubNear);

	//				// The NDC z component is depth this line linearizes that value as after perspective divide it is exponential
	//				// this line also normalizes that value to [0, 1]
	//				if (FragInfo.Depth < Buffer[Idx])
	//					Buffer[Idx] = FragInfo.Depth;
	//				else
	//					continue;
	//			}
	//		}
	//	}
	//}
};