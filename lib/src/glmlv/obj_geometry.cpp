#include "tiny_obj_loader.h"

#include <glmlv/obj_geometry.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <tiny_obj_loader.h>

namespace glmlv {
    ObjGeometry loadObj(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes)
    {
//    unsigned long nb_vertices = attrib.vertices.size() / 3;
//    std::vector<Vertex3f3f2f> vertexBuffer(nb_vertices);
        std::vector<Vertex3f3f3f2f> vertexBuffer;
        std::vector<std::vector<int32_t>> indexBuffers;
        std::vector<uint32_t> materialIndexes;

        // Read vertices properties
        for(size_t s = 0; s < shapes.size(); ++s) {
            std::vector<int32_t> indexBuffer;
            std::vector<Vertex3f3f3f2f> vertices;

            std::map<uint32_t, std::vector<int32_t >> mvp;

            for (size_t i = 0; i < shapes[s].mesh.indices.size(); ++i) {

                tinyobj::index_t idx = shapes[s].mesh.indices[i];

                /*
                printf("attrib.vertices.size =%d\nattrib.normals.size =%d\nattrib.texcoords.size =%d\n",
                       attrib.vertices.size(), attrib.normals.size(), attrib.texcoords.size());

                printf("idx.vertex_index =%d\nidx.normal_index =%d\nidx.texcoord_index=%d\n",
                       idx.vertex_index, idx.normal_index, idx.texcoord_index);
                //*/
                Vertex3f3f3f2f vertex;
                glm::vec3 position;
                glm::vec3 normal(0.f, 0.f, 0.f);
                glm::vec3 tangent(0.f, 0.f, 0.f);
                glm::vec2 texCoord(0.f, 0.f);


                if(idx.vertex_index >= 0)
                    position = {
                            attrib.vertices[idx.vertex_index * 3 + 0],
                            attrib.vertices[idx.vertex_index * 3 + 1],
                            attrib.vertices[idx.vertex_index * 3 + 2]
                    };

                if(idx.normal_index >= 0)
                    normal = {
                            attrib.normals[idx.normal_index * 3 + 0],
                            attrib.normals[idx.normal_index * 3 + 1],
                            attrib.normals[idx.normal_index * 3 + 2]
                    };

                if(idx.texcoord_index >= 0)
                    texCoord = {
                            attrib.texcoords[idx.texcoord_index * 2 + 0],
                            1.f - attrib.texcoords[idx.texcoord_index * 2 + 1]
                    };

                if(mvp.find(idx.vertex_index) == mvp.end()) {
                    mvp.insert(std::make_pair(idx.vertex_index, std::vector<int32_t>()));
                }
                mvp[idx.vertex_index].push_back(vertices.size());

                vertex = { position, normal, tangent, texCoord };
                vertices.push_back(vertex);

//                indexBuffer.push_back(idx.vertex_index);
//                vertexBuffer[idx.vertex_index] = v;

//                indexBuffer.push_back(vertexBuffer.size());
//                vertexBuffer.push_back(vertex);
            }

            // Calculate vertices tangents
            // For each triangle/face
            for (size_t i = 0; i < vertices.size(); i += 3) {

                tinyobj::index_t idx0 = shapes[s].mesh.indices[i];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[i + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[i + 2];

                Vertex3f3f3f2f& v0 = vertices[mvp[idx0.vertex_index][0]];
                Vertex3f3f3f2f& v1 = vertices[mvp[idx1.vertex_index][0]];
                Vertex3f3f3f2f& v2 = vertices[mvp[idx2.vertex_index][0]];

                // Edges
                glm::vec3 e1 = v1.position - v0.position;
                glm::vec3 e2 = v2.position - v0.position;

                glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
                glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

                float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                glm::vec3 tangent = (e1 * deltaUV2.y   - e2 * deltaUV1.y) * r;
                glm::vec3 bitangent = (e2 * deltaUV1.x   - e1 * deltaUV2.x) * r;

                if(tangent.x < 0) tangent.x = - tangent.x;
//                if(bitangent.y < 0) bitangent.y = - bitangent.y;

                for(size_t v = 0; v < mvp[idx0.vertex_index].size(); ++v)
                    vertices[mvp[idx0.vertex_index][v]].tangent += tangent;

                for(size_t v = 0; v < mvp[idx1.vertex_index].size(); ++v)
                    vertices[mvp[idx1.vertex_index][v]].tangent += tangent;

                for(size_t v = 0; v < mvp[idx2.vertex_index].size(); ++v)
                    vertices[mvp[idx2.vertex_index][v]].tangent += tangent;

//                v0.tangent += tangent;
//                v1.tangent += tangent;
//                v2.tangent += tangent;

                // Smoothed tangents
//                v0.tangent += glm::normalize(glm::cross(bitangent, v0.normal));
//                v1.tangent += glm::normalize(glm::cross(bitangent, v1.normal));
//                v2.tangent += glm::normalize(glm::cross(bitangent, v2.normal));
            }

            for (unsigned int i = 0 ; i < vertices.size() ; i++) {
                vertices[i].tangent = glm::normalize(vertices[i].tangent);
            }

            // Send vertices and indices to "buffers"
            for(size_t v = 0; v < vertices.size(); ++v) {
                indexBuffer.push_back(vertexBuffer.size());
                vertexBuffer.push_back(vertices[v]);
            }

            indexBuffers.push_back(indexBuffer);
            materialIndexes.push_back(shapes[s].mesh.material_ids[shapes[s].mesh.material_ids.size() - 1]);
        }

        return {vertexBuffer, indexBuffers, materialIndexes};
    }
}