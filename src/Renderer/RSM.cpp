#include "RSM.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Light.hpp"
#include "Renderer/Program.hpp"
#include "Scene/Scene.hpp"
#include "Config.hpp"

#include <iostream>
#include <cassert>
#include <glm/gtc/type_ptr.hpp>


class RSM::Computer: public Renderer {
public:
    Computer()
    :   m_program(SHADERS_PATH "rsm.vs.glsl", SHADERS_PATH "rsm.fs.glsl"),
        m_sampler(0), m_fbo(0), m_lightMVP(m_program.uniform("uLightViewProjMatrix")),
        m_mv(m_program.uniform("uModelViewMatrix")), m_normal(m_program.uniform("uNormalMatrix")),
        m_intensity(m_program.uniform("uIntensity")), m_kd(m_program.uniform("uKd")),
        m_tdiffuse(m_program.uniform("uKdSampler")), m_light(nullptr)
    {
        glCreateSamplers(1, &m_sampler);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        static GLenum const attachments[] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
        };

        glCreateFramebuffers(1, &m_fbo);
        glNamedFramebufferDrawBuffers(m_fbo, 3, attachments);

        assert(glGetError() == GL_NO_ERROR);
    }

    virtual ~Computer() {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteSamplers(1, &m_sampler);
    }

    virtual Light const& light() const override {
        return *m_light;
    }

    virtual GLuint uniform(Identifier uniform) const override {
        switch (uniform) {
            case LIGHT_MVP:     return m_lightMVP;
            case MV:            return m_mv;
            case NORMAL_MAT:    return m_normal;
            case KD:            return m_kd;
            case TEX_DIFFUSE:   return m_tdiffuse;

            default:            assert(false);
        }
    }

    virtual GLuint texunit(Identifier texunit) const override {
        if (texunit == TEX_DIFFUSE) {
            return 0;
        }

        assert(false);
    }

    void compute(RSM& rsm, Scene const& scene, Light const& light) {
        glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, rsm.m_textures[RSM::DEPTH], 0);
        glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, rsm.m_textures[RSM::POSITION], 0);
        glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT1, rsm.m_textures[RSM::NORMAL], 0);
        glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT2, rsm.m_textures[RSM::FLUX], 0);

        if (glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Frame buffer has not been set correctly: " << glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER) << std::endl;
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        m_program.use();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, rsm.m_resolution, rsm.m_resolution);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindSampler(0, m_sampler);
        glUniform3fv(m_intensity, 1, glm::value_ptr(light.intensity()));

        m_light = &light;
        scene.draw(*this, Scene::DRAW_RSM);
        m_light = nullptr;

        glBindSampler(0, 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    }


    Computer(Computer const&) = delete;
    Computer& operator=(Computer const&) = delete;

private:
    Program m_program;
    GLuint m_sampler;
    GLuint m_fbo;

    GLuint m_lightMVP;
    GLuint m_mv;
    GLuint m_normal;
    GLuint m_intensity;
    GLuint m_kd;
    GLuint m_tdiffuse;

    Light const* m_light;
};

class RSMRenderer {
public:
    RSMRenderer()
    :   m_ptexture(SHADERS_PATH "position.vs.glsl", SHADERS_PATH "texture.fs.glsl"),
        m_pdepth(SHADERS_PATH "position.vs.glsl", SHADERS_PATH "depth.fs.glsl"),
        m_utexture(m_ptexture.uniform("uDepth")), m_udepth(m_pdepth.uniform("uDepth")),
        m_vao(0), m_vbo(0), m_sampler(0)
    {
        // VBO.
        glm::vec2 vertices[] = { glm::vec2(-1, -1), glm::vec2(3, -1), glm::vec2(-1, 3) };
        glCreateBuffers(1, &m_vbo);
        glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);

        // VAO.
        glCreateVertexArrays(1, &m_vao);
        glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(glm::vec2));

        GLint position = 0;
        glVertexArrayAttribBinding(m_vao, position, 0);
        glEnableVertexArrayAttrib(m_vao, position);
        glVertexArrayAttribFormat(m_vao, position, 2, GL_FLOAT, GL_FALSE, 0);

        // Sampler.
        glCreateSamplers(1, &m_sampler);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        assert(glGetError() == GL_NO_ERROR);
    }

    ~RSMRenderer() {
        glDeleteSamplers(1, &m_sampler);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    void draw(RSM const& RSM, RSM::Texture texture) const {
        (texture == RSM::DEPTH ? m_pdepth : m_ptexture).use();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1i(texture == RSM::DEPTH ? m_udepth : m_utexture, 0);
        glBindSampler(0, m_sampler);
        glBindTextureUnit(0, RSM.texture(texture));

        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glBindTextureUnit(0, 0);
        glBindSampler(0, 0);
    }

private:
    Program m_ptexture, m_pdepth;
    GLuint m_utexture, m_udepth;

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_sampler;
};


RSM::RSM(unsigned int resolution)
:   m_resolution(resolution), m_textures{0}
{
    glCreateTextures(GL_TEXTURE_2D, Texture_Count, m_textures);

    glTextureStorage2D(m_textures[DEPTH], 1, GL_DEPTH_COMPONENT32F, m_resolution, m_resolution);
    glTextureStorage2D(m_textures[POSITION], 1, GL_RGB32F, m_resolution, m_resolution);
    glTextureStorage2D(m_textures[NORMAL], 1, GL_RGB32F, m_resolution, m_resolution);
    glTextureStorage2D(m_textures[FLUX], 1, GL_RGB32F, m_resolution, m_resolution);

    assert(glGetError() == GL_NO_ERROR);
}

RSM::RSM(RSM&& other)
:   m_resolution(other.m_resolution), m_textures{0}
{
    for (int i = 0; i < Texture_Count; ++ i) {
        std::swap(m_textures[i], other.m_textures[i]);
    }
}

RSM::~RSM() {
    glDeleteTextures(Texture_Count, m_textures);
}

void RSM::compute(Scene const& scene, Light const& light) {
    static Computer computer;
    computer.compute(*this, scene, light);
}

void RSM::draw(Texture texture) const {
    static RSMRenderer renderer;
    renderer.draw(*this, texture);
}