#ifndef QUAD_H
#define QUAD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <learnopengl/shader.h>

class Shader;

class Quad
{
public:
    Quad();
    void Draw(std::shared_ptr<Shader> shader, unsigned int& frameCounter);

private:
    GLuint vao;
    GLuint vbo;
};

#endif // !QUAD_H
