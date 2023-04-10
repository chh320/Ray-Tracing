#ifndef SCENE_H
#define SCENE_H

#include "mesh.h"
#include "bvh.h"
#include "material.h"
#include "triangle.h"

using namespace std;

class Scene {
public:
	Scene(){}
	Scene(const std::string& objectDirectory);

	std::vector<Triangle> triangles;
	std::vector<glm::vec3> trianglesAttrib;
	std::vector<glm::vec3> bvhNodes;
};


Scene::Scene(const std::string& objectDirectory) {
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, 0.4f, 0.f));
	model = glm::scale(model, glm::vec3(0.1f));

	shared_ptr<Material> m = make_shared<Material>();
	m->emissive = glm::vec3(1.0, 0.8, 0.0);
	shared_ptr<Mesh> sphere = make_shared<Mesh>(objectDirectory + "sphere2.obj",		m, model);
	shared_ptr<Mesh> bunny  = make_shared<Mesh>(objectDirectory + "Stanford Bunny.obj", make_shared<Material>());
	shared_ptr<Mesh> floor  = make_shared<Mesh>(objectDirectory + "quad2.obj", make_shared<Material>());

	std::vector<Triangle>().swap(triangles);
	triangles.insert(triangles.end(), bunny->triangles.begin(), bunny->triangles.end());
	triangles.insert(triangles.end(), sphere->triangles.begin(), sphere->triangles.end());
	triangles.insert(triangles.end(), floor->triangles.begin(), floor->triangles.end());

	shared_ptr<Bvh> bvh = std::make_shared<Bvh>(triangles, 0, triangles.size() - 1, 8);

	std::vector<glm::vec3>().swap(trianglesAttrib);
	for (int i = 0; i < triangles.size(); i++) {
		Triangle& t = triangles[i];
		trianglesAttrib.emplace_back(t.p1);  // vertices
		trianglesAttrib.emplace_back(t.p2);  
		trianglesAttrib.emplace_back(t.p3);  
		trianglesAttrib.emplace_back(t.n1);  // normal
		trianglesAttrib.emplace_back(t.n2);  
		trianglesAttrib.emplace_back(t.n3);  
		trianglesAttrib.emplace_back(t.material->emissive);
		trianglesAttrib.emplace_back(t.material->baseColor);
		trianglesAttrib.emplace_back(glm::vec3(t.material->subsurface,     t.material->metallic,  t.material->specular));
		trianglesAttrib.emplace_back(glm::vec3(t.material->specularTint,   t.material->roughness, t.material->anisotropic));
		trianglesAttrib.emplace_back(glm::vec3(t.material->sheen,		   t.material->sheenTint, t.material->clearcoat));
		trianglesAttrib.emplace_back(glm::vec3(t.material->clearcoatGloss, t.material->IOR,		  t.material->transmission));
	}

	std::vector<glm::vec3>().swap(bvhNodes);
	for (int i = 0; i < bvh->nodes.size(); i++) {
		bvhNodes.emplace_back(bvh->nodes[i].left, bvh->nodes[i].right, bvh->nodes[i].padding1);
		bvhNodes.emplace_back(bvh->nodes[i].n,    bvh->nodes[i].index, bvh->nodes[i].padding2);
		bvhNodes.emplace_back(bvh->nodes[i].Pmin);
		bvhNodes.emplace_back(bvh->nodes[i].Pmax);
	}
}
#endif // !SCENE_H
