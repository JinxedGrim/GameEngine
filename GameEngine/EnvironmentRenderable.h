#pragma once
#include "Renderable.h"

class EnvironmentRenderable : public TerraPGE::Renderable
{
public:
	virtual void Render() = 0;
};