#pragma once
#include "GameObject.h"
#include "Shading.h"

namespace TerraPGE
{
	class Renderable : public GameObject
	{
	public:
		Renderable() = delete;


		Renderable(Mesh* mesh, Camera* Cam, const Vec3& Scalar, const Vec3& EulerRotatiom, const Vec3& Pos, const std::function<void(ShaderArgs*)> Shader, const ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT) : GameObject(Scalar, EulerRotatiom, Pos)
		{
			this->mesh = mesh;
			this->Shader = Shader;
			this->SHADER_TYPE = SHADER_TYPE;
		}


		Mesh* mesh = nullptr;
		std::function<void(ShaderArgs*)> Shader = EngineShaders::Shader_Frag_Phong_Shadows;
		ShaderTypes SHADER_TYPE = ShaderTypes::SHADER_FRAGMENT;
	};
}