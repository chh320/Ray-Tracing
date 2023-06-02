#ifndef RENDER_H
#define RENDER_H

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <vector>
#include <string>
#include <iomanip>

#include "Quad.h"
#include "mesh.h"
#include "material.h"
#include "bvh.h"
#include "scene.h"
#include "envmap.h"

#include <learnopengl/shader.h>
#include <hdrloader/hdrloader.h>

class Render
{
public:
	Render(){}
	Render(const std::string& shadersDirectory, const std::string& objectDirectory, const std::string& evnmapDirectory, const int& width, const int& height) : renderWidth(width), renderHeight(height){
		quad = std::make_shared<Quad>();
		
		Scene scene = Scene(objectDirectory);
		trianglesAttrib = scene.trianglesAttrib;
		bvhNodes = scene.bvhNodes;

		camera = std::make_shared<Camera>(glm::vec3(-0.f, 0.3f, 0.8));

		envMap = std::make_shared<Envmap>(evnmapDirectory + "blue_photo_studio_4k.hdr");
		envMap->generateHdrCache();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, envMap->envMapTex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, envMap->envMapCache);


		InitGPUDataBuffers();
		InitFBOs();
		InitShaders(shadersDirectory);
	}
	void Draw();
	void Update(unsigned int& frameCounter);
	void ResizeRenderer(const std::string& shadersDirectory);

public:
	std::shared_ptr<Quad> quad;
	std::shared_ptr<Mesh> bunny;
	std::shared_ptr<Mesh> sphere;
	std::shared_ptr<Mesh> floor;

	std::shared_ptr<Envmap> envMap;

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
	std::shared_ptr<Shader> tonemapShader;

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
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_2D, envMapTex);
}

void Render::InitFBOs() {
	glGenFramebuffers(1, &pathTraceFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);

	glGenTextures(1, &pathTraceTexture);
	glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pathTraceTexture, 0);

	glGenFramebuffers(1, &accumFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);

	glGenTextures(1, &accumTexture);
	glBindTexture(GL_TEXTURE_2D, accumTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, renderWidth, renderHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::InitShaders(const std::string& shadersDirectory) {
	pathTraceShader = std::make_shared<Shader>((shadersDirectory + "vertex.glsl").c_str(), (shadersDirectory + "pathTrace.glsl").c_str());
	outputShader    = std::make_shared<Shader>((shadersDirectory + "vertex.glsl").c_str(), (shadersDirectory + "output.glsl").c_str());
	tonemapShader   = std::make_shared<Shader>((shadersDirectory + "vertex.glsl").c_str(), (shadersDirectory + "tonemap.glsl").c_str());

	pathTraceShader->use();
	pathTraceShader->setInt("nTriangles", trianglesAttrib.size() / 12);
	pathTraceShader->setInt("hdrResolution", max(envMap->width, envMap->height));
	pathTraceShader->setInt("width", renderWidth);
	pathTraceShader->setInt("height", renderHeight);
	pathTraceShader->setVec3("cameraPos", camera->Position);
	pathTraceShader->setMat4("cameraRotate", glm::mat4(1.f));
	pathTraceShader->setInt("accumTex", 0);
	pathTraceShader->setInt("triangleTex", 1);
	pathTraceShader->setInt("nodesTex", 2);
	pathTraceShader->setInt("envMapTex", 3);
	pathTraceShader->setInt("envMapCache", 4);

	pathTraceShader->stopUse();

	outputShader->use();
	outputShader->setInt("accumTex", 0);
	outputShader->stopUse();

	tonemapShader->use();
	tonemapShader->setInt("accumTex", 0);
	tonemapShader->stopUse();
}

void Render::Draw() {
	glBindFramebuffer(GL_FRAMEBUFFER, pathTraceFBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, accumTexture);
	quad->Draw(pathTraceShader);

	glBindFramebuffer(GL_FRAMEBUFFER, accumFBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pathTraceTexture);
	quad->Draw(outputShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, accumTexture);
	quad->Draw(tonemapShader);
}

void Render::Update(unsigned int& frameCounter) {
	pathTraceShader->use();
	pathTraceShader->setInt("frameCounter", frameCounter++);
	pathTraceShader->stopUse();
}

void Render::ResizeRenderer(const std::string& shadersDirectory) {
	// Delete textures
	glDeleteTextures(1, &pathTraceTexture);
	glDeleteTextures(1, &accumTexture);

	// Delete FBOs
	glDeleteFramebuffers(1, &pathTraceFBO);
	glDeleteFramebuffers(1, &accumFBO);

	// Delete shaders
	pathTraceShader = nullptr;
	outputShader = nullptr;
	tonemapShader = nullptr;

	InitFBOs();
	InitShaders(shadersDirectory);
}
#endif // !RENDER_H
