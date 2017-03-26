#pragma once

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Renderer/Renderer.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Object.hpp"

class Cube {
public:
    Cube()
    :   m_vbo(0), m_ibo(0), m_vao(0), m_size(36)
    {
        /* VBO. */
        glCreateBuffers(1, &m_vbo);

        Mesh::vertex vertices[] = {
            { glm::vec3(-1.f, -1.f, -1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(-1.f, -1.f, +1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(-1.f, +1.f, -1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(-1.f, +1.f, +1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(+1.f, -1.f, -1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(+1.f, -1.f, +1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(+1.f, +1.f, -1.f), glm::vec3(1.f), glm::vec2(0.f) },
            { glm::vec3(+1.f, +1.f, +1.f), glm::vec3(1.f), glm::vec2(0.f) }
        };

        glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);


        /* IBO. */
        glCreateBuffers(1, &m_ibo);

        unsigned int indices[] = {
            0, 1, 2, 1, 2, 3,
            4, 5, 6, 5, 6, 7,

            0, 1, 5, 0, 5, 4,
            2, 3, 7, 2, 7, 6,

            0, 2, 4, 2, 4, 6,
            1, 3, 7, 1, 7, 5
        };

        glNamedBufferStorage(m_ibo, sizeof(indices), indices, 0);

        /* VAO. */
        glCreateVertexArrays(1, &m_vao);
        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Mesh::vertex));
        glVertexArrayElementBuffer(m_vao, m_ibo);

        GLuint position = 0;
        glVertexArrayAttribBinding(m_vao, position, 0);
        glEnableVertexArrayAttrib(m_vao, position);
        glVertexArrayAttribFormat(m_vao, position, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::vertex, position));

        GLuint normal = 1;
        glVertexArrayAttribBinding(m_vao, normal, 0);
        glEnableVertexArrayAttrib(m_vao, normal);
        glVertexArrayAttribFormat(m_vao, normal, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::vertex, normal));

        GLuint texcoords = 2;
        glVertexArrayAttribBinding(m_vao, texcoords, 0);
        glEnableVertexArrayAttrib(m_vao, texcoords);
        glVertexArrayAttribFormat(m_vao, texcoords, 2, GL_FLOAT, GL_FALSE, offsetof(Mesh::vertex, texcoords));
    }

    ~Cube() {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_ibo);
        glDeleteBuffers(1, &m_vbo);
    }

    void draw(Renderer const& renderer, Scene const& scene) const {
        glm::mat4 mv = scene.view() * glm::translate(glm::vec3(0.f, 20.f, -500.f)) * glm::scale(glm::vec3(200.f));

        glUniformMatrix4fv(renderer.uniform(Renderer::MVP), 1, false, glm::value_ptr(scene.projection() * mv));
        glUniformMatrix4fv(renderer.uniform(Renderer::MV), 1, false, glm::value_ptr(mv));
        glUniformMatrix4fv(renderer.uniform(Renderer::NORMAL_MAT), 1, false, glm::value_ptr(glm::inverseTranspose(mv)));

        // glUniformMatrix4fv(renderer.uniform(Renderer::LIGHT_MVP), 1, false, glm::value_ptr(scene.projection() * mv));

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_vao;

    unsigned int m_size;
};