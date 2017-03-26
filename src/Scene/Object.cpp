#include "Object.hpp"
#include "Renderer/Renderer.hpp"
#include "Scene/Scene.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <iostream>

/***************************************************/
Texture::Texture(unsigned int width, unsigned int height, unsigned char* data) 
:   m_id(0)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(m_id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

Texture::Texture(Texture&& other) 
:   m_id(0)
{
    std::swap(m_id, other.m_id);
}

Texture::~Texture() {
    if (m_id) glDeleteTextures(1, &m_id);
}
/***************************************************/


/***************************************************/
Material::Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float shininess, int tka, int tkd, int tks, int tshininess)
:   m_kcolors{ std::move(ka), std::move(kd), std::move(ks) }, m_shininess(shininess), m_textures{ tka, tkd, tks, tshininess }
{}
/***************************************************/


/***************************************************/
Mesh::Mesh(std::vector<vertex> const& vertices, std::vector<unsigned int> const& indices, unsigned int material)
:   m_material(material), m_size(indices.size()), m_min(std::numeric_limits<float>::max()), m_max(std::numeric_limits<float>::lowest()),
    m_vbo(0), m_ibo(0), m_vao(0)
{
    /* VBO. */
    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, sizeof(vertex) * vertices.size(), vertices.data(), 0);

    /* IBO. */
    glCreateBuffers(1, &m_ibo);
    glNamedBufferStorage(m_ibo, sizeof(unsigned int) * indices.size(), indices.data(), 0);

    /* VAO. */
    glCreateVertexArrays(1, &m_vao);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(vertex));
    glVertexArrayElementBuffer(m_vao, m_ibo);

    for (auto const& v: vertices) {
        m_min = glm::min(m_min, v.position);
        m_max = glm::max(m_max, v.position);
    }
}

Mesh::Mesh(Mesh&& other)
:   m_material(other.m_material), m_size(other.m_size), m_min(other.m_min), m_max(other.m_max), m_vbo(0), m_ibo(0), m_vao(0)
{
    std::swap(m_vbo, other.m_vbo);
    std::swap(m_ibo, other.m_ibo);
    std::swap(m_vao, other.m_vao);
}

Mesh::~Mesh() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_ibo) glDeleteBuffers(1, &m_ibo);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void Mesh::configure(Renderer const& renderer) {
    GLuint position = renderer.attribute(Renderer::POSITION);
    glVertexArrayAttribBinding(m_vao, position, 0);
    glEnableVertexArrayAttrib(m_vao, position);
    glVertexArrayAttribFormat(m_vao, position, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, position));

    GLuint normal = renderer.attribute(Renderer::NORMAL);
    glVertexArrayAttribBinding(m_vao, normal, 0);
    glEnableVertexArrayAttrib(m_vao, normal);
    glVertexArrayAttribFormat(m_vao, normal, 3, GL_FLOAT, GL_FALSE, offsetof(vertex, normal));

    GLuint texcoords = renderer.attribute(Renderer::TEXCOORDS);
    glVertexArrayAttribBinding(m_vao, texcoords, 0);
    glEnableVertexArrayAttrib(m_vao, texcoords);
    glVertexArrayAttribFormat(m_vao, texcoords, 2, GL_FLOAT, GL_FALSE, offsetof(vertex, texcoords));
}

void Mesh::drawAll(Renderer const& renderer, Scene const& scene) const {
    glBindVertexArray(m_vao);

    Material const& material = scene.getMaterial(m_material);
    glUniform3fv(renderer.uniform(Renderer::KA), 1, glm::value_ptr(material.color(Material::AMBIENT)));
    glUniform3fv(renderer.uniform(Renderer::KD), 1, glm::value_ptr(material.color(Material::DIFFUSE)));
    glUniform3fv(renderer.uniform(Renderer::KS), 1, glm::value_ptr(material.color(Material::SPECULAR)));
    glUniform1f(renderer.uniform(Renderer::SHININESS), material.shininess());

    struct { Material::Attribute attribute; GLuint unit; } textures[4] = {
        { Material::AMBIENT, renderer.texunit(Renderer::TEX_AMBIENT) },
        { Material::DIFFUSE, renderer.texunit(Renderer::TEX_DIFFUSE) },
        { Material::SPECULAR, renderer.texunit(Renderer::TEX_SPECULAR) },
        { Material::SHININESS, renderer.texunit(Renderer::TEX_SHININESS) }
    };

    for (auto const& t: textures) {
        int id = material.texture(t.attribute);

        if (id >= 0) {
            Texture const& texture = scene.getTexture(id);
            glBindTextureUnit(t.unit, texture.identifier());
        }
    }

    glDrawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT, 0);

    for (auto const& t: textures) {
        glBindTextureUnit(t.unit, 0);
    }

    glBindVertexArray(0);
}

