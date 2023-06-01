#ifndef ENVMAP_H
#define ENVMAP_H

class Envmap {
public:
	Envmap(const std::string& evnmapPath) {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        unsigned char* data = stbi_load(evnmapPath.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &envMapTex);
            glBindTexture(GL_TEXTURE_2D, envMapTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            std::cout << "Successed to load HDR image." << std::endl;
        }
        else
        {
            std::cout << "Failed to load HDR image." << std::endl;
        }
	}
	GLuint envMapTex;
};

#endif // !ENVMAP_H
