#pragma once
#include "EnvironmentRenderable.h"
#include "Texture.h"
#include "Camera.h"

class Skybox : public Renderable
{
	CubeMap* Texture = nullptr;
	const Camera* Cam = nullptr;
	Cube* SkyboxCube = new Cube(1.0, 1.0, 1.0);

	Skybox()
	{

	}

public:
	static Skybox* Create(CubeMap* Map)
	{
		Skybox* Sky = new Skybox();

		Sky->Texture = Map;

		return Sky;
	}

	void SetCameraPtr(const Camera* Cam)
	{
		this->Cam = Cam;
	}

	virtual void Render() override
	{
		Matrix3x3 Rot = this->Cam->GetRotationMatrix();

		for (const Triangle& Tri : SkyboxCube->Triangles)
		{
			Vec4 pos = Vec4(vertexPosition, 1.0f);

			Vec4 clip =
				pos * ViewNoTranslation * Projection;

			Texture->Sample();

			
		}
	}
};