void Mesh::drawGeometry() const {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::drawReflections(Renderer const& renderer, Scene const& scene) const {
    glBindVertexArray(m_vao);

    Material const& material = scene.getMaterial(m_material);
    glUniform3fv(renderer.uniform(Renderer::KD), 1, glm::value_ptr(material.color(Material::DIFFUSE)));

    GLuint unit = renderer.texunit(Renderer::TEX_DIFFUSE);
    int id = material.texture(Material::DIFFUSE);
    if (id >= 0) {
        Texture const& texture = scene.getTexture(id);
        glBindTextureUnit(unit, texture.identifier());
    }

    glDrawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT, 0);

    glBindTextureUnit(unit, 0);
    glBindVertexArray(0);
}
/***************************************************/


/***************************************************/
Object::Object(std::vector<unsigned int> meshes, glm::mat4 transformation)
:   m_meshes(std::move(meshes)), m_children(), m_position(), m_orientation(), m_scale(), m_transformation(std::move(transformation))
{
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(transformation, m_scale, m_orientation, m_position, skew, perspective);
}

Object::Object(Object&& other)
:   m_meshes(), m_children(), m_position(), m_orientation(), m_scale(), m_transformation()
{
    std::swap(m_meshes, other.m_meshes);
    std::swap(m_children, other.m_children);
    std::swap(m_position, other.m_position);
    std::swap(m_orientation, other.m_orientation);
    std::swap(m_scale, other.m_scale);
    std::swap(m_transformation, other.m_transformation);
}

void Object::addChild(Object child) {
    m_children.emplace_back(std::move(child));
}

void print(glm::mat4 const& m) {
    const float *pSource = (const float*)glm::value_ptr(m);
    std::cout << pSource[0] << " " << pSource[1] << " " << pSource[2] << " " << pSource[3] << '\n';
    std::cout << pSource[4] << " " << pSource[5] << " " << pSource[6] << " " << pSource[7] << '\n';
    std::cout << pSource[8] << " " << pSource[9] << " " << pSource[10] << " " << pSource[11] << '\n';
    std::cout << pSource[12] << " " << pSource[13] << " " << pSource[14] << " " << pSource[15] << '\n';
}

void Object::drawAll(Renderer const& renderer, Scene const& scene, glm::mat4 const& upper) const {
    glm::mat4 mv = upper * m_transformation;

    glUniformMatrix4fv(renderer.uniform(Renderer::MVP), 1, false, glm::value_ptr(scene.projection() * mv));
    glUniformMatrix4fv(renderer.uniform(Renderer::MV), 1, false, glm::value_ptr(mv));
    glUniformMatrix4fv(renderer.uniform(Renderer::NORMAL_MAT), 1, false, glm::value_ptr(glm::inverseTranspose(mv)));

    for (auto const& m: m_meshes) {
        scene.getMesh(m).drawAll(renderer, scene);
    }

    for (auto const& c: m_children) {
        c.drawAll(renderer, scene, mv);
    }
}

void Object::drawShadows(Renderer const& renderer, Scene const& scene, glm::mat4 const& upper) const {
    glm::mat4 mvp = upper * m_transformation;
    glUniformMatrix4fv(renderer.uniform(Renderer::LIGHT_MVP), 1, false, glm::value_ptr(mvp));
    
    for (auto const& m: m_meshes) {
        scene.getMesh(m).drawGeometry();
    }

    for (auto const& c: m_children) {
        c.drawShadows(renderer, scene, mvp);
    }
}

void Object::drawRSM(Renderer const& renderer, Scene const& scene, glm::mat4 const& cupper, glm::mat4 const& lupper) const {
    glm::mat4 cmv = cupper * m_transformation;
    glUniformMatrix4fv(renderer.uniform(Renderer::MV), 1, false, glm::value_ptr(cmv));
    glUniformMatrix4fv(renderer.uniform(Renderer::NORMAL_MAT), 1, false, glm::value_ptr(glm::inverseTranspose(cmv)));

    // static int i = 0;
    // if (i == 0) {
    //     print (cmv);
    // }
    // ++ i;

    glm::mat4 lmvp = lupper * m_transformation;
    glUniformMatrix4fv(renderer.uniform(Renderer::LIGHT_MVP), 1, false, glm::value_ptr(lmvp));
    
    for (auto const& m: m_meshes) {
        scene.getMesh(m).drawReflections(renderer, scene);
    }

    for (auto const& c: m_children) {
        c.drawRSM(renderer, scene, cmv, lmvp);
    }
}
/***************************************************/
