#pragma once
#ifdef TPGE_EXPORTS
#define TPGE_API extern "C" __declspec(dllexport)
#else
#define TPGE_API 
#endif
#include "ExampleScene.h"

namespace TerraPGE
{
	namespace API
	{
		TPGE_API void CreateDefaultWindow()
		{
			BrushPP ClearBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
			WndCreatorW Wnd = WndCreatorW(CS_OWNDC, L"GameEngine", L"Game Engine", LoadCursorW(NULL, IDC_ARROW), NULL, ClearBrush, (DWORD)WndExModes::BorderLessEx, (DWORD)WndModes::BorderLess | (DWORD)WndModes::ClipChildren, 0, 0, TerraPGE::Renderer::sx, TerraPGE::Renderer::sy);
		}

		namespace _Engine
		{
			struct GraphicsSettings
			{
				bool UseHDR;
				bool DoGammaCorrection;
				bool DebugClip;
				bool DebugShadowMap;
				bool DebugColliders;
				bool DebugDepthBuffer;
				bool DoLighting;
				int shadowMapWidth;
				int shadowMapHeight;
			};

			TPGE_API void SetGraphicsSettings(GraphicsSettings settings)
			{
				TerraPGE::Renderer::UseHDR = settings.UseHDR;
				TerraPGE::Renderer::DoGammaCorrection = settings.DoGammaCorrection;
				TerraPGE::Renderer::DebugClip = settings.DebugClip;
				TerraPGE::Renderer::DebugShadowMap = settings.DebugShadowMap;
				TerraPGE::DebugColliders = settings.DebugColliders;
				TerraPGE::Renderer::DebugDepthBuffer = settings.DebugDepthBuffer;
				TerraPGE::Renderer::DoLighting = settings.DoLighting;
				TerraPGE::Renderer::ShadowMapWidth = settings.shadowMapWidth;
				TerraPGE::Renderer::ShadowMapHeight = settings.shadowMapHeight;
			}

			TPGE_API GraphicsSettings GetGraphicsSettings()
			{
				GraphicsSettings settings;
				settings.UseHDR = TerraPGE::Renderer::UseHDR;
				settings.DoGammaCorrection = TerraPGE::Renderer::DoGammaCorrection;
				settings.DebugClip = TerraPGE::Renderer::DebugClip;
				settings.DebugShadowMap = TerraPGE::Renderer::DebugShadowMap;
				settings.DebugColliders = TerraPGE::DebugColliders;
				settings.DebugDepthBuffer = TerraPGE::Renderer::DebugDepthBuffer;
				settings.DoLighting = TerraPGE::Renderer::DoLighting;
				settings.shadowMapWidth = TerraPGE::Renderer::ShadowMapWidth;
				settings.shadowMapHeight = TerraPGE::Renderer::ShadowMapHeight;
				return settings;
			}
		}


		namespace _Scene
		{
			TPGE_API void DestroyScene(void* scene)
			{
				ExampleScene* exScene = static_cast<ExampleScene*>(scene);
				if (exScene)
				{
					delete exScene;
				}
			}

			TPGE_API void* GetCurrentScene()
			{
				return TerraPGE::CurrScene;
			}
		}


		namespace _Camera
		{
			TPGE_API void* GetMainCamera(void* scene)
			{
				ExampleScene* exScene = static_cast<ExampleScene*>(scene);
				if (exScene && exScene->MainCamera)
				{
					return exScene->MainCamera;
				}
			}

			TPGE_API void ChangeCameraPosition(void* Cam, float x, float y, float z)
			{
				((Camera*)Cam)->SetLocalPosition(Vec3(x, y, z));
				Camera* CamPtr = (Camera*)Cam;
			}
		}

		namespace ObjectInterface
		{
			struct MaterialInfo
			{
				float ACr;
				float ACg;
				float ACb;
				float DCr;
				float DCg;
				float DCb;
				float SCr;
				float SCg;
				float SCb;
				float ECr;
				float ECg;
				float ECb;
				float Es;
				char* DiffuseMapName;
				char* SpecularMapName;
				char* EmmissiveMapName;
				char* MaterialName;
			};


			struct MeshInfo
			{

			};


			namespace Material
			{
				TPGE_API void* CreateMaterial(MaterialInfo info)
				{
					return ::Material::CreateMaterial(Vec3(info.ACr, info.ACg, info.ACb), Vec3(info.DCr, info.DCg, info.DCb), Vec3(info.SCr, info.SCg, info.SCb), info.Es, std::string(info.MaterialName));
				}

				TPGE_API void** GetAllLoadedMaterials()
				{
					return (void**)(::Material::LoadedMaterials.data());
				}

				TPGE_API void** SetMaterialInfo(MaterialInfo Info, void* Material)
				{
					return nullptr;
				}

