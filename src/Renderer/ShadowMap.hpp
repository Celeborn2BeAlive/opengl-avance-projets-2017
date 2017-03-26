#pragma once

#include <glad/glad.h>

class Scene;
class Light;

class ShadowMap {
public:
    ShadowMap(unsigned int resolution);
    ShadowMap(ShadowMap&& other);
    ~ShadowMap();

    void compute(Scene const& scene, Light const& light);
    void draw() const;

    GLuint texture() const { return m_texture; }


    ShadowMap(ShadowMap const&) = delete;
    ShadowMap& operator=(ShadowMap const&) = delete;

private:
    class Computer;

    unsigned int m_resolution;
    GLuint m_texture;
};