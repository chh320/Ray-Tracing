#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

#include <learnopengl/camera.h>

#include "render.h"

GLFWwindow* window;
std::shared_ptr<Render> render;

const int SCR_WIDTH = 900;
const int SCR_HEIGHT = 800;


float lastX = SCR_WIDTH  * 0.5f;
float lastY = SCR_HEIGHT * 0.5f;
bool firstMouse = true;

float deltaTime = 0.f;
float lastFrame = 0.f;

unsigned int frameCounter;
clock_t t1, t2;

const std::string shaderPath = "./assets/shaders/";
const std::string objectPath = "./assets/obj/";

bool initWindow();
bool initRender();
void Draw();

int main() {
	if (!initWindow())
		return -1;
	
	if (!initRender())
		return -1;

	while (!glfwWindowShouldClose(window)) {
		Draw();
	}
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

bool initWindow(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Path Trace Toy", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
}

bool initRender() {
	render = std::make_shared<Render>(shaderPath, objectPath, SCR_WIDTH, SCR_HEIGHT);
	return true;
}

void Draw() {
	float curFrame = glfwGetTime();
	deltaTime = curFrame - lastFrame;
	lastFrame = curFrame;

	// 帧计时
	t2 = clock();
	float dt = (double)(t2 - t1) / CLOCKS_PER_SEC;
	float fps = 1.0 / dt;
	std::cout << "\r";
	std::cout << std::fixed << std::setprecision(2) << "FPS : " << fps << "    迭代次数: " << frameCounter << std::endl;
	t1 = t2;

	processInput(window);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	render->draw(frameCounter);

	glfwSwapBuffers(window);
	glfwPollEvents();
}