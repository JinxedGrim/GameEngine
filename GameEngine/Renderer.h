#pragma once
#include "Rasterizer.h"
#include "Renderable.h"

namespace TerraPGE::Renderer
{
	enum class RenderingBackend
	{
		CPU = 0,
		MULTITHREADED = 1,
		GPU = 2
	};

	RenderingBackend CurrBackend = RenderingBackend::CPU;
	static GdiPP* EngineGdi = nullptr;

	BrushPP ClearBrush = -1;
	static const Vec3 PlaneNormal = { 0.0f, 0.0f, -1.0f };



	void PrepareRenderingBackend(WndCreator& Wnd)
	{
		switch (CurrBackend)
		{
		case RenderingBackend::CPU:
			EngineGdi = new GdiPP(Wnd.Wnd, true);
			Core::sx = Wnd.GetClientArea().Width;
			Core::sy = Wnd.GetClientArea().Height;

			Core::UpdateSystemInfo();

			delete[] Core::DepthBuffer;

			// Create depth buffer
			Core::DepthBuffer = DEBUG_NEW float[(SIZE_T)(Core::sx * Core::sy)];

			// Initial Client Region
			EngineGdi->UpdateClientRgn();
			break;
		default:
			std::cout << "Backend Not Found" << std::endl;
		}
	}


	void DispatchRenderingCall()
	{

	}


	void SetClearColor(int BrushIdx)
	{
		ClearBrush = (HBRUSH)GetStockObject(BrushIdx);
	}


	void ClearScreen()
	{
		// clear the screen

		if ((HBRUSH)ClearBrush == (HBRUSH)-1)
		{
			SetClearColor(BLACK_BRUSH);
		}

		Renderer::EngineGdi->Clear(GDIPP_PIXELCLEAR, ClearBrush);
		std::fill(Core::DepthBuffer, Core::DepthBuffer + Core::sx * Core::sy, 1.0f);
		std::fill(Core::ShadowMap, Core::ShadowMap + Core::ShadowMapWidth * Core::ShadowMapHeight, 1.0f);
	}


	void PrepareLights(LightObject** SceneLights, size_t LightCount)
	{
		// already do this? TODO
		for (int i = 0; i < LightCount; i++)
		{
			LightObject* Light = SceneLights[i];

			Light->CalcVpMats();
		}
	}


	__inline Triangle TransformToWorld(const Triangle& Tri, const Matrix& Model)
	{
		Triangle out = Tri;
		out.ApplyMatrix(Model);

		for (int i = 0; i < 3; ++i)
			out.WorldSpaceVerts[i] = out.Points[i];

		return out;
	}


	__inline void PorjectToViewSpace(Triangle* Tri, Camera* Cam)
	{
		// 3d Space -> Viewed Space
		Tri->ApplyMatrix(Cam->CalcCamViewMatrix());

		for (int i = 0; i < 3; i++)
		{
			Tri->ViewSpaceVerts[i] = Tri->Points[i];
		}
	}


	__inline bool ShouldCulltriangle(const Triangle& tri, const Camera* Cam)
	{
		//if ((TriNormal.Dot(WorldSpaceTri.Points[0] - Cam->Pos) < 0.0f) || !TerraPGE::DoCull || !MeshToRender->BackfaceCulling) // backface culling
		if (!Core::DoCull)
			return false;
		Vec3 normal = tri.FaceNormal;
		float facing = normal.Dot(tri.Points[0] - Cam->Transform.GetWorldPosition());
		return facing >= 0.0f;
	}


