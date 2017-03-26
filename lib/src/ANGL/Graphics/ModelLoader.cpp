#include <ANGL/Graphics/ModelLoader.hpp>

#include <glmlv/Image2DRGBA.hpp>
#include <tiny_obj_loader.h>


namespace ANGL {


    void ModelLoader::Load(ModelGeometry &pGeometry, std::string & pFilename, std::string & pBasepath)
    {
        tinyobj::attrib_t aAttrib;
        std::vector<tinyobj::shape_t> aShapes;
        std::vector<tinyobj::material_t> aMaterials;

        std::string err;
        bool ret = tinyobj::LoadObj(&aAttrib, &aShapes, &aMaterials, &err, pFilename.c_str(), pBasepath.c_str(), true);

        if (!err.empty()) { // `err` may contain warning message.
            std::cerr << err << std::endl;
        }

        if (!ret) {
            exit(1);
        }

        std::map<std::string, GLuint> aTexturesIDs;

        LoadTextures(aMaterials, aTexturesIDs, pBasepath);
        LoadMaterials(pGeometry, aMaterials, aTexturesIDs);

        LoadGeometry(pGeometry, aAttrib, aShapes);
    }


    static bool FileExists(const std::string &abs_filename) {
        bool ret;
        FILE *fp = fopen(abs_filename.c_str(), "rb");
        if (fp) {
            ret = true;
            fclose(fp);
        } else {
            ret = false;
        }

        return ret;
    }

    void LoadTexture(std::string & pTextureName, std::string &pBasepath, std::map<std::string, GLuint> &pTexturesIDs)
    {
        if (pTextureName.length() > 0) {
            // Only load the texture if it is not already loaded
            if (pTexturesIDs.find(pTextureName) == pTexturesIDs.end()) {

                std::string texture_filename = pTextureName;
                if (!FileExists(texture_filename)) {
                    // Append base dir.
                    texture_filename = pBasepath + pTextureName;
                    if (!FileExists(texture_filename)) {
                        std::cerr << "Unable to find file: " << pTextureName << std::endl;
                        exit(1);
                    }
                }

                auto image = glmlv::readImage(pBasepath + pTextureName);
                GLuint texture_id;

                glCreateTextures( GL_TEXTURE_2D, 1,&texture_id);
                glTextureStorage2D(texture_id, 1, GL_RGB32F, image.width(), image.height());
                glTextureSubImage2D(texture_id, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.data());

                pTexturesIDs.insert(std::make_pair(pTextureName, texture_id));
            }
        }
    }


    void ModelLoader::LoadTextures(std::vector<tinyobj::material_t> pMaterials, std::map<std::string, GLuint> &pTexturesIDs, std::string &pBasepath)
    {
        for (size_t m = 0; m < pMaterials.size(); ++m)
        {
            tinyobj::material_t* mp = &pMaterials[m];

            LoadTexture(mp->ambient_texname, pBasepath, pTexturesIDs);
            LoadTexture(mp->diffuse_texname, pBasepath, pTexturesIDs);
            LoadTexture(mp->specular_texname, pBasepath, pTexturesIDs);
            LoadTexture(mp->specular_highlight_texname, pBasepath, pTexturesIDs);
            LoadTexture(mp->alpha_texname, pBasepath, pTexturesIDs);
            LoadTexture(mp->bump_texname, pBasepath, pTexturesIDs);
        }
    }


