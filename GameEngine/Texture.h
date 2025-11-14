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

	private:

	static inline std::vector<Texture*> LoadedTextures = {};

	int Width = 0;
	int Height = 0;
	std::vector<unsigned char> PixelData = {};
	WrappingMode WrapMode;

	~Texture()
	{
		auto it = std::find(LoadedTextures.begin(), LoadedTextures.end(), this);

		if (it != LoadedTextures.end())
		{
			this->LoadedTextures.erase(it);
		}
	}

	public:

	Texture()
	{
		this->Used = false;
		this->Width = 0;
		this->Height = 0;

		this->WrapMode = WrappingMode::Clamp;
	}

	Texture(const std::string& Filename, WrappingMode Mode = WrappingMode::Clamp)
	{
		Width = 0;
		Height = 0;
		if (Filename.find(".bmp") != std::string::npos)
		{
			Used = LoadBMP(Filename);
		}
		else if (Filename.find(".spr") != std::string::npos)
		{
			Used = LoadSPR(Filename);
		}

		this->WrapMode = Mode;
	}

	bool LoadBMP(const std::string& filename)
	{
		if (this->FindTexture(filename) != nullptr)
		{
			*this = *FindTexture(filename);
			return true;
		}

		this->Name = filename;

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
		int colorDepth = *(int*)&header[28]; // Bits per pixel

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

		this->LoadedTextures.push_back(this);

		return true;
	}

	bool LoadSPR(const std::string& filename)
	{
		if (this->FindTexture(filename) != nullptr)
		{
			*this = *FindTexture(filename);
			return true;
		}

		this->Name = filename;

		std::ifstream file(filename, std::ios::in | std::ios::binary);

		if (!file) {
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

		LoadedTextures.push_back(this);
		return true;
	}

	bool Used = false;

	void SetWrapMode(WrappingMode WrapMode)
	{
		this->WrapMode = WrapMode;
	}

	std::string Name = "";

	Texture* FindTexture(std::string Name)
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
		if (this->PixelData.empty() || this->PixelData.size() == 0)
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
		
		// Calculate pixel coordinates
		int x = PixelRound(u * (float)(this->Width - 1.0f));
		int y = PixelRound(v * (float)(this->Height - 1.0f));

		// Ensure coordinates are within bounds
		x = std::clamp<int>(x, 0, (this->Width - 1));
		y = std::clamp<int>(y, 0, (this->Height - 1));

		// Calculate the index in the pixel data
		int index = (x + this->Width * y) * 3;

		// Extract RGB values
		float b = static_cast<float>(this->PixelData[index]);
		float g = static_cast<float>(this->PixelData[index + 1]);
		float r = static_cast<float>(this->PixelData[index + 2]);

		return Color(r, g, b);
	}

	void Delete()
	{
		auto it = std::find(LoadedTextures.begin(), LoadedTextures.end(), this);

		if (it != LoadedTextures.end())
		{
			this->LoadedTextures.erase(it);
		}
	}
};