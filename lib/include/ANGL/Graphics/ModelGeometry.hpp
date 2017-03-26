/**
 * @file Provides the implementation for a geometry.
 * A geometry responsibility is to hold a vertex geometry and OpenGL buffers.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>

namespace ANGL {

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
    };

    struct PhongMaterial
    {
        glm::vec3 Ka = glm::vec3(0); // Ambient multiplier
        glm::vec3 Kd = glm::vec3(0); // Diffuse multiplier
        glm::vec3 Ks = glm::vec3(0); // Glossy multiplier
        float shininess = 0.f; // Glossy exponent
        float dissolve = 1.f; // Alpha

        // Indices in the textures vector:
        GLuint KaTextureId = -1;
        GLuint KdTextureId = -1;
        GLuint KsTextureId = -1;
        GLuint shininessTextureId = -1;
        GLuint dissolveTextureId = -1;
        GLuint normalTextureId = -1;
    };

    class ModelGeometry {

        friend class ModelLoader;

        public:
            void Draw();

            //void BindBuffers();
            void BuildBuffers();

        private:

            std::vector<Vertex3f3f3f2f> mVertexBuffer;
            std::vector<std::vector<int32_t>> mIndexBuffers;

            std::vector<PhongMaterial> mMaterials;
            std::vector<uint32_t> mMaterialIndexes;

            GLuint mVBO;
            std::vector<GLuint> mIBOs;
            std::vector<GLuint> mVAOs;
    };

} // namespace ANGL