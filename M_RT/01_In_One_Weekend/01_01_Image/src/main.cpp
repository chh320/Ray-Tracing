#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <learnopengl/shader.h>

#include <draw.h>
#include <ray.h>
#include <camera.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int main() {
	const int image_width = 1600;
	const int image_height = 800;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(image_width, image_height, "M_RT", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	Shader shader("src/image.vs", "src/image.fs");
	
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		shader.use();
		shader.setVec3("camera.lower_left_corner", -2.0, -1.0, -1.0);
		shader.setVec3("camera.horizontal", 4.0, 0.0, 0.0);
		shader.setVec3("camera.verticle", 0.0, 2.0, 0.0);
		shader.setVec3("camera.origin", 0.0, 0.0, 0.0);

		shader.setVec2("screenSize", image_width, image_height);

		RenderImage();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}