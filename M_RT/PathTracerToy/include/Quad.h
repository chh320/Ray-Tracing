#ifndef QUAD_H
#define QUAD_H

#include <Config.h>

namespace GLSLPT {
	class Program;

	class Quad
	{
	public:
		Quad();
		void Draw(Program*);
	private:
		GLuint vao;
		GLuint vbo;
	};
}
#endif // !QUAD_H
