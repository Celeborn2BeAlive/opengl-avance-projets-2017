#include "Deferred.hpp"
#include "Scene/Scene.hpp"
#include "GUI/GUI.hpp"
#include "Config.hpp"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Deferred::Deferred()
:   m_geometry(SHADERS_PATH "geometry.vs.glsl", SHADERS_PATH "geometry.fs.glsl"),
    // m_shading(SHADERS_PATH "shading.vs.glsl", SHADERS_PATH "shading-sm.fs.glsl"),
    m_shading(SHADERS_PATH "shading.vs.glsl", SHADERS_PATH "shading-rsm.fs.glsl"),
    m_umvp(0), m_umv(0), m_unormalmat(0),
    m_uka(0), m_ukd(0), m_uks(0), m_ushininess(0),
    m_utexambient(0), m_utexdiffuse(0), m_utexspecular(0), m_utexshininess(0),
    m_ulightdir(0), m_ulightint(0), 
    m_ulightmvp(0), m_ushadowmap(0), m_usmbias(0),
    m_ursmposition(0), m_ursmnormal(0), m_ursmflux(0), m_umaxsamples(0), m_uceil(0),
    m_uposition(0), m_unormal(0), m_uambient(0), m_udiffuse(0), m_uglossyshininess(0), 
    m_aposition(0), m_anormal(0), m_atexcoords(0),
    m_samplers{0}, m_fbo(0), m_gbuffers{0},
    m_vbo(0), m_vao(0),
    m_light(glm::vec3(), glm::vec3()), m_shadowMap(2048), m_rsm(2048)
{
    // Uniform locations.
    m_umvp = m_geometry.uniform("uModelViewProjMatrix");
    m_umv = m_geometry.uniform("uModelViewMatrix");
    m_unormalmat = m_geometry.uniform("uNormalMatrix");

    m_uka = m_geometry.uniform("uKa");
    m_ukd = m_geometry.uniform("uKd");
    m_uks = m_geometry.uniform("uKs");
    m_ushininess = m_geometry.uniform("uShininess");

    m_utexambient = m_geometry.uniform("uKaSampler");
    m_utexdiffuse = m_geometry.uniform("uKdSampler");
    m_utexspecular = m_geometry.uniform("uKsSampler");
    m_utexshininess = m_geometry.uniform("uShininessSampler");

    m_ulightdir = m_shading.uniform("uLDirection");
    m_ulightint = m_shading.uniform("uLIntensity");

    m_ulightmvp = m_shading.uniform("uShadowMapMatrix");
    m_ushadowmap = m_shading.uniform("uShadowMap");
    m_usmbias = m_shading.uniform("uShadowMapBias");

    m_ursmposition = m_shading.uniform("uRSMPosition");
    m_ursmnormal = m_shading.uniform("uRSMNormal");
    m_ursmflux = m_shading.uniform("uRSMFlux");
    m_umaxsamples = m_shading.uniform("uMaxSamples");
    m_uceil = m_shading.uniform("uCeil");

    m_uposition = m_shading.uniform("uGPosition");
    m_unormal = m_shading.uniform("uGNormal");
    m_uambient = m_shading.uniform("uGAmbient");
    m_udiffuse = m_shading.uniform("uGDiffuse");
    m_uglossyshininess = m_shading.uniform("uGGlossyShininess");

    // Attribute locations.
    m_aposition = m_geometry.attribute("aPosition");
    m_anormal = m_geometry.attribute("aNormal");
    m_atexcoords = m_geometry.attribute("aTexCoords");

    // Linear sampler.
    glCreateSamplers(2, m_samplers);
    glSamplerParameteri(m_samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(m_samplers[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(m_samplers[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // glSamplerParameteri(m_samplers[1], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    // glSamplerParameteri(m_samplers[1], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // Create the frame buffer.
    static GLenum const attachments[] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4
    };

    glCreateFramebuffers(1, &m_fbo);
    glNamedFramebufferDrawBuffers(m_fbo, 5, attachments);

    int w, h;
    GUI::size(w, h);
    onResize(w, h);

    // Create the spanning triangle.
    glm::vec2 vertices[] = { glm::vec2(-1, -1), glm::vec2(3, -1), glm::vec2(-1, 3) };
    glCreateBuffers(1, &m_vbo);
    glNamedBufferStorage(m_vbo, sizeof(vertices), vertices, 0);

    glCreateVertexArrays(1, &m_vao);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(glm::vec2));

    GLint lposition = m_shading.attribute("aPosition");
    glVertexArrayAttribBinding(m_vao, lposition, 0);
    glEnableVertexArrayAttrib(m_vao, lposition);
    glVertexArrayAttribFormat(m_vao, lposition, 2, GL_FLOAT, GL_FALSE, 0);

    assert(glGetError() == GL_NO_ERROR);
}

Deferred::~Deferred() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);

    glDeleteTextures(GBuffer_Count, m_gbuffers);
    glDeleteFramebuffers(1, &m_fbo);

    glDeleteSamplers(2, m_samplers);
}

GLuint Deferred::uniform(Identifier uniform) const {
    switch (uniform) {
        case MVP:               return m_umvp;
        case MV:                return m_umv;
        case NORMAL_MAT:        return m_unormalmat;

        case KA:                return m_uka;
        case KD:                return m_ukd;
        case KS:                return m_uks;
        case SHININESS:         return m_ushininess;

        case TEX_AMBIENT:       return m_utexambient;
        case TEX_DIFFUSE:       return m_utexdiffuse;
        case TEX_SPECULAR:      return m_utexspecular;
        case TEX_SHININESS:     return m_utexshininess;

        default:                assert(false);
    }
}

GLuint Deferred::attribute(Identifier attribute) const {
    switch (attribute) {
        case POSITION:          return m_aposition;
        case NORMAL:            return m_anormal;
        case TEXCOORDS:         return m_atexcoords;

        default:                assert(false);
    }
}

GLuint Deferred::texunit(Identifier texunit) const {
    switch (texunit) {
        case TEX_AMBIENT:       return 0;
        case TEX_DIFFUSE:       return 1;
        case TEX_SPECULAR:      return 2;
        case TEX_SHININESS:     return 3;

        default:                assert(false);
    }
}

void Deferred::draw(Scene const& scene) const {
    geometryPass(scene);
    shadingPass(scene);
}

void Deferred::geometryPass(Scene const& scene) const {
    static struct { GLuint unit; GLuint uniform; } const textures[4] = {
        { texunit(TEX_AMBIENT), uniform(TEX_AMBIENT) },
        { texunit(TEX_DIFFUSE), uniform(TEX_DIFFUSE) },
        { texunit(TEX_SPECULAR), uniform(TEX_SPECULAR) },
        { texunit(TEX_SHININESS), uniform(TEX_SHININESS) }
    };

    m_geometry.use();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto const& t: textures) {
        glBindSampler(t.unit, m_samplers[0]);
        glUniform1i(t.uniform, t.unit);
    }

    scene.draw(*this);

    for (auto const& t: textures) {
        glBindSampler(t.unit, 0);
    }
}

void Deferred::shadingPass(Scene const& scene) const {
    m_shading.use();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static struct { GBuffer name; GLuint uniform; GLuint unit; } const buffers[5] = {
        { GPOSITION, m_uposition, 0 },
        { GNORMAL, m_unormal, 1 },
        { GAMBIENT, m_uambient, 2 },
        { GDIFFUSE, m_udiffuse, 3 },
        { GGLOSSYSHININESS, m_uglossyshininess, 4 },
    };

    for (auto const& b: buffers) {
        glUniform1i(b.uniform, b.unit);
        glBindTextureUnit(b.unit, m_gbuffers[b.name]);
    }

    glm::vec3 ldirection = scene.view() * glm::vec4(m_light.direction(), 0);
    glUniform3fv(m_ulightdir, 1, glm::value_ptr(ldirection));
    glUniform3fv(m_ulightint, 1, glm::value_ptr(m_light.intensity()));

    glUniformMatrix4fv(m_ulightmvp, 1, false, glm::value_ptr(m_light.viewproj(scene) * glm::inverse(scene.view())));
    glUniform1f(m_usmbias, 0.001f);
    glUniform1i(m_ushadowmap, 5);
    glBindSampler(5, m_samplers[1]);
    glBindTextureUnit(5, m_rsm.texture(RSM::DEPTH));

    glUniform1i(m_ursmposition, 6);
    glBindTextureUnit(6, m_rsm.texture(RSM::POSITION));

    glUniform1i(m_ursmnormal, 7);
    glBindTextureUnit(7, m_rsm.texture(RSM::NORMAL));

    glUniform1i(m_ursmflux, 8);
    glBindTextureUnit(8, m_rsm.texture(RSM::FLUX));

    glUniform1ui(m_umaxsamples, 1024);
    glUniform1ui(m_uceil, 1024);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glBindTextureUnit(5, 0);
    glBindSampler(5, 0);

    for (auto const& b: buffers) {
        glBindTextureUnit(b.unit, 0);
    }

    // blitBuffer();
}

void Deferred::drawRSM() const {
    m_rsm.draw(RSM::NORMAL);
}

void Deferred::onResize(int width, int height) {
    static struct { unsigned int name; GLenum format; GLenum attachment; } const binds[] = {
        { GPOSITION, GL_RGB32F, GL_COLOR_ATTACHMENT0 },
        { GNORMAL, GL_RGB32F, GL_COLOR_ATTACHMENT1 },
        { GAMBIENT, GL_RGB32F, GL_COLOR_ATTACHMENT2 },
        { GDIFFUSE, GL_RGB32F, GL_COLOR_ATTACHMENT3 },
        { GGLOSSYSHININESS, GL_RGBA32F, GL_COLOR_ATTACHMENT4 },
        { GDEPTH, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT }
    };

    glDeleteTextures(GBuffer_Count, m_gbuffers);
    glCreateTextures(GL_TEXTURE_2D, GBuffer_Count, m_gbuffers);

    for (auto const& b: binds) {
        glTextureStorage2D(m_gbuffers[b.name], 1, b.format, width, height);
        glNamedFramebufferTexture(m_fbo, b.attachment, m_gbuffers[b.name], 0);
    }

    if (glCheckNamedFramebufferStatus(m_fbo, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Frame buffer has not been set correctly." << std::endl;
    }
}

void Deferred::blitBuffer() const {
    int width, height;
    GUI::size(width, height);

    glNamedFramebufferReadBuffer(m_fbo, GL_COLOR_ATTACHMENT0);
    glBlitNamedFramebuffer(m_fbo, 0, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

