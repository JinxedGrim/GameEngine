#pragma once
#include "Math.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#define _NULL_TEXTURE_VALUES 255.0f, 0.0f, 255.0f
#define NULL_TEXTURE_COLOR Color(_NULL_TEXTURE_VALUES)
#define NULL_TEXTURE_COLOR_VEC3 Vec3(_NULL_TEXTURE_VALUES)

// Future plans
// Resource system should actually look for images rather than the texture object -- texture obj doesnt store much data compared to images
// Resource system should also count references
// Finish some of the wrapping modes and test
// fix uv issues
// add texture filtering



// call fact create / load
//     this function creates and registers the data
//     during registry a hash is created that points to the obj
//     on delete, make sure to clear the data and deregister (or subtract ref count and delete on 0)



class Image2D
{
	std::vector<unsigned char> PixelData = {};
	bool Loaded = false;
	int Width = 0;
	int Height = 0;

	Image2D()
	{

	}

	public:


	bool LoadBMP(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);

		if (!file || !file.is_open())
		{
			return false;
		}

		// Read the BMP header
		char header[54];
		file.read(header, 54);

		// Check the BMP signature
		if (header[0] != 'B' || header[1] != 'M')
		{
			file.close();
			return false;
		}

		// Extract width and height from the header
		Width = *(int*)&header[18];
		Height = *(int*)&header[22];
		int colorDepth = *(__int16*)&header[28]; // Bits per pixel

		// Calculate the number of bytes per pixel (based on color depth)
		int bytesPerPixel = colorDepth / 8;

		// Calculate the size of the pixel data (excluding padding)
		int dataSize = Width * Height * bytesPerPixel;

		// Read pixel data
		std::vector<unsigned char> rawPixelData(dataSize);
		file.read(reinterpret_cast<char*>(rawPixelData.data()), dataSize);

		file.close();

		// If color depth is greater than 3 (such as 4), convert to 3 (24 bits)
		if (bytesPerPixel > 3) {
			// Convert to 24-bit (3 bytes per pixel)
			PixelData.resize(Width * Height * 3);
			for (int i = 0, j = 0; i < dataSize; i += bytesPerPixel, j += 3) {
				PixelData[j] = rawPixelData[i];         // Red
				PixelData[j + 1] = rawPixelData[i + 1]; // Green
				PixelData[j + 2] = rawPixelData[i + 2]; // Blue
			}
		}
		else {
			// Color depth is already 3, no need to convert
			PixelData = std::move(rawPixelData);
		}

		this->Loaded = true;
		return true;
	}


	bool LoadSPR(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);

		if (!file) 
		{
			std::cerr << "Failed to open SPR file." << std::endl;
			return false;
		}

		// Read SPR header (assuming a simple format)
		int sprWidth = 0, sprHeight = 0;
		file.read(reinterpret_cast<char*>(&sprWidth), sizeof(int));
		file.read(reinterpret_cast<char*>(&sprHeight), sizeof(int));


		// Ensure that the header was read correctly
		if (sprWidth <= 0 || sprHeight <= 0) {
			std::cerr << "Invalid SPR file format." << std::endl;
			return false;
		}

		// Update the texture width and height
		this->Width = sprWidth;
		this->Height = sprHeight;

		// Read pixel data
		int dataSize = this->Width * this->Height * 3; // Assuming 3 channels (RGB)
		this->PixelData.resize(dataSize);
		file.read(reinterpret_cast<char*>(this->PixelData.data()), dataSize);

		file.close();

		this->Loaded = true;
		return true;
	}


	static Image2D* Load(const std::string& FilePath)
	{
		Image2D* Out = new Image2D();

		bool Success = false;

		if (FilePath.find(".bmp") != std::string::npos)
		{
			Success = Out->LoadBMP(FilePath);
		}
		else if (FilePath.find(".spr") != std::string::npos)
		{
			Success = Out->LoadSPR(FilePath);
		}

		if (!Out->Loaded || !Success)
		{
			return nullptr;
		}

		return Out;
	}


	bool IsLoaded() const
	{
		return this->Loaded;
	}


	Color GetColorAtPixel(int x, int y) const
	{
		int index = (x + this->Width * y) * 3;

		float b = static_cast<float>(this->PixelData[index]);
		float g = static_cast<float>(this->PixelData[index + 1]);
		float r = static_cast<float>(this->PixelData[index + 2]);

		return Color(r, g, b);
	}


	bool IsSquare()
	{
		if (Width == Height)
		{
			return true;
		}

		return false;
	}


	int GetWidth() const
	{
		return this->Width;
	}


	int GetHeight() const
	{
		return this->Height;
	}


	void Delete()
	{
		delete this;
	}
};


class TextureCoords
{
	public:

	TextureCoords()
	{
		u = 0;
		v = 0;
		w = 1.0f;
	}

	TextureCoords(float _u, float _v, float _w = 1.0f)
	{
		u = _u;
		v = _v;
		w = _w;
	}

	float u = 0;
	float v = 0;
	float w = 1.0f;

	void __inline CorrectPerspective(float _w)
	{
		if (_w != 0)
		{
			this->u = this->u / _w;
			this->v = this->v / _w;
			this->w = 1.0f / _w;
		}
	}