    void ModelLoader::LoadMaterials(ModelGeometry &pGeometry, std::vector<tinyobj::material_t> pMaterials, std::map<std::string, GLuint> &pTexturesIDs)
    {
        for (const tinyobj::material_t & aMaterial : pMaterials)
        {
            PhongMaterial phongMaterial;

            phongMaterial.Ka = glm::vec3(aMaterial.ambient[0], aMaterial.ambient[1], aMaterial.ambient[2]);
            phongMaterial.Kd = glm::vec3(aMaterial.diffuse[0], aMaterial.diffuse[1], aMaterial.diffuse[2]);
            phongMaterial.Ks = glm::vec3(aMaterial.specular[0], aMaterial.specular[1], aMaterial.specular[2]);
            phongMaterial.shininess = aMaterial.shininess;
            phongMaterial.dissolve = aMaterial.dissolve;

            if (!aMaterial.ambient_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.ambient_texname);
                phongMaterial.KaTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }
            if (!aMaterial.diffuse_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.diffuse_texname);
                phongMaterial.KdTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }
            if (!aMaterial.specular_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.specular_texname);
                phongMaterial.KsTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }
            if (!aMaterial.specular_highlight_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.specular_highlight_texname);
                phongMaterial.shininessTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }
            if (!aMaterial.alpha_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.alpha_texname);
                phongMaterial.dissolveTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }

            if (!aMaterial.bump_texname.empty()) {
                const auto it = pTexturesIDs.find(aMaterial.bump_texname);
                phongMaterial.normalTextureId = it != end(pTexturesIDs) ? (*it).second : -1;
            }

            pGeometry.mMaterials.push_back(phongMaterial);
        }
    }


    void ModelLoader::LoadGeometry(ModelGeometry &pGeometry, tinyobj::attrib_t pAttrib, std::vector<tinyobj::shape_t> pShapes)
    {
//        std::vector<Vertex3f3f3f2f> vertexBuffer;
//        std::vector<std::vector<int32_t>> indexBuffers;
//        std::vector<uint32_t> mMaterialIndexes;

        // Read vertices properties
        for(size_t s = 0; s < pShapes.size(); ++s) {
            std::vector<int32_t> indexBuffer;
            std::vector<Vertex3f3f3f2f> vertices;

            std::map<uint32_t, std::vector<int32_t >> mvp;

            for (size_t i = 0; i < pShapes[s].mesh.indices.size(); ++i) {

                tinyobj::index_t idx = pShapes[s].mesh.indices[i];

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
                            pAttrib.vertices[idx.vertex_index * 3 + 0],
                            pAttrib.vertices[idx.vertex_index * 3 + 1],
                            pAttrib.vertices[idx.vertex_index * 3 + 2]
                    };

                if(idx.normal_index >= 0)
                    normal = {
                            pAttrib.normals[idx.normal_index * 3 + 0],
                            pAttrib.normals[idx.normal_index * 3 + 1],
                            pAttrib.normals[idx.normal_index * 3 + 2]
                    };

                if(idx.texcoord_index >= 0)
                    texCoord = {
                            pAttrib.texcoords[idx.texcoord_index * 2 + 0],
                            1.f - pAttrib.texcoords[idx.texcoord_index * 2 + 1]
                    };

                if(mvp.find(idx.vertex_index) == mvp.end()) {
                    mvp.insert(std::make_pair(idx.vertex_index, std::vector<int32_t>()));
                }
                mvp[idx.vertex_index].push_back(vertices.size());

                vertex = { position, normal, tangent, texCoord };
                vertices.push_back(vertex);
            }

            // Calculate vertices tangents
            // For each triangle/face
            for (size_t i = 0; i < vertices.size(); i += 3) {

                tinyobj::index_t idx0 = pShapes[s].mesh.indices[i];
                tinyobj::index_t idx1 = pShapes[s].mesh.indices[i + 1];
                tinyobj::index_t idx2 = pShapes[s].mesh.indices[i + 2];

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

                for(size_t v = 0; v < mvp[idx0.vertex_index].size(); ++v)
                    vertices[mvp[idx0.vertex_index][v]].tangent += tangent;

                for(size_t v = 0; v < mvp[idx1.vertex_index].size(); ++v)
                    vertices[mvp[idx1.vertex_index][v]].tangent += tangent;

                for(size_t v = 0; v < mvp[idx2.vertex_index].size(); ++v)
                    vertices[mvp[idx2.vertex_index][v]].tangent += tangent;
            }

            for (unsigned int i = 0 ; i < vertices.size() ; i++) {
                vertices[i].tangent = glm::normalize(vertices[i].tangent);
            }

            // Send vertices and indices to "buffers"
            for(size_t v = 0; v < vertices.size(); ++v) {
                indexBuffer.push_back(pGeometry.mVertexBuffer.size());
                pGeometry.mVertexBuffer.push_back(vertices[v]);
            }

            pGeometry.mIndexBuffers.push_back(indexBuffer);
            pGeometry.mMaterialIndexes.push_back(pShapes[s].mesh.material_ids[pShapes[s].mesh.material_ids.size() - 1]);
        }
    }

} // namespace ANGL


