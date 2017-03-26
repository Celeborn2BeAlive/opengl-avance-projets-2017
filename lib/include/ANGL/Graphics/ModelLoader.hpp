/**
 * @file Provides the implementation for a model loader.
 * A model loader responsibility is to load the textures and vertex geometry from an external file.
 * The current implementation wil rely on TinyObj Loader
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>

#include <ANGL/Graphics/ModelGeometry.hpp>
#include <tiny_obj_loader.h>

namespace ANGL {

    class  ModelLoader {

    public:

        static void Load(ModelGeometry &pGeometry, std::string &pFilename, std::string &pBasepath);

    private:

        static void LoadTextures(std::vector<tinyobj::material_t> pMaterials, std::map<std::string, GLuint> &pTexturesIDs, std::string &pBasepath);
        static void LoadMaterials(ModelGeometry &pGeometry, std::vector<tinyobj::material_t> pMaterials, std::map<std::string, GLuint> &pTexturesIDs);
        static void LoadGeometry(ModelGeometry &pGeometry, tinyobj::attrib_t pAttrib, std::vector<tinyobj::shape_t> pShapes);
    };

} // ANGL