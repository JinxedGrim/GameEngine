#pragma once
class BufferType
{
	void* Data = nullptr;
	__int64 DataSz = 0;

public:
	BufferType() = delete;

	BufferType(void* Data)
	{
		this->Data = Data;
	}
};

template <typename T>
class BufferObject : public BufferType
{
	unsigned __int32 H = 0;
	unsigned __int32 W = 0;
	unsigned __int32 C = 0;

	void* AllocateBuffer(unsigned __int32 H, unsigned __int32 W, unsigned __int32 C)
	{
		return malloc((sizeof(T) * H) * (sizeof(T) * W) * (C * sizeof(T)));
	}

	BufferObject(unsigned __int32 H, unsigned __int32 W, unsigned __int32 C) : AllocateBuffer(H, W, C)
	{
		this->H = H;
		this->H = W;
		this->H = C;
	}

public:
	BufferObject(void* Data) : BufferType(Data)
	{}

	BufferObject* Create(__int32 H, __int32 W, __int32 C)
	{
		this = new BufferObject(H, W, C)
	}

	void Delete()
	{
		delete[] this->Data;
	}
};

class FrameBuffer : public BufferObject<char>
{
	FrameBuffer(unsigned __int32 Height, unsigned __int32 Width) : BufferObject(Height, Width, 1)
	{

	}
};

class HDRBuffer : public BufferObject<float>
{
	HDRBuffer(unsigned __int32 Height, unsigned __int32 Width) : BufferObject(Height, Width, 3)
	{

	}
};

class DepthBuffer : public BufferObject<float>
{
	DepthBuffer(unsigned __int32 Height, unsigned __int32 Width) : BufferObject(Height, Width, 1)
	{

	}
};

class ShadowBuffer : public BufferObject<float>
{
	ShadowBuffer(unsigned __int32 Height, unsigned __int32 Width) : BufferObject(Height, Width, 1)
	{

	}
};