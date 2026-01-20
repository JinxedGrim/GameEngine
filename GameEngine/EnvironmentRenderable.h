#pragma once
#include "Renderable.h"

class EnvironmentRenderable : public TerraPGE::Renderable
{
public:
	EnvironmentRenderable(Camera* Cam): Renderable(nullptr, Cam, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), TerraPGE::EngineShaders::DefaultShader)
	{
		
	}

	virtual Color Render(int x, int y,  int width, int height, const Matrix3x3& CamRot) = 0;
};