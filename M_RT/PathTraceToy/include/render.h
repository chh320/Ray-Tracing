#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include <string>
#include "Quad.h"
#include <learnopengl/shader.h>

class Render
{
public:
	Render(const std::string& shadersDirectory) {
		quad = std::make_shared<Quad>();
		InitGPUDataBuffers();
		InitFBOs();
		InitShaders();
	}
protected:
	std::shared_ptr<Quad> quad;
private:
	void InitFBOs();
	void InitGPUDataBuffers();
	void InitShaders();
};
#endif // !RENDER_H
