#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <tinydir/tinydir.h>

#include <Mat4.h>
#include <Scene.h>
#include <Loader.h>
#include <GLTFLoader.h>

using namespace std;
using namespace GLSLPT;

float mouseSensitivity = 0.01f;
bool keyPressed = false;
int sampleSceneIdx = 0;
int selectedInstance = 0;
int envMapIdx = 0;
bool done = false;
double lastTime = SDL_GetTicks();

std::vector<string> sceneFiles;
std::vector<string> envMaps;

std::string shadersDir = "./assets/shaders/";
std::string assetsDir = "./assets/";
std::string envMapDir = "./assets/HDR/";

RenderOptions renderOptions;

struct LoopData
{
	SDL_Window* mWindow = nullptr;
	SDL_GLContext mGLContext = nullptr;
};

void GetSceneFiles()
{
	tinydir_dir dir;
	int i;
	tinydir_open_sorted(&dir, assetsDir.c_str());

	for (i = 0; i < dir.n_files; i++)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);

		std::string ext = std::string(file.extension);
		if (ext == "scene" || ext == "gltf" || ext == "glb")
		{
			sceneFiles.push_back(assetsDir + std::string(file.name));
		}
	}

	tinydir_close(&dir);
}

void GetEnvMaps()
{
	tinydir_dir dir;
	int i;
	tinydir_open_sorted(&dir, envMapDir.c_str());

	for (i = 0; i < dir.n_files; i++)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);

		std::string ext = std::string(file.extension);
		if (ext == "hdr")
		{
			envMaps.push_back(envMapDir + std::string(file.name));
		}
	}

	tinydir_close(&dir);
}

void LoadScene(Scene* scene, std::string sceneName)
{
	delete scene;
	scene = new Scene();
	std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);

	bool success = false;
	Mat4 xform;

	if (ext == "scene")
		success = LoadSceneFromFile(sceneName, scene, renderOptions);
	else if(ext == "gltf")
		success = LoadGLTF(sceneName, scene, renderOptions, xform, false);
	else if(ext == "glb")
		success = LoadGLTF(sceneName, scene, renderOptions, xform, true);

	if (!success)
	{
		printf("Unable to load scene\n");
		exit(0);
	}

	//loadCornellTestScene(scene, renderOptions);
	selectedInstance = 0;

	// Add a default HDR if there are no lights in the scene
	if (!scene->envMap && !envMaps.empty())
	{
		scene->AddEnvMap(envMaps[envMapIdx]);
		renderOptions.enableEnvMap = scene->lights.empty() ? true : false;
		renderOptions.envMapIntensity = 1.5f;
	}
	
	scene->renderOptions = renderOptions;
}

bool InitRenderer(Renderer* renderer, Scene* scene)
{
	delete renderer;
	renderer = new Renderer(scene, shadersDir);
	return true;
}

void Update(Renderer* renderer, float secondsElapsed)
{
	keyPressed = false;
	renderer->Update(secondsElapsed);
}

void Render(Renderer* renderer)
{
	renderer->Render();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, renderOptions.windowResolution.x, renderOptions.windowResolution.y);
	renderer->Present();
}

void MainLoop(void* arg, Scene* scene, Renderer* renderer)
{
	LoopData& loopdata = *(LoopData*)arg;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			done = true;
		}

		if (event.type == SDL_WINDOWEVENT)
		{
			if (event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				renderOptions.windowResolution = iVec2(event.window.data1, event.window.data2);
				int w, h;
				SDL_GL_GetDrawableSize(loopdata.mWindow, &w, &h);
				renderOptions.windowResolution.x = w;
				renderOptions.windowResolution.y = h;

				if (!renderOptions.independentRenderSize)
					renderOptions.renderResolution = renderOptions.windowResolution;

				scene->renderOptions = renderOptions;
				renderer->ResizeRenderer();
			}
			if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(loopdata.mWindow))
			{
				done = true;
			}
		}
	}
	double presentTime = SDL_GetTicks();
	Update(renderer, (float)(presentTime - lastTime));
	lastTime = presentTime;
	glClearColor(0., 0., 0., 0.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	Render(renderer);
	SDL_GL_SwapWindow(loopdata.mWindow);
}
#endif // !UTIL_H
