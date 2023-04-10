#ifndef RENDER_H
#define RENDER_H

#include <vector>
#include <string>
#include <iomanip>

#include "Quad.h"
#include "mesh.h"
#include "material.h"
#include "bvh.h"
#include "scene.h"
#include <learnopengl/shader.h>

class Render
{
public:
	Render(){}
	Render(const std::string& shadersDirectory, const std::string& objectDirectory, const int& width, const int& height) : renderWidth(width), renderHeight(height){
		quad = std::make_shared<Quad>();
		
		Scene scene = Scene(objectDirectory);
		trianglesAttrib = scene.trianglesAttrib;
		bvhNodes = scene.bvhNodes;

		camera = std::make_shared<Camera>(glm::vec3(0.f, 0.1f, 0.5f));

		InitGPUDataBuffers();
		//InitFBOs();
		InitShaders(shadersDirectory);
	}
	void draw(unsigned int& frameCounter);

protected:
	std::shared_ptr<Quad> quad;
	std::shared_ptr<Mesh> bunny;
	std::shared_ptr<Mesh> sphere;
	std::shared_ptr<Mesh> floor;

	std::shared_ptr<Bvh> bvh;

	std::vector<glm::vec3> trianglesAttrib;
	std::vector<glm::vec3> bvhNodes;

	std::shared_ptr<Camera> camera;

	// texture buffer for data
	GLuint triangleBuffer;
	GLuint normalsBuffer;
	GLuint bvhNodeBuffer;

	GLuint triangleTex;
	GLuint bvhNodeTex;

	// FBOs
	GLuint pathTraceFBO;
	GLuint accumFBO;

	// Shaders
	std::shared_ptr<Shader> pathTraceShader;
	std::shared_ptr<Shader> outputShader;

	// Render textures
	GLuint pathTraceTexture;
	GLuint accumTexture;

	int renderWidth;
	int renderHeight;

	bool initialized;

private:
	void InitFBOs();
	void InitGPUDataBuffers();
	void InitShaders(const std::string& shadersDirectory);
};

void Render::InitGPUDataBuffers() {
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// Create buffer and texture for vertices
	glGenBuffers(1, &triangleBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, triangleBuffer);
	glBufferData(GL_TEXTURE_BUFFER, trianglesAttrib.size() * sizeof(glm::vec3), trianglesAttrib.data(), GL_STATIC_DRAW);
	glGenTextures(1, &triangleTex);
	glBindTexture(GL_TEXTURE_BUFFER, triangleTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, triangleBuffer);

	glGenBuffers(1, &bvhNodeBuffer);
	glBindBuffer(GL_TEXTURE_BUFFER, bvhNodeBuffer);
	glBufferData(GL_TEXTURE_BUFFER, bvhNodes.size() * sizeof(glm::vec3), bvhNodes.data(), GL_STATIC_DRAW);
	glGenTextures(1, &bvhNodeTex);
	glBindTexture(GL_TEXTURE_BUFFER, bvhNodeTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, bvhNodeBuffer);

	// Bind textures to texture slots as they will not change slots during the lifespan of the renderer
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, triangleTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, bvhNodeTex);
}

void Render::InitShaders(const std::string& shadersDirectory) {
	pathTraceShader = std::make_shared<Shader>((shadersDirectory + "vertex.glsl").c_str(), (shadersDirectory + "pathTrace.glsl").c_str());
	//outputShader = std::make_shared<Shader>((shadersDirectory + "vertex.glsl").c_str(), (shadersDirectory + "output.glsl").c_str());

	pathTraceShader->use();
	pathTraceShader->setInt("nTriangles", trianglesAttrib.size() / 12);
	pathTraceShader->setInt("width", renderWidth);
	pathTraceShader->setInt("height", renderHeight);
	pathTraceShader->setVec3("cameraPos", camera->Position);
	pathTraceShader->setMat4("cameraPos", glm::mat4(1.f));
	pathTraceShader->setInt("triangleTex", 1);
	pathTraceShader->setInt("nodesTex", 2);

}

void Render::draw(unsigned int& frameCounter) {

	quad->Draw(pathTraceShader, frameCounter);
}

#endif // !RENDER_H
