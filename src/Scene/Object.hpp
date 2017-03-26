#pragma once

#define NOMINMAX

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

class Renderer;
class Scene;


// Texture class.
class Texture {
public:
    // Constructor.
    Texture(unsigned int width, unsigned int height, unsigned char* data);

    // Move constructor.
    Texture(Texture&& other);

    // Destructor.
    ~Texture();

    // Return the identifier of the texture.
    GLuint identifier() const { return m_id; }


    Texture(Texture const&) = delete;
    Texture& operator=(Texture const&) = delete;

private:
    GLuint m_id;
};


// Material class.
class Material {
public:
    enum Attribute {
        AMBIENT, DIFFUSE, SPECULAR, SHININESS
    };

    // Constructor.
    Material(glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float shininess, int tka, int tkd, int tks, int tshininess);

    // Return the queried attribute.
    glm::vec3 color(Attribute attribute) const { return m_kcolors[attribute]; }
    float shininess() const { return m_shininess; }

    // Return the queried texture.
    int texture(Attribute attribute) const { return m_textures[attribute]; }

private:
    glm::vec3 m_kcolors[3];
    float m_shininess;

    int m_textures[4];
};


// Mesh class.
class Mesh {
public:
    struct vertex {
        glm::vec3 position = glm::vec3(0);
        glm::vec3 normal = glm::vec3(0);
        glm::vec2 texcoords = glm::vec2(0);

        vertex() = default;

        vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t) : position(p), normal(n), texcoords(t) {}
    };

    // Constructor.
    Mesh(std::vector<vertex> const& vertices, std::vector<unsigned int> const& indices, unsigned int material);

    // Move constructor.
    Mesh(Mesh&& other);

    // Destructor.
    ~Mesh();

    // Configure attribute bindings.
    void configure(Renderer const& renderer);

    // Draw the mesh.
    void drawAll(Renderer const& renderer, Scene const& scene) const;
    void drawGeometry() const;
    void drawReflections(Renderer const& renderer, Scene const& scene) const;

    // Return the vertices extremities of the mesh.
    glm::vec3 const& min() const { return m_min; }
    glm::vec3 const& max() const { return m_max; }


    Mesh(Mesh const&) = delete;
    Mesh& operator=(Mesh const&) = delete;

private:
    unsigned int m_material;
    unsigned int m_size;

    glm::vec3 m_min;
    glm::vec3 m_max;

    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_vao;
};


// Object class.
class Object {
public:
    // Constructor.
    Object(std::vector<unsigned int> meshes, glm::mat4 transformation);

    // Move constructor.
    Object(Object&& other);

    // Add a child to this node.
    void addChild(Object child);

    // Draw the object.
    void drawAll(Renderer const& renderer, Scene const& scene, glm::mat4 const& view) const;
    void drawShadows(Renderer const& renderer, Scene const& scene, glm::mat4 const& lvp) const;
    void drawRSM(Renderer const& renderer, Scene const& scene, glm::mat4 const& view, glm::mat4 const& lvp) const;

    
    Object(Object const&) = delete;
    Object& operator=(Object const&) = delete;

private:
    std::vector<unsigned int> m_meshes;
    std::vector<Object> m_children;

    glm::vec3 m_position;
    glm::quat m_orientation;
    glm::vec3 m_scale;

    glm::mat4 m_transformation;
};
