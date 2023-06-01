#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "material.h"
#include "triangle.h"

class Mesh
{
public:
    Mesh(){}
	Mesh(const std::string& objectPath, std::shared_ptr<Material> m, glm::mat4 transform = glm::mat4(1.0f)){
		readObj(objectPath);
        material = m;
        triNums = points.size() / 3;
        for (int i = 0; i < triNums; i++) {
            triangles.emplace_back(glm::vec3(transform * glm::vec4(points[i * 3], 1.0)), 
                                    glm::vec3(transform * glm::vec4(points[i * 3 + 1], 1.0)), 
                                    glm::vec3(transform * glm::vec4(points[i * 3 + 2], 1.0)), 
                                    material);
        }
	}

	void readObj(const std::string& path);
    void Draw(std::shared_ptr<Shader> shader);

    int pointNums;
    int triNums;

    std::vector<Triangle> triangles;
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> texcoords;
	std::vector<glm::vec3> normals;

    std::shared_ptr<Material> material;

private:
    GLuint vao;
    GLuint vbo;
};

void Mesh::readObj(const std::string& path) {
    std::vector<glm::vec3> vectexPosition;
    std::vector<glm::vec2> vertexTexcoord;
    std::vector<glm::vec3> vectexNormal;

    std::vector<glm::ivec3> positionIndex;
    std::vector<glm::ivec3> texcoordIndex;
    std::vector<glm::ivec3> normalIndex;

    std::ifstream fin(path);
    std::string line;
    if (!fin.is_open())
    {
        std::cout << "file " << path << " can't open." << std::endl;
        exit(-1);
    }

    while (std::getline(fin, line))
    {
        std::istringstream sin(line);   
        std::string type;
        GLfloat x, y, z;
        int v0, vt0, vn0;   
        int v1, vt1, vn1;   
        int v2, vt2, vn2;   
        char slash;

        int slashCnt = 0;
        for (int i = 0; i < line.length(); i++) {
            if (line[i] == '/') slashCnt++;
        }

        sin >> type;
        if (type == "v") {
            sin >> x >> y >> z;
            vectexPosition.push_back(glm::vec3(x, y, z));
        }
        if (type == "vt") {
            sin >> x >> y;
            vertexTexcoord.push_back(glm::vec2(x, y));
        }
        if (type == "vn") {
            sin >> x >> y >> z;
            vectexNormal.push_back(glm::vec3(x, y, z));
        }
        if (type == "f") {
            if (slashCnt == 6) {
                sin >> v0 >> slash >> vt0 >> slash >> vn0;
                sin >> v1 >> slash >> vt1 >> slash >> vn1;
                sin >> v2 >> slash >> vt2 >> slash >> vn2;
            }
            else if (slashCnt == 3) {
                sin >> v0 >> slash >> vt0;
                sin >> v1 >> slash >> vt1;
                sin >> v2 >> slash >> vt2;
            }
            else {
                sin >> v0 >> v1 >> v2;
            }
            positionIndex.push_back(glm::ivec3(v0 - 1, v1 - 1, v2 - 1));
            //texcoordIndex.push_back(glm::ivec3(vt0 - 1, vt1 - 1, vt2 - 1));
            //normalIndex.push_back(glm::ivec3(vn0 - 1, vn1 - 1, vn2 - 1));
        }
    }

    for (int i = 0; i < positionIndex.size(); i++)
        for (int j = 0; j < 3; j++) {
                points.push_back(vectexPosition[positionIndex[i][j]]);
        }
            
}

void Mesh::Draw(std::shared_ptr<Shader> shader) {
    shader->use();
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, pointNums);
    glBindVertexArray(0);
}
#endif // !MESH_H
