#ifndef GLTFLOADER_H
#define GLTFLOADER_H

#include <Scene.h>

namespace GLSLPT
{
	class Scene;

	bool LoadGLTF(const std::string& filename, Scene* scene, RenderOptions& renderOptions, Mat4 xform, bool binary);
}
#endif // !GLTFLOADER_H
