#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "material.h"

class Triangle
{
public:
    Triangle() {}
    Triangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, std::shared_ptr<Material> m) : p1(v1), p2(v2), p3(v3), material(m) {
        n1 = p1;
        n2 = p2;
        n3 = p3;
        center = (p1 + p2 + p3) / glm::vec3(3.0);
    }

    glm::vec3 p1, p2, p3;
    glm::vec3 n1, n2, n3;
    glm::vec3 center;
    std::shared_ptr<Material> material;
};
#endif // !TRIANGLE_H
