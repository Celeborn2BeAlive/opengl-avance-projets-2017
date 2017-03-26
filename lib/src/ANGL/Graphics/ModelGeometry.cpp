#include <ANGL/Graphics/ModelGeometry.hpp>
#include <ANGL/Graphics/Renderer.hpp>

namespace ANGL {


    void ModelGeometry::Draw()
    {
        for(size_t s = 0; s < mVAOs.size(); ++s) {
            glBindVertexArray(mVAOs[s]);
            // Get material properties
            int32_t matIndex = mMaterialIndexes[s];

            glm::vec3 Ka = mMaterials[matIndex].Ka;
            GLuint KaTextureId = mMaterials[matIndex].KaTextureId;
            bool KaMap = KaTextureId != -1;

            glm::vec3 Kd = mMaterials[matIndex].Kd;
            GLuint KdTextureId = mMaterials[matIndex].KdTextureId;
            bool KdMap = KdTextureId != -1;

            glm::vec3 Ks = mMaterials[matIndex].Ks;
            GLuint KsTextureId = mMaterials[matIndex].KsTextureId;
            bool KsMap = KsTextureId != -1;

            float Ns = mMaterials[matIndex].shininess;
            GLuint NsTextureId = mMaterials[matIndex].shininessTextureId;
            bool NsMap = NsTextureId != -1;

            float d = mMaterials[matIndex].dissolve;
            GLuint dTextureId = mMaterials[matIndex].dissolveTextureId;
            bool dMap = dTextureId != -1;

            GLuint NormalTextureId = mMaterials[matIndex].normalTextureId;
            bool NormalMap = NormalTextureId != -1;

//                glActiveTexture(GL_TEXTURE0);
            glUniform3f(Renderer::getRenderer()->m_uKa_location, Ka[0], Ka[1], Ka[2]);
            glUniform1i(Renderer::getRenderer()->m_uKaMap_location, KaMap);
            if(KaMap)
                glBindTextureUnit(0, KaTextureId);

//                glActiveTexture(GL_TEXTURE1);
            glUniform3f(Renderer::getRenderer()->m_uKd_location, Kd[0], Kd[1], Kd[2]);
            glUniform1i(Renderer::getRenderer()->m_uKdMap_location, KdMap);
            if(KdMap)
                glBindTextureUnit(1, KdTextureId);

//                glActiveTexture(GL_TEXTURE2);
            glUniform3f(Renderer::getRenderer()->m_uKs_location, Ks[0], Ks[1], Ks[2]);
            glUniform1i(Renderer::getRenderer()->m_uKsMap_location, KsMap);
            if(KsMap)
                glBindTextureUnit(2, KsTextureId);

//                glActiveTexture(GL_TEXTURE3);
            glUniform1i(Renderer::getRenderer()->m_uNsMap_location, NsMap);
            glUniform1f(Renderer::getRenderer()->m_uNs_location, Ns);
            if(NsMap)
                glBindTextureUnit(3, NsTextureId);

//                glActiveTexture(GL_TEXTURE4);
            glUniform1i(Renderer::getRenderer()->m_udMap_location, dMap);
            glUniform1f(Renderer::getRenderer()->m_ud_location, d);
            if(dMap)
                glBindTextureUnit(4, dTextureId);

//                glActiveTexture(GL_TEXTURE5);
            glUniform1i(Renderer::getRenderer()->m_uNormalMap_location, NormalMap);
            if(NormalMap)
                glBindTextureUnit(5, NormalTextureId);

            glDrawElements(GL_TRIANGLES, mIndexBuffers[s].size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    void ModelGeometry::BuildBuffers()
    {
        ILogger::getLogger()->getStream(SeverityDebug, "ModelGeometry.cpp", __LINE__)
        << "ModelGeometry::BuildBuffer()"<< std::endl;

        Renderer* aRenderer = Renderer::getRenderer();

        glCreateBuffers(1, &mVBO);
        glNamedBufferStorage(mVBO, mVertexBuffer.size() * sizeof(Vertex3f3f3f2f), mVertexBuffer.data(), 0);

        // For each shape
        for(size_t i = 0; i < mIndexBuffers.size(); ++i) {

            // Build IBO for the shape
            GLuint IBO;
            glCreateBuffers(1, &IBO);
            glNamedBufferStorage(IBO, mIndexBuffers[i].size() * sizeof(int32_t), mIndexBuffers[i].data(), 0);
            mIBOs.push_back(IBO);

            // Create VAO and bind attributes
            GLuint VAO;
            glCreateVertexArrays(1, &VAO);

            glVertexArrayVertexBuffer(VAO, Renderer::gVBOBindingIndex, mVBO, 0, sizeof(Vertex3f3f3f2f));

            glVertexArrayAttribBinding(VAO, aRenderer->gPositionAttr_binding_geometry, Renderer::gVBOBindingIndex);
            glEnableVertexArrayAttrib(VAO, aRenderer->gPositionAttr_binding_geometry);
            glVertexArrayAttribFormat(VAO, aRenderer->gPositionAttr_binding_geometry, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex3f3f3f2f, position));

            glVertexArrayAttribBinding(VAO, aRenderer->gNormalAttr_binding, Renderer::gVBOBindingIndex);
            glEnableVertexArrayAttrib(VAO, aRenderer->gNormalAttr_binding);
            glVertexArrayAttribFormat(VAO, aRenderer->gNormalAttr_binding, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex3f3f3f2f, normal));

            glVertexArrayAttribBinding(VAO, aRenderer->gTangentAttr_binding, Renderer::gVBOBindingIndex);
            glEnableVertexArrayAttrib(VAO, aRenderer->gTangentAttr_binding);
            glVertexArrayAttribFormat(VAO, aRenderer->gTangentAttr_binding, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex3f3f3f2f, tangent));

            glVertexArrayAttribBinding(VAO, aRenderer->gTexCoordsAttr_binding, Renderer::gVBOBindingIndex);
            glEnableVertexArrayAttrib(VAO, aRenderer->gTexCoordsAttr_binding);
            glVertexArrayAttribFormat(VAO, aRenderer->gTexCoordsAttr_binding, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex3f3f3f2f, texCoords));

            glVertexArrayElementBuffer(VAO, IBO);
            mVAOs.push_back(VAO);
        }
    }
} // namespace ANGL