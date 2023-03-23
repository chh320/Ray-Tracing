#ifndef ENVIRONMENTMAP_H
#define ENVIRONMENTMAP_H

#include <vector>
#include <string>
#include <MathUtils.h>
#include <stb_image/stb_image.h>

namespace GLSLPT
{
	class EnvironmentMap
	{
	public:
		EnvironmentMap() : width(0), height(0), img(nullptr), cdf(nullptr) {};
		~EnvironmentMap() { stbi_image_free(img); delete[] cdf; }

		bool LoadMap(const std::string& filename);
		void BuildCDF();

		int width;
		int height;
		float totalSum;
		float* img;
		float* cdf;
	};
}
#endif // !ENVIRONMENTMAP_H
