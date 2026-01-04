#pragma once
#include "EnvironmentRenderable.h"
#include "Texture.h"
#include "Camera.h"

class Skybox : public EnvironmentRenderable
{
	CubeMap* Texture = nullptr;
	Camera* Cam = nullptr;
	Cube* SkyboxCube = nullptr;

	Skybox(Camera* Cam, CubeMap* Textures)
	{
		this->Texture = Textures;
		this->Cam = Cam;
		this->SkyboxCube = DEBUG_NEW Cube(1, 1, 1);
	}

public:
	static Skybox* Create(Camera* Cam, CubeMap* Map)
	{
		Skybox* Sky = DEBUG_NEW Skybox(Cam, Map);

		Sky->Texture = Map;

		return Sky;
	}

	void SetCameraPtr(Camera* Cam)
	{
		this->Cam = Cam;
	}

	virtual void Render() override
	{
		Matrix MVP = Cam->GetViewMatrix();
		for (const Triangle& Tri : SkyboxCube->Triangles)
		{
//			Vec4 pos = Vec4(vertexPosition, 1.0f);

//			Vec4 clip =
//				pos * ViewNoTranslation * Projection;

//			Texture->Sample();
		}
	}
};