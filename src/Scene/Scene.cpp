#include "Scene.hpp"
#include "Renderer/Renderer.hpp"
#include "Cube.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <limits>


Scene::Scene(std::vector<Object> objects, std::vector<Mesh> meshes, std::vector<Material> materials, std::vector<Texture> textures)
:   m_objects(std::move(objects)), m_meshes(std::move(meshes)), m_materials(std::move(materials)), m_textures(std::move(textures)),
    m_camera(), m_size(), m_center(), m_view(), m_projection()
{
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

    for (auto const& m: m_meshes) {
        min = glm::min(min, m.min());
        max = glm::max(max, m.max());
    }

    m_size = glm::length(max - min);
    m_center = (max + min) / 2.f;
    m_camera.setNear(m_size * 0.01f);
    m_camera.setFar(m_size);
    // m_camera.far(5.f * m_size);
}

Scene::~Scene() {
}

void Scene::update() {
    m_view = m_camera.view();
    m_projection = m_camera.projection();
}

void Scene::draw(Renderer const& renderer, DrawType type) const {
    for (auto const& o: m_objects) {
        switch (type) {
        case DRAW_ALL:
            o.drawAll(renderer, *this, m_view);
            break;

        case DRAW_SHADOWS:
            o.drawShadows(renderer, *this, renderer.light().viewproj(*this));
            break;

        case DRAW_RSM:
            o.drawRSM(renderer, *this, m_view, renderer.light().viewproj(*this));
            break;
        }
    }
}