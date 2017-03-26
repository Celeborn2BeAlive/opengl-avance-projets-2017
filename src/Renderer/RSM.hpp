#pragma once

#include <glad/glad.h>

class Scene;
class Light;

class RSM {
public:
    enum Texture {
        DEPTH, POSITION, NORMAL, FLUX, Texture_Count
    };

    RSM(unsigned int resolution);
    RSM(RSM&& other);
    ~RSM();

    void compute(Scene const& scene, Light const& light);
    void draw(Texture texture) const;

    GLuint texture(Texture texture) const { return m_textures[texture]; }


    RSM(RSM const&) = delete;
    RSM& operator=(RSM const&) = delete;

private:
    class Computer;

    unsigned int m_resolution;
    GLuint m_textures[Texture_Count];
};