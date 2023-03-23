#ifndef LOADER_H
#define LOADER_H

#include <Scene.h>

namespace GLSLPT
{
	class Scene;

	bool LoadSceneFromFile(const std::string& filename, Scene* scene, RenderOptions& renderOptions);
}
#endif // !LOADER_H
