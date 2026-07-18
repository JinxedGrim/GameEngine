#pragma once

template <typename T>
class BaseResource
{
public:
	std::vector<T*> LoadedResources;

	BaseResource() = default;
	virtual ~BaseResource() = default;


}