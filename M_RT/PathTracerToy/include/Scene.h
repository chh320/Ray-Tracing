#ifndef  SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <map>

#include <Camera.h>

namespace GLSLPT {
	class Camera;

	enum LightType 
	{
		RectLight,
		SphereLight,
		DistantLight
	};

	struct Light {
		Vec3 position;
		Vec3 emission;
		Vec3 u;
		Vec3 v;
		float radius;
		float area;
		float type;
	};

	struct Indices {
		int x, y, z;
	};

	class Scene {
	public:
		RenderOptions renderOptions;
	};
}// GLSLPT
#endif // ! SCENE_H
