#include "ShadowMap.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Light.hpp"
#include "Renderer/Program.hpp"
#include "Scene/Scene.hpp"
#include "Config.hpp"

#include <iostream>
#include <cassert>


class ShadowMap::Computer: public Renderer {
public:
    Computer()
    :   m_program(SHADERS_PATH "directional-sm.vs.glsl", SHADERS_PATH "directional-sm.fs.glsl"),
        m_fbo(0), m_lightMVP(m_program.uniform("uLightViewProjMatrix")), m_light(nullptr)
    {
        glCreateFramebuffers(1, &m_fbo);

        assert(glGetError() == GL_NO_ERROR);
    }

    virtual ~Computer() {
        glDeleteFramebuffers(1, &m_fbo);
    }

    virtual GLuint uniform(Identifier uniform) const override {
        assert(uniform == Renderer::LIGHT_MVP);
        return m_lightMVP;
    }

    virtual Light const& light() const override {
        return *m_light;
    }

    void compute(ShadowMap& shadowMap, Scene const& scene, Light const& light) {
        glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, shadowMap.m_texture, 0);
        if (glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Frame buffer has not been set correctly." << std::endl;
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_program.use();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, shadowMap.m_resolution, shadowMap.m_resolution);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_light = &light;
        scene.draw(*this, Scene::DRAW_SHADOWS);
        m_light = nullptr;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }


    Computer(Computer const&) = delete;
    Computer& operator=(Computer const&) = delete;

private:
    Program m_program;
    GLuint m_fbo;

    GLuint m_lightMVP;

    Light const* m_light;
};

class ShadowMapRenderer {
public:
    ShadowMapRenderer()
    :   m_program(SHADERS_PATH "position.vs.glsl", SHADERS_PATH "depth.fs.glsl"),
        m_depth(m_program.uniform("uDepth")),
        m_vao(0), m_vbo(0), m_sampler(0)
    {
        // VBO.
        glm::vec2 vertices[] = { glm::vec2(-1, -1), glm::vec2(3, -1), glm::vec2(-1, 3) };
        glCreateBuffers(1, &m_vbo);
        glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);

        // VAO.
        glCreateVertexArrays(1, &m_vao);
        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(glm::vec2));

        GLint position = m_program.attribute("aPosition");
        glVertexArrayAttribBinding(m_vao, position, 0);
        glEnableVertexArrayAttrib(m_vao, position);
        glVertexArrayAttribFormat(m_vao, position, 2, GL_FLOAT, GL_FALSE, 0);

        // Sampler.
        glCreateSamplers(1, &m_sampler);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        assert(glGetError() == GL_NO_ERROR);
    }

    ~ShadowMapRenderer() {
        glDeleteSamplers(1, &m_sampler);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    void draw(ShadowMap const& shadowMap) const {
        m_program.use();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1i(m_depth, 0);
        glBindSampler(0, m_sampler);
        glBindTextureUnit(0, shadowMap.texture());

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glBindTextureUnit(0, 0);
        glBindSampler(0, 0);
    }

private:
    Program m_program;
    GLuint m_depth;

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_sampler;
};


ShadowMap::ShadowMap(unsigned int resolution)
:   m_resolution(resolution), m_texture(0)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
    glTextureStorage2D(m_texture, 1, GL_DEPTH_COMPONENT32F, m_resolution, m_resolution);
}

ShadowMap::ShadowMap(ShadowMap&& other)
:   m_resolution(other.m_resolution), m_texture(0)
{
    std::swap(m_texture, other.m_texture);
}

ShadowMap::~ShadowMap() {
    glDeleteTextures(1, &m_texture);
}

void ShadowMap::compute(Scene const& scene, Light const& light) {
    static Computer computer;
    computer.compute(*this, scene, light);
}

void ShadowMap::draw() const {
    static ShadowMapRenderer renderer;
    renderer.draw(*this);
}