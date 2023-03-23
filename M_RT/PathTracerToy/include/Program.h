#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <Shader.h>

/* generate shader and link program */

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

		void setInt(const std::string& name, const int& a) {
			glUniform1i(glGetUniformLocation(object, name.c_str()), a);
		}

		void setFloat(const std::string& name, const float& a) {
			glUniform1f(glGetUniformLocation(object, name.c_str()), a);
		}

		void setFloat2(const std::string& name, const float& a, const float& b) {
			glUniform2f(glGetUniformLocation(object, name.c_str()), a, b);
		}

		void setFloat3(const std::string& name, const float& a, const float& b, const float& c) {
			glUniform3f(glGetUniformLocation(object, name.c_str()), a, b, c);
		}
	};
}
#endif // !PROGRAM_H
