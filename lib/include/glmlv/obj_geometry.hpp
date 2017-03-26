#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>

namespace glmlv
{

struct Vertex3f3f3f2f
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoords;

    Vertex3f3f3f2f() = default;

    Vertex3f3f3f2f(glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoords):
            position(position), normal(normal), tangent(tangent), texCoords(texCoords)
    {
    }

//    Vertex3f3f3f2f(Vertex3f3f2f v, glm::vec3 tangent):
//            position(v.position), normal(v.normal), tangent(tangent), texCoords(v.texCoords)
//    {
//    }
};

struct ObjGeometry
{
    std::vector<Vertex3f3f3f2f> vertexBuffer;
    std::vector<std::vector<int32_t>> indexBuffers;
    std::vector<uint32_t> materialIndexes;

};

ObjGeometry loadObj(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes);

}