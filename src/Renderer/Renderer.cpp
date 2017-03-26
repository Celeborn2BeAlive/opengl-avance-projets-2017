#include "Renderer.hpp"
#include "Light.hpp"

#include <cassert>

GLuint Renderer::uniform(Identifier) const {
    assert(false);
    return -1;
}

GLuint Renderer::attribute(Identifier) const {
    assert(false);
    return -1;
}

GLuint Renderer::texunit(Identifier) const {
    assert(false);
    return -1;
}

Light const& Renderer::light() const {
    assert(false);
    static const Light l{ glm::vec3(), glm::vec3() };
    return l;
}