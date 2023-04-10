#ifndef MATERIAL_H
#define MATERIAL_H

class Material {
public:
    Material() {
        emissive = glm::vec3(0.0f);
        baseColor = glm::vec3(1.0f);

        subsurface = 0.0f;
        metallic = 0.0f;
        specular = 0.0f;

        specularTint = 0.0f;
        roughness = 0.0f;
        anisotropic = 0.0f;

        sheen = 0.0f;
        sheenTint = 0.0f;
        clearcoat = 0.0f;

        clearcoatGloss = 0.f;
        IOR = 1.5f;
        transmission = 0.f;
    }
public:
    glm::vec3 emissive;
    glm::vec3 baseColor;

    float subsurface;
    float metallic;
    float specular;

    float specularTint;
    float roughness;
    float anisotropic;

    float sheen;
    float sheenTint;
    float clearcoat;

    float clearcoatGloss;
    float IOR;
    float transmission;
};
#endif // !MATERIAL_H
