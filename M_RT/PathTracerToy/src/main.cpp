#include <time.h>
#include <math.h>
#include <string>

#include "SDL2/SDL.h"
#include "GL/gl3w.h"
#include <tinydir/tinydir.h>

#include <Scene.h>
#include <Loader.h>
#include <GLTFLoader.h>
#include <Renderer.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include<stb_image/stb_image.h>
#include <stb_image/stb_image_write.h>

using namespace std;
using namespace GLSLPT;

Scene* scene = nullptr;
Renderer* renderer = nullptr;

std::vector<string> sceneFiles;
std::vector<string> envMaps;

float mouseSensitivity = 0.01f;
bool keyPressed = false;
int sampleSceneIdx = 0;
int selectedInstance = 0;
double lastTime = SDL_GetTicks();
int envMapIdx = 0;
bool done = false;

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
        if (ext == "scene")
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

void LoadScene(std::string sceneName)
{
    delete scene;
    scene = new Scene();
    std::string ext = sceneName.substr(sceneName.find_last_of(".") + 1);

    bool success = false;
    Mat4 xform;

    if (ext == "scene")
        success = LoadSceneFromFile(sceneName, scene, renderOptions);

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

bool InitRenderer()
{
    delete renderer;
    renderer = new Renderer(scene, shadersDir);
    return true;
}

void SaveFrame(const std::string filename)
{
    unsigned char* data = nullptr;
    int w, h;
    renderer->GetOutputBuffer(&data, w, h);
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filename.c_str(), w, h, 4, data, w * 4);
    printf("Frame saved: %s\n", filename.c_str());
    delete[] data;
}

void Render()
{
    renderer->Render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, renderOptions.windowResolution.x, renderOptions.windowResolution.y);
    renderer->Present();
}

void Update(float secondsElapsed)
{
    keyPressed = false;
    renderer->Update(secondsElapsed);
}


void MainLoop(void* arg)
{
    LoopData& loopdata = *(LoopData*)arg;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        //ImGui_ImplSDL2_ProcessEvent(&event);
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
    Update((float)(presentTime - lastTime));
    lastTime = presentTime;
    glClearColor(0., 0., 0., 0.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    Render();
    SDL_GL_SwapWindow(loopdata.mWindow);
}

int main(int argc, char** argv)
{
    srand((unsigned int)time(0));

    std::string sceneFile;

    GetSceneFiles();
    GetEnvMaps();
    LoadScene(sceneFiles[sampleSceneIdx]);

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    LoopData loopdata;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0, &current);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    loopdata.mWindow = SDL_CreateWindow("GLSL PathTracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderOptions.windowResolution.x, renderOptions.windowResolution.y, window_flags);

    // Query actual drawable window size
    int w, h;
    SDL_GL_GetDrawableSize(loopdata.mWindow, &w, &h);
    renderOptions.windowResolution.x = w;
    renderOptions.windowResolution.y = h;

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    loopdata.mGLContext = SDL_GL_CreateContext(loopdata.mWindow);
    if (!loopdata.mGLContext)
    {
        fprintf(stderr, "Failed to initialize GL context!\n");
        return 1;
    }
    SDL_GL_SetSwapInterval(0); // Disable vsync

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    if (!InitRenderer())
        return 1;

    while (!done)
    {
        MainLoop(&loopdata);
    }

    delete renderer;
    delete scene;

    SDL_GL_DeleteContext(loopdata.mGLContext);
    SDL_DestroyWindow(loopdata.mWindow);
    SDL_Quit();
    return 0;
}

