#pragma once

#include "IOglShader.h"

#include <xmemory>

namespace Ogl
{

class LumaShader:public Ogl::IShader
{
public:
	LumaShader();
	~LumaShader();

protected:
	void ApplyParameters(GLenum tex);

private:
	static const std::string vsCode;
	static const std::string fsCode;
};

};
