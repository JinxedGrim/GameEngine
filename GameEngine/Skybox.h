#pragma once
#include "EnvironmentRenderable.h"
#include "Texture.h"
#include "Camera.h"

class Skybox : public EnvironmentRenderable
{
	CubeMap* Texture = nullptr;
	Camera* Cam = nullptr;

	Skybox(Camera* Cam, CubeMap* Textures) : EnvironmentRenderable(Cam)
	{
		this->Texture = Textures;
		this->Cam = Cam;
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

	Vec3 SampleForSunDirection(bool Debug = false)
	{
		int sz = this->Texture->GetFaceSize();
		float MaxLuminance = 0.0f;
		int MaxFace = 0;
		int MaxX = 0;
		int MaxY = 0;

		for (int face = 0; face < 6; face++)
		{
			for (int x = 0; x < sz; x++)
			{
				for (int y = 0; y < sz; y++)
				{
					float SampledLum = this->Texture->Sample(x, y, face).Denormalized().CalculateLuminance();

					if (SampledLum > MaxLuminance)
					{
						MaxFace = face;
						MaxX = x;
						MaxY = y;
						MaxLuminance = SampledLum; 
					}
				}
			}
		}

		float u = (MaxX + 0.5f) / sz; // 0..1
		float v = (MaxY + 0.5f) / sz; // 0..1
		float px = 2.0f * u - 1.0f;         // -1..1
		float py = 1.0f - 2.0f * v;         // -1..1

		Vec3 LightDir;

		switch (MaxFace)
		{
			case CUBEMAP_PX:
				LightDir = Vec3(1, -py, px);
				break;
			case CUBEMAP_NX:
				LightDir = Vec3(-1, -py, -px);
				break;
			case CUBEMAP_PY:
				LightDir = Vec3(px, 1, py);
				break;
			case CUBEMAP_NY:
				LightDir = Vec3(px, -1, -py);
				break;
			case CUBEMAP_PZ:
				LightDir = Vec3(px, -py, 1);
				break;
			case CUBEMAP_NZ:
				LightDir = Vec3(-px, -py, -1);				
				break;
			default:
				throw;
				break;
		}

		LightDir = -(LightDir.Normalized());

		if (Debug)
		{
			this->Texture->SetPixel(MaxX, MaxY, MaxFace, Color(255.0f, 0.0f, 0.0f));
		}

		std::cout << "MaxLum: " << MaxLuminance << " at face: " << MaxFace << " at: (" << MaxX << ", " << MaxY << ")" << std::endl << "Calculated LightDir: " << LightDir << std::endl;
		return LightDir;
	}

	// replace with shader call TODO
	virtual Color Render(const int x, const int y, const int width, const int height, const float& Fov, const Matrix3x3& CamRot) override
	{
		float aspect = (float)width / height;

		float px = (2.0f * (x + 0.5f) / width - 1.0f) * tan(Fov * 0.5f) * aspect;
		float py = (1.0f - 2.0f * (y + 0.5f) / height) * tan(Fov * 0.5f);

		Vec3 viewDir = Vec3(px, py, 1.0f).Normalized();

		// 3. Rotate into world space (NO translation)
		Vec3 worldDir = viewDir * CamRot;
		worldDir = worldDir;

		// 4. Sample cubemap
		return Texture->Sample(worldDir);
	}
};