	TextureCoords Lerp(const TextureCoords& end, float t) const
	{
		TextureCoords result;

		result.u = this->u + (end.u - this->u) * t;
		result.v = this->v + (end.v - this->v) * t;
		result.w = this->w + (end.w - this->w) * t;

		return result;
	}

	__inline Vec3 AsVec3()
	{
		return
		{
			this->u,
			this->v,
			this->w,
		};
	}

	void Lerped(const TextureCoords& end, float t)
	{
		this->u = this->u + (end.u - this->u) * t;
		this->v = this->v + (end.v - this->v) * t;
		this->w = this->w + (end.w - this->w) * t;
	}
};


class Texture
{
	public:

	enum class WrappingMode
	{
		Clamp,
		Repeat,
		MirroredRepeat
	};

	enum class FilterMode
	{
		Nearest,
		Bilinear
	};

	private:

	static inline std::vector<Texture*> LoadedTextures = {};

	WrappingMode WrapMode;
	Image2D* Image = nullptr;

	~Texture()
	{
		auto it = std::find(LoadedTextures.begin(), LoadedTextures.end(), this);

		if (it != LoadedTextures.end())
		{
			this->LoadedTextures.erase(it);
		}
	}

	public:
	bool Used = false;


	Texture()
	{
		this->Used = false;
		this->WrapMode = WrappingMode::Clamp;
	}


	Texture(const std::string& Filename, WrappingMode Mode = WrappingMode::Clamp)
	{
		if (this->FindTexture(Filename) != nullptr)
		{
			*this = *FindTexture(Filename);
			return;
		}

		this->Image = Image2D::Load(Filename);

		this->Used = this->Image->IsLoaded();
		this->Name = Filename;
		this->WrapMode = Mode;
		LoadedTextures.push_back(this);
	}


	void SetWrapMode(WrappingMode WrapMode)
	{
		this->WrapMode = WrapMode;
	}


	std::string Name = "";


	Texture* FindTexture(std::string Name) const
	{
		for (Texture* T : LoadedTextures)
		{
			if (T->Name == Name)
			{
				return T;
			}
		}

		return nullptr;
	}


	Color GetPixelColor(float u, float v) const
	{
		if (this->Image == nullptr || this->Image->IsLoaded() == false)
		{
			return NULL_TEXTURE_COLOR;
		}

		if (WrapMode == WrappingMode::Repeat)
		{
			u = u - std::floor(u); // Repeat mode
			v = v - std::floor(v);
		}
		else if (WrapMode == WrappingMode::MirroredRepeat)
		{
			if (static_cast<int>(std::floor(u)) % 2) { u = 1.0f - (u - std::floor(u)); }
			else { u = u - std::floor(u); }
			if (static_cast<int>(std::floor(v)) % 2) { v = 1.0f - (v - std::floor(v)); }
			else { v = v - std::floor(v); }
		}

		int Width = this->Image->GetWidth();
		int Height = this->Image->GetHeight();
		
		// Calculate pixel coordinates
		int x = PixelRound(u * (float)(Width - 1.0f));
		int y = PixelRound(v * (float)(Height - 1.0f));

		// Ensure coordinates are within bounds
		x = std::clamp<int>(x, 0, (Width - 1));
		y = std::clamp<int>(y, 0, (Height - 1));

		return this->Image->GetColorAtPixel(x, y);
	}


	void Delete()
	{
		this->Image->Delete();

		auto it = std::find(LoadedTextures.begin(), LoadedTextures.end(), this);

		if (it != LoadedTextures.end())
		{
			this->LoadedTextures.erase(it);
		}
	}
};


class CubeMap
{
	Image2D* Faces[6];
	int FaceSize;

	CubeMap()
	{

	}

	bool CheckFaces()
	{
		for (int i = 0; i < 6; i++)
		{
			if ((Faces[i] == nullptr) || !Faces[i]->IsLoaded() && !Faces[i]->IsSquare())
			{
				return false;
			}
		}

		return true;
	}

	public:

	static CubeMap* LoadCubemapFromImages(std::string px, std::string py, std::string pz, std::string nx, std::string ny, std::string nz)
	{
		CubeMap* Out = new CubeMap();
		Out->Faces[0] = Image2D::Load(px);
		Out->Faces[1] = Image2D::Load(py);
		Out->Faces[2] = Image2D::Load(pz);

		Out->Faces[3] = Image2D::Load(nx);
		Out->Faces[4] = Image2D::Load(ny);
		Out->Faces[5] = Image2D::Load(nz);

		if (!Out->CheckFaces())
		{
			return nullptr;
		}

		Out->FaceSize = Out->Faces[0]->GetWidth();

		return Out;
	}

	static CubeMap* LoadCubemapFromDirectory(const std::string Directory, const std::string Ext=".bmp")
	{
		return LoadCubemapFromImages(Directory + "px" + Ext, Directory + "py" + Ext, Directory + "pz" + Ext, Directory + "nx" + Ext, Directory + "ny" + Ext, Directory + "nz" + Ext);
	}

	Color Sample(const Vec3& dir) const
	{

	}
};