#pragma once

#include <vector>
#include <map>

class Object;
class Mesh;
class Material;
class Texture;

class Importer {
public:
    struct Data {
        Data();
        ~Data();

        std::vector<Object> objects;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<Texture> textures;
        std::map<std::string, unsigned int> tkeys;
    };

    // Load the given file.
    static void load(Data& data, std::string const& path); 
    static unsigned int loadTexture(Data& data, std::string const& path);

    // Set verbose mode.
    static void verbose(bool state) { m_verbose = state; }

private:
    static bool m_verbose;
};