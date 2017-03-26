#include "Importer.hpp"
#include "Scene/Object.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <algorithm>
#include <stdexcept>
#include <iostream>

bool Importer::m_verbose = false;

static char const* textureString(aiTextureType type) {
    switch (type) {
        case aiTextureType_AMBIENT:     return "AMBIENT";
        case aiTextureType_DIFFUSE:     return "DIFFUSE";
        case aiTextureType_SPECULAR:    return "SPECULAR";
        case aiTextureType_SHININESS:   return "SHININESS";

        default:                        return "UNDEFINED";          
    }
}

static Object loadObject(aiNode const* node, unsigned int offset) {
    std::vector<unsigned int> meshes;
    meshes.reserve(node->mNumMeshes);

    for (unsigned int i = 0; i < node->mNumMeshes; ++ i) {
        meshes.emplace_back(offset + node->mMeshes[i]);
    }

    aiMatrix4x4 const& trs = node->mTransformation;
    glm::mat4 transformations{
        trs.a1, trs.b1, trs.c1, trs.d1,
        trs.a2, trs.b2, trs.c2, trs.d2,
        trs.a3, trs.b3, trs.c3, trs.d3,
        trs.a4, trs.b4, trs.c4, trs.d4
    };

    Object object(std::move(meshes), std::move(transformations));

    for (unsigned int c = 0; c < node->mNumChildren; ++ c) {
        object.addChild(loadObject(node->mChildren[c], offset));
    }

    return object;
}

void Importer::load(Data& data, std::string const& path) {
    Assimp::Importer importer;

    unsigned int componentsToRemove = 
        aiComponent_TANGENTS_AND_BITANGENTS |
        aiComponent_BONEWEIGHTS |
        aiComponent_LIGHTS |
        aiComponent_CAMERAS;

    int primitivesToRemove = 
        aiPrimitiveType_POINT |
        aiPrimitiveType_LINE;

    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, componentsToRemove);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, primitivesToRemove);

    int flags = 
        aiProcess_JoinIdenticalVertices | 
        aiProcess_Triangulate |
        aiProcess_RemoveComponent |
        aiProcess_GenNormals |
        aiProcess_SortByPType |
        aiProcess_FindInvalidData |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords |
        aiProcess_FlipUVs;
        

    if (m_verbose) std::cout << "Loading scene `" << path << "`.\n";

    aiScene const* scene = importer.ReadFile(path, flags);
    if (!scene) {
        throw std::runtime_error("Unable to import `" + path + "`.");
    }

    std::string directory = path.substr(0, path.find_last_of("/")) + '/';

    /* Materials. */
    unsigned int material_offset = data.materials.size();
    data.materials.reserve(material_offset + scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; ++ i) {
        aiMaterial* material = scene->mMaterials[i];

        if (m_verbose) {
            aiString name;
            material->Get(AI_MATKEY_NAME, name);
            std::cout << "Registering material n°" << (i + 1) << ": `" << name.C_Str() << "`.\n";
        }

        aiColor3D ka, kd, ks;
        float shininess;

        material->Get(AI_MATKEY_COLOR_AMBIENT, ka);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
        material->Get(AI_MATKEY_COLOR_SPECULAR, ks);
        material->Get(AI_MATKEY_SHININESS, shininess);

        int tka = -1, tkd = -1, tks = -1, tshininess = -1;

        /* Textures. */
        struct { aiTextureType type; int& id; } texs[] = {
            { aiTextureType_AMBIENT, tka },
            { aiTextureType_DIFFUSE, tkd },
            { aiTextureType_SPECULAR, tks },
            { aiTextureType_SHININESS, tshininess }
        };

        for (auto const& tex: texs) {
            if (material->GetTextureCount(tex.type)) {
                aiString name;
                material->Get(AI_MATKEY_TEXTURE(tex.type, 0), name);

                std::string tpath = directory + name.C_Str();
                std::replace(tpath.begin(), tpath.end(), '\\', '/');

                try {
                    tex.id = loadTexture(data, tpath);

                } catch(std::runtime_error const& e) {
                    std::cerr << e.what() << std::endl;
                    tex.id = -1;
                }

                if (m_verbose) std::cout << "Binding texture `" << name.C_Str() << "` as " << textureString(tex.type) << ".\n";
            }
        }

        data.materials.emplace_back(glm::vec3{ ka.r, ka.g, ka.b }, glm::vec3{ kd.r, kd.g, kd.b }, glm::vec3{ ks.r, ks.g, ks.b }, shininess, tka, tkd, tks, tshininess);
    }

    /* Meshes. */
    unsigned int mesh_offset = data.meshes.size();
    data.meshes.reserve(mesh_offset + scene->mNumMeshes);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++ m) {
        aiMesh* mesh = scene->mMeshes[m];

        // if (m_verbose) std::cout << "Registering mesh `" << mesh->mName.C_Str() << "` (mat n°" << mesh->mMaterialIndex << ").\n";

        /* Vertices. */
        std::vector<Mesh::vertex> vertices;
        vertices.reserve(mesh->mNumVertices);

        for (unsigned int v = 0; v < mesh->mNumVertices; ++ v) {
            aiVector3D vertex = mesh->mVertices[v];
            aiVector3D normal = mesh->mNormals[v];
            aiVector3D texcoords = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][v] : aiVector3D(0.f, 0.f, 0.f);

            Mesh::vertex dest;
            dest.position = { vertex.x, vertex.y, vertex.z };
            dest.normal = { normal.x, normal.y, normal.z };
            dest.texcoords = { texcoords.x, texcoords.y };
            
            vertices.emplace_back(std::move(dest));
        }

        /* Indices. */
        std::vector<unsigned int> indices;
        indices.reserve(mesh->mNumFaces * 3);

        for (unsigned int f = 0; f < mesh->mNumFaces; ++ f) {
            aiFace& face = mesh->mFaces[f];

            for (unsigned int i = 0; i < 3; ++ i) {
                indices.emplace_back(face.mIndices[i]);
            }
        }

        data.meshes.emplace_back(vertices, indices, material_offset + mesh->mMaterialIndex);
    }

    /* Objects. */
    data.objects.emplace_back(loadObject(scene->mRootNode, mesh_offset));
}

unsigned int Importer::loadTexture(Data& data, std::string const& path) {
    auto it = data.tkeys.find(path);
    if (it != data.tkeys.end()) {
        return it->second;
    }

    if (m_verbose) std::cout << "Loading texture `" << path << "`.\n";

    unsigned int next = data.textures.size();

    int width, height;
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, 0, 4);
    if (!pixels) {
        data.tkeys[path] = -1;
        throw std::runtime_error("Unable to import `" + path + "`.");
    }

    Texture texture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), pixels);
    data.textures.emplace_back(std::move(texture));
    stbi_image_free(pixels);

    data.tkeys[path] = next;
    return next;
}

Importer::Data::Data()
:   objects(), meshes(), materials(), textures(), tkeys()
{}

Importer::Data::~Data() {
}