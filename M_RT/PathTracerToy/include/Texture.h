#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include <algorithm>
#include <string>

namespace GLSLPT
{
	class Texture
	{
	public:

		Texture() : width(0), height(0), components(0) {};
		Texture(std::string texName, unsigned char* data, int w, int h, int c);
		~Texture() { }

		bool LoadTexture(const std::string& filename);

		int width;
		int height;
		int components;
		std::vector<unsigned char> texData;
		std::string name;
	};
}
#endif // !TEXTURE_H
