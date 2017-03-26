#pragma once

#include "Scene/Camera.hpp"
#include "Scene/Object.hpp"
#include "Renderer/Light.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <cassert>

class Renderer;

class Scene {
public:
    enum DrawType {
        DRAW_ALL, DRAW_SHADOWS, DRAW_RSM
    };

    Scene(std::vector<Object> objects, std::vector<Mesh> meshes, std::vector<Material> materials, std::vector<Texture> textures);
    ~Scene();

    void update();
    void draw(Renderer const& renderer, DrawType type = DRAW_ALL) const;

    Camera& camera() { return m_camera; }
    float size() const { return m_size; }
    glm::vec3 const& center() const { return m_center; }

    Mesh const& getMesh(unsigned int index) const { 
        assert(index < m_meshes.size());
        return m_meshes[index]; 
    }

    Material const& getMaterial(unsigned int index) const { 
        assert(index < m_materials.size());
        return m_materials[index];
    }

    Texture const& getTexture(unsigned int index) const { 
        assert(index < m_textures.size());
        return m_textures[index]; 
    }

    glm::mat4 const& view() const { return m_view; }
    glm::mat4 const& projection() const { return m_projection; }


    Scene(Scene const&) = delete;
    Scene& operator=(Scene const&) = delete;

private:
    std::vector<Object> m_objects;
    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<Texture> m_textures;

    Camera m_camera;
    float m_size;
    glm::vec3 m_center;

    glm::mat4 m_view;
    glm::mat4 m_projection;
};