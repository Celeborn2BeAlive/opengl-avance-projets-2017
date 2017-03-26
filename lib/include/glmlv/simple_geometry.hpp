#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace glmlv
{

struct Vertex3f3f3f
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 Color;

    Vertex3f3f3f() = default;

    Vertex3f3f3f(glm::vec3 position, glm::vec3 normal, glm::vec3 Color):
        position(position), normal(normal), Color(Color)
    {
    }
};

struct SimpleGeometry
{
    std::vector<Vertex3f3f3f> vertexBuffer;
    std::vector<uint32_t> indexBuffer;
};

SimpleGeometry makeTriangle();
SimpleGeometry makeCube(glm::vec3 t,glm::vec3 s,glm::vec3 c);
SimpleGeometry makeScene();
// Pass a number of subdivision to apply on the longitude of the sphere
SimpleGeometry makeSphere(uint32_t subdivLongitude, glm::vec3 c);

}
