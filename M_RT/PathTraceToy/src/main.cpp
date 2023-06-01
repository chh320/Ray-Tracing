#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>

#include <learnopengl/camera.h>
#include "render.h"

GLFWwindow* window;
std::shared_ptr<Render> render;

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 800;

int lastWidth = SCR_WIDTH;
int lastHeight = SCR_HEIGHT;

float lastX = SCR_WIDTH  * 0.5f;
float lastY = SCR_HEIGHT * 0.5f;
bool firstMouse = true;

float deltaTime = 0.f;
float lastFrame = 0.f;

unsigned int frameCounter = 1;
clock_t t1, t2;

const std::string shaderPath = "./assets/shaders/";
const std::string objectPath = "./assets/obj/";
const std::string evnmapDirectory = "./assets/hdr/";

bool initWindow();
bool initRender();
void Draw();
void Update();
void showFPS(GLFWwindow* pWindow);

int main() {
	if (!initWindow())
		return -1;
	
	if (!initRender())
		return -1;

	GLFWvidmode return_struct;

	while (!glfwWindowShouldClose(window)) {
		showFPS(window);
		Draw();
		Update();
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	render->camera->ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	render->camera->ProcessMouseScroll(static_cast<GLfloat>(yoffset));
}

bool initWindow(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
	//glfwSetCursorPosCallback(window, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSwapInterval(0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
}

bool initRender() {
	render = std::make_shared<Render>(shaderPath, objectPath, evnmapDirectory, SCR_WIDTH, SCR_HEIGHT);
	return true;
}

void showFPS(GLFWwindow* pWindow)
{
	// Measure speed
	float curFrame = glfwGetTime();
	deltaTime = curFrame - lastFrame;

	t2 = clock();
	float dt = (double)(t2 - t1) / CLOCKS_PER_SEC;
	float fps = 1.0 / dt;
	if (deltaTime >= 1.0) {
		std::stringstream ss;
		ss << "Path Trace Toy " << " [" << fps << " FPS]" << " frame count : " << frameCounter;

		glfwSetWindowTitle(pWindow, ss.str().c_str());

		lastFrame = curFrame;
	}
	t1 = t2;

}

void Draw() {
	processInput(window);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	render->Draw();

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void updateRenderSize() {
	glfwGetWindowSize(window, &render->renderWidth, &render->renderHeight);
	if (render->renderWidth != lastWidth || render->renderHeight != lastHeight) {
		lastWidth = render->renderWidth;
		lastHeight = render->renderHeight;
		render->ResizeRenderer(shaderPath);
	}
}

void Update(){
	/*if(frameCounter % 50 == 0)
		std::cout << "frame count : " << frameCounter << std::endl;*/
	updateRenderSize();
	render->Update(frameCounter);

}