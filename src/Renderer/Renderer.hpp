#pragma once

#include <glad/glad.h>

class Light;

class Renderer {
public:
    enum Identifier {
        POSITION, NORMAL, TEXCOORDS,
        MVP, MV, NORMAL_MAT, 
        KA, KD, KS, SHININESS, TEX_AMBIENT, TEX_DIFFUSE, TEX_SPECULAR, TEX_SHININESS, 
        LIGHT_MVP
    };
    
    virtual ~Renderer() = default;

    virtual GLuint uniform(Identifier uniform) const;
    virtual GLuint attribute(Identifier attribute) const;
    virtual GLuint texunit(Identifier texunit) const;

    virtual Light const& light() const;
};