#pragma once
#include "ConstantDefine.h"

class KayUtils
{
public:
	static Material* CreateDefaultMaterial();

	static Model* CreateGridModel(unsigned int rows, unsigned int cols);
};
