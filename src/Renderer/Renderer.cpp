#include "Renderer.hpp"

#include <cassert>

GLuint Renderer::uniform(Identifier) const {
    assert(false);
}

GLuint Renderer::attribute(Identifier) const {
    assert(false);
}

GLuint Renderer::texunit(Identifier) const {
    assert(false);
}

Light const& Renderer::light() const {
    assert(false);
}