				namespace Texture
				{
					TPGE_API void* CreateTexture()
					{
//						return ::Texture::Create(Vec3(info.ACr, info.ACg, info.ACb), Vec3(info.DCr, info.DCg, info.DCb), Vec3(info.SCr, info.SCg, info.SCb), info.Es, std::string(info.MaterialName));
						return nullptr;
					}

					TPGE_API void** GetAllLoadedTextures()
					{
						return (void**)(::Texture::LoadedTextures.data());
					}
				}
			}


			TPGE_API size_t GetObjectCount(void* scene)
			{
				ExampleScene* exScene = static_cast<ExampleScene*>(scene);
				if (exScene)
				{
					return exScene->GetObjects()->size();
				}

				return 0;
			}


			TPGE_API char* GetObjectName(void* object)
			{
				TerraPGE::Renderable* rend = static_cast<TerraPGE::Renderable*>(object);
				if (rend)
				{
					return (char*)rend->mesh->MeshName.c_str();
				}
				return nullptr;
			}


			TPGE_API void* GetObjectAt(void* scene, __int32 index)
			{
				ExampleScene* exScene = static_cast<ExampleScene*>(scene);
				if (exScene)
				{
					const std::vector<TerraPGE::Renderable*>* objects = exScene->GetObjects();
					if (index < objects->size())
					{
						return (*objects)[index];
					}
				}
				return nullptr;
			}


			TPGE_API void* SpawnCubeAt(void* scene, float x, float y, float z)
			{
				ExampleScene* exScene = static_cast<ExampleScene*>(scene);
				if (exScene)
				{
					return exScene->SpawnCubeAt(Vec3(x, y, z));
				}
				return nullptr;
			}


			TPGE_API __int32 GetMaterialCount(void* object)
			{
				return static_cast<TerraPGE::Renderable*>(object)->mesh->Materials.size();
			}


			TPGE_API MaterialInfo GetMaterialInfo(void* object, __int32 mIdx)
			{
				TerraPGE::Renderable* rend = static_cast<TerraPGE::Renderable*>(object);

				::Material* mat = rend->mesh->Materials[mIdx];

				MaterialInfo info;

				info.ACr = mat->AmbientColor.x;
				info.ACg = mat->AmbientColor.y;
				info.ACb = mat->AmbientColor.z;
				info.DCr = mat->DiffuseColor.x;
				info.DCg = mat->DiffuseColor.y;
				info.DCb = mat->DiffuseColor.z;
				info.SCr = mat->SpecularColor.x;
				info.SCg = mat->SpecularColor.y;
				info.SCb = mat->SpecularColor.z;
				info.ECr = mat->EmissiveColor.x;
				info.ECg = mat->EmissiveColor.y;
				info.ECb = mat->EmissiveColor.z;
				info.Es = mat->EmissiveStrength;

				if(mat->DiffuseMap)
					info.DiffuseMapName = _strdup(mat->DiffuseMap->Name.c_str());

				if (mat->SpecularMap)
					info.SpecularMapName = _strdup(mat->SpecularMap->Name.c_str());

				if (mat->EmissiveMap)	
					info.EmmissiveMapName = _strdup(mat->EmissiveMap->Name.c_str());

				if (!mat->MaterialName.empty())
					info.MaterialName = _strdup(mat->MaterialName.c_str());

				return info;
			}


			TPGE_API void SetObjectMaterial(void* object, void* material)
			{
				TerraPGE::Renderable* rend = static_cast<TerraPGE::Renderable*>(object);
				::Material* mat = static_cast<::Material*>(material);
				if (rend && mat)
				{
					rend->mesh->ForceMatInfo((::Material*)material);
				}
			}
			

			TPGE_API void SetObjectMaterialInfo(void* object, __int32 mIdx, MaterialInfo& info)
			{
				TerraPGE::Renderable* rend = static_cast<TerraPGE::Renderable*>(object);
				::Material* mat = rend->mesh->Materials[mIdx];

				mat->AmbientColor = Vec3(info.ACr, info.ACg, info.ACb);
				mat->DiffuseColor = Vec3(info.DCr, info.DCg, info.DCb);
				mat->SpecularColor = Vec3(info.SCr, info.SCg, info.SCb);
				mat->EmissiveColor = Vec3(info.ECr, info.ECg, info.ECb);
				mat->EmissiveStrength = info.Es;
				mat->MaterialName = std::string(info.MaterialName);
			}


			TPGE_API void GetMeshInfo()
			{

			}


			TPGE_API void GetRootObject()
			{

			}
		}
	}
}



// need a way to provide a scene to the engine from the outside, maybe a function pointer or a scene factory
// maybe serialized scene?
TPGE_API void RunEngine(HWND WndHnd)
{
	WndCreatorW Wnd = WndCreatorW(WndHnd);

#ifdef _DEBUG
	TerraPGE::Renderer::RenderingUtils::OpenConsole();
#endif

	ExampleScene* ExScene = new ExampleScene();

	TerraPGE::Run(Wnd, ExScene);
}