#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <ShaderIncludes.h>
#include <Config.h>

namespace GLSLPT {
	class Shader {
	private:
		GLuint object;
	public:
		Shader(const ShaderInclude::ShaderSource& sourceObj, GLuint shaderType);
		GLuint getObject() const;
	};
}
#endif // !SHADER_H