	std::vector<Triangle> VertexShader(Camera* Cam, Renderable* Object)
	{
		std::vector<Triangle> ClipSpaceTris = {};
		ClipSpaceTris.reserve(Object->mesh->Triangles.size());

		Triangle Clipped[2];
		Vec3 NormPos = Vec3(0, 0, 0);
		Vec3 NormDir = Vec3(0, 0, 0);
		Vec3 TriNormal;

		for (const Triangle& Tri : Object->mesh->Triangles)
		{

			// 3D Space / World Space
			Triangle WorldSpaceTri = TransformToWorld(Tri, Object->Transform.GetWorldMatrix());

			//TODO Fix this whole damn thing calc normals at mesh gen or at vertex shade time fuck the rest
			TriNormal = Tri.FaceNormal;
			if (Object->mesh->Normals.size() == 0)
			{
				NormPos = ((Tri.Points[0] + Tri.Points[1] + Tri.Points[2]) / 3.0f);
				NormDir = (Tri.Points[1] - Tri.Points[0]).GetVec3().CrossNormalized((Tri.Points[2] - Tri.Points[0])).Normalized();

				TriNormal = (WorldSpaceTri.Points[1] - WorldSpaceTri.Points[0]).GetVec3().CrossNormalized((WorldSpaceTri.Points[2] - WorldSpaceTri.Points[0])).Normalized(); // this line and the if statement is used for culling

				for (int i = 0; i < 3; i++)
				{
					WorldSpaceTri.VertexNormals[i] *= Object->Transform.Normal;
				}

				WorldSpaceTri.NormalPositions[0] = NormPos;
				WorldSpaceTri.NormalPositions[1] = Tri.Points[0];
				WorldSpaceTri.NormalPositions[2] = Tri.Points[1];
				WorldSpaceTri.NormalPositions[3] = Tri.Points[2];
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

			WorldSpaceTri.FaceNormal = TriNormal;



			if (!ShouldCulltriangle(WorldSpaceTri, Cam)) // backface culling
			{
				PorjectToViewSpace(&WorldSpaceTri, Cam);

				int Count = WorldSpaceTri.ClipAgainstPlane(Cam->NearPlane, PlaneNormal, Clipped[0], Clipped[1], Core::DebugClip);

				if (Count == 0)
					continue;

				for (int i = 0; i < Count; i++)
				{
					Triangle ClippedViewSpace = Clipped[i];

					// Viewed Space -> clip space
					Triangle ClipSpaceTri = Cam->ProjectTriangle(&ClippedViewSpace);

					for (int p = 0; p < 3; p++)
					{
						ClipSpaceTri.ClipSpaceVerts[p] = ClipSpaceTri.Points[p];
					}

					// Add Triangle to render list
					ClipSpaceTris.push_back(ClipSpaceTri);
				}
			}
		}
		return ClipSpaceTris;
	}


	std::vector<Triangle> Clipping(Triangle* Tri)
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
					NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, Clipped[0], Clipped[1], Core::DebugClip);
					break;
				}
				case 1:
				{
					NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, (float)Core::sy - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, Clipped[0], Clipped[1], Core::DebugClip);
					break;
				}
				case 2:
				{
					NewTrisToAdd = Test.ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Core::DebugClip);
					break;
				}
				case 3:
				{
					NewTrisToAdd = Test.ClipAgainstPlane({ (float)Core::sx - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, Clipped[0], Clipped[1], Core::DebugClip);
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


	// Render function for rendering entire meshes
	// Rendering Pipeline: Recieve call with mesh info including positions & lights sources -> Calc and apply matrices + normals -> backface culling	
	// -> Clipping + Frustum culling -> Call Draw Triangle from graphics API with shader supplied -> Shader called from triangle routine with pixel info + normals -> SetPixel
	//template<typename T>
	void RenderMeshes(Camera* Cam, Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount /*T&& Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT*/)
	{
		// TODO MultiThread??
		// TODO Allow vertex shaders (:
		// Project and translate object 
		PrepareLights(SceneLights, LightCount);

		for (int i = 0; i < ObjectCount; i++)
		{
			Renderable* Object = SceneObjects[i];

			std::vector<Triangle> ClipSpaceTris = VertexShader(Cam, Object);

			for (const Triangle& ClipSpaceTri : ClipSpaceTris)
			{
				// Clip Space -> NDC Space
				Triangle NdcSpaceTri = ClipSpaceTri;
				NdcSpaceTri.ApplyPerspectiveDivide();


				// Offset to viewport space (Ndc -> Screen Space)
				NdcSpaceTri.Scale(Vec3((float)(Core::sx * 0.5f), (float)(Core::sy * 0.5f), 1.0f));
				NdcSpaceTri.Translate(Vec3((float)(Core::sx * 0.5f), (float)(Core::sy * 0.5f), 0.0f));

				std::vector<Triangle> ClippedScreenSpace = Clipping(&NdcSpaceTri);

				// sort faces 
		//		std::sort(TrisToRender.begin(), TrisToRender.end(), [](const Triangle& t1, const Triangle& t2)
		//		{
		//			float z1 = (t1.Points[0].z + t1.Points[1].z + t1.Points[2].z) / 3.0f;
		//			float z2 = (t2.Points[0].z + t2.Points[1].z + t2.Points[2].z) / 3.0f;
		//			return z1 > z2;
		//		});


				ShaderArgs* Args = DEBUG_NEW ShaderArgs();
				Args->AddShaderDataByValue(TPGE_SHDR_TYPE, Object->SHADER_TYPE, 0);
				Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_CAMERA_POS, Cam->Transform.GetWorldPosition(), 0);
				Args->AddShaderDataByValue<Vec3>(TPGE_SHDR_CAMERA_LDIR, Cam->GetLookDirection(), 0);
				Args->AddShaderDataByValue<Matrix>(TPGE_SHDR_CAMERA_VIEW_MATRIX, Cam->CalcCamViewMatrix(), 0);
				Args->AddShaderDataPtr(TPGE_SHDR_CAMERA_PROJ_MATRIX, &Cam->ProjectionMatrix, 0);
				Args->AddShaderDataPtr(TPGE_SHDR_OBJ_MATRIX, &Object->Transform.World, 0);
				Args->AddShaderDataPtr(TPGE_SHDR_LIGHT_OBJECTS, SceneLights, 0);
				Args->AddShaderDataByValue<size_t>(TPGE_SHDR_LIGHT_COUNT, LightCount, 0);
				Args->AddShaderDataByValue<bool>(TPGE_SHDR_DEBUG_SHADOWS, DebugShadows);
				Args->AddShaderDataByValue<bool>(TPGE_SHDR_IS_IN_SHADOW, false);
				Args->AddShaderDataPtr(TPGE_SHDR_TRI, nullptr, 0);

				for (auto& ToDraw : ClippedScreenSpace)
				{
					//SceneLights, LightCount
					Args->EditShaderData(TPGE_SHDR_TRI, &ToDraw, 0);



					// Calc lighting (only if lighting is applied at a tri level)
					if ((Core::DoLighting) && Object->SHADER_TYPE == ShaderTypes::SHADER_TRIANGLE)
					{
						Object->Shader(Args);
					}

					if (!Core::WireFrame)
					{
						if (!Core::ShowTriLines)
						{
							if (Core::DoMultiThreading)
								Renderer::BaryCentricRasterizer(Core::DepthBuffer, Core::sx, Core::sy, EngineGdi, Object->Shader, Args);
							else
								Renderer::BaryCentricRasterizer(Core::DepthBuffer, Core::sx, Core::sy, EngineGdi, Object->Shader, Args);
						}
						else
						{
							if (Object->mesh->MeshName != "Ray")
							{
								EngineGdi->DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(1, 1, 1)));

								//if (ShowNormals)
								//{
								//	Ray NormalRay = Ray(ToDraw.NormalPositions[0], ToDraw.NormDirections[0]);
								//	NormalRay.GenerateMesh();

								//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

								//	NormalRay.mesh.UseSingleMat = true;

								//	NormalRay = Ray(ToDraw.NormalPositions[1], ToDraw.NormDirections[0]);
								//	NormalRay.GenerateMesh();
								//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

								//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

								//	NormalRay = Ray(ToDraw.NormalPositions[2], ToDraw.NormDirections[0]);
								//	NormalRay.GenerateMesh();
								//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

								//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);

								//	NormalRay = Ray(ToDraw.NormalPositions[3], ToDraw.NormDirections[0]);
								//	NormalRay.GenerateMesh();
								//	NormalRay.mesh.Mat.AmbientColor = Vec3(0, 0, 255);

								//	RenderMesh(Gdi, Cam, NormalRay.mesh, Scalar, RotationRads, Pos, Vec3(0, 0, 0), Vec3(1, 1, 1), 0.f, 0.f, 0.f, EngineShaders::Shader_Material);
								//}
							}
							else
							{
								EngineGdi->DrawFilledTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), BrushPP(RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
							}
						}
					}
					else
					{
						EngineGdi->DrawTriangle(PixelRound(ToDraw.Points[0].x), PixelRound(ToDraw.Points[0].y), PixelRound(ToDraw.Points[1].x), PixelRound(ToDraw.Points[1].y), PixelRound(ToDraw.Points[2].x), PixelRound(ToDraw.Points[2].y), PenPP(PS_SOLID, 1, RGB(ToDraw.Col.x, ToDraw.Col.y, ToDraw.Col.z)));
					}
				}

				Args->Delete();

			}

		}
	}


	void RenderShadowMaps(Renderable** SceneObjects, LightObject** SceneLights, size_t ObjectCount, size_t LightCount, float* Buffer)
	{
		std::vector<Triangle> TrisToRender = {};
		Triangle Clipped[2];
		Vec3 NormPos = Vec3(0, 0, 0);
		Vec3 NormDir = Vec3(0, 0, 0);
		Vec3 TriNormal;

		bool HasLight = LightCount > 1;

		// TODO MultiThread??
		// TODO Allow vertex shaders (:
		// Project and translate object 
		for (int i = 0; i < ObjectCount; i++)
		{
			Renderable* Object = SceneObjects[i];
			TrisToRender.reserve(Object->mesh->Triangles.size());

			for (const Triangle& Tri : Object->mesh->Triangles)
			{
				// 3D Space / World Space
				Triangle Proj = Tri;
				Proj.ApplyMatrix(Object->Transform.World);

				for (int i = 0; i < 3; i++)
				{
					Proj.WorldSpaceVerts[i] = Proj.Points[i];
				}

				if (Core::DoShadows && HasLight)
				{
					for (int i = 0; i < LightCount; i++)
					{
						LightObject* Light = SceneLights[i];
						// 3D Space -> Viewed Space -> Clipped Space

						if (Light->Type == LightTypes::DirectionalLight)
						{

							const Matrix VpMatrix = Light->VpMatrices[0];
							Proj.ApplyMatrix(VpMatrix);
							// Clipped Space -> NDC Space
							Proj.ApplyPerspectiveDivide();

							// Offset to viewport space (Ndc -> Screen Space)
							Proj.Scale(Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
							Proj.Translate(Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));


							Renderer::BaryCentricRasterizerDepth(&Proj, Core::ShadowMap, (SIZE_T)Core::ShadowMapWidth, (SIZE_T)Core::ShadowMapHeight, VpMatrix);
						}
						else if (SceneLights[i]->Type == LightTypes::PointLight)
						{
							for (int face = 0; face < 6; face++)
							{
								Matrix VpMatrix = SceneLights[i]->VpMatrices[face];
								Proj.ApplyMatrix(VpMatrix);
								// Clipped Space -> NDC Space
								Proj.ApplyPerspectiveDivide();

								// Offset to viewport space (Ndc -> Screen Space)
								Proj.Scale(Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 1.0f));
								Proj.Translate(Vec3((float)(Core::ShadowMapWidth * 0.5f), (float)(Core::ShadowMapHeight * 0.5f), 0.0f));

								Renderer::BaryCentricRasterizerDepth(&Proj, Core::ShadowMap, (SIZE_T)Core::ShadowMapWidth, (SIZE_T)Core::ShadowMapHeight, VpMatrix);
							}
						}
					}
				}
			}
		}
	}


	namespace Multithreaded
	{

	}


	namespace SIMD
	{

	}


	namespace GPU
	{

	}
}