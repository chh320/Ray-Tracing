#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Ray
{
public:
	Ray(){}
	Ray(const glm::vec3& o, const glm::vec3& d): ori(o), dir(d){}
	glm::vec3 origin() const { return ori; }
	glm::vec3 direction() const { return dir; }

	glm::vec3 GetRayLocation(float t) {
		return ori + t * dir;
	}

public:
	glm::vec3 ori;
	glm::vec3 dir;
};
#endif