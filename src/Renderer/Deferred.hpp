#pragma once

#include <glad/glad.h>
#include "Renderer.hpp"
#include "Program.hpp"
#include "Light.hpp"
#include "ShadowMap.hpp"
#include "RSM.hpp"
#include "GUI/Listener.hpp"

class Scene;

class Deferred: public Renderer, public ResizeListener {
public:
    Deferred();
    virtual ~Deferred();

    void draw(Scene const& scene) const;

    virtual void onResize(int width, int height) override;

    void light(Light light) { m_light = std::move(light); }
    void computeRSM(Scene const& scene) { m_rsm.compute(scene, m_light); }

    virtual GLuint uniform(Identifier uniform) const override;
    virtual GLuint attribute(Identifier attribute) const override;
    virtual GLuint texunit(Identifier texunit) const override;

    virtual Light const& light() const override { return m_light; }
    

    Deferred(Deferred const&) = delete;
    Deferred& operator=(Deferred const&) = delete;

    void drawRSM() const;

private:
    void geometryPass(Scene const& scene) const;
    void shadingPass(Scene const& scene) const;

    enum GBuffer {
        GPOSITION, GNORMAL, GAMBIENT, GDIFFUSE, GGLOSSYSHININESS, GDEPTH, GLOCATION, GBuffer_Count
    };

    void blitBuffer() const;

    // Programs.
    Program m_geometry;
    Program m_shading;
    
    // Uniform locations.
    GLuint m_umvp, m_umv, m_unormalmat;
    GLuint m_uka, m_ukd, m_uks, m_ushininess;
    GLuint m_utexambient, m_utexdiffuse, m_utexspecular, m_utexshininess;
    GLuint m_ulightdir, m_ulightint;
    GLuint m_ulightmvp, m_ushadowmap, m_usmbias;
    GLuint m_ursmposition, m_ursmnormal, m_ursmflux, m_umaxsamples, m_uceil;
    GLuint m_uposition, m_unormal, m_uambient, m_udiffuse, m_uglossyshininess;

    // Attribute locations.
    GLuint m_aposition, m_anormal, m_atexcoords;

    // Renderer objects.
    GLuint m_samplers[2];
    GLuint m_fbo;
    GLuint m_gbuffers[GBuffer_Count];
    GLuint m_vbo;
    GLuint m_vao;

    // Light.
    Light m_light;
    ShadowMap m_shadowMap;
    RSM m_rsm;
};