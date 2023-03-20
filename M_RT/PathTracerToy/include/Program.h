#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <Shader.h>

namespace GLSLPT {
	class Program {
	private:
		GLuint object;
	public:
		Program(const std::vector<Shader> shaders);
		~Program();
		void Use();
		void StopUsing();
		GLuint getObject();
	};
}
#endif // !PROGRAM_H
