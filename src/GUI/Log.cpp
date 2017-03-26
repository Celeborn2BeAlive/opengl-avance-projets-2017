#include "Log.hpp"
#include <iostream>

#include <glad/glad.h>

char const* log_string(GLenum value) {
    switch (value) {
        case GL_DEBUG_SOURCE_API:               return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     return "WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   return "THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION:       return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:             return "OTHER";

        case GL_DEBUG_TYPE_ERROR:               return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
        case GL_DEBUG_TYPE_OTHER:               return "OTHER";

        case GL_DEBUG_SEVERITY_HIGH:            return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:          return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:             return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION:    return "NOTIFICATION";

        default:                                return "UNDEFINED";
    }
}

void log_gl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, GLchar const* message, GLvoid* /*uparam*/) {
    std::cerr << "OpenGL: " << message << 
                    " [source=" << log_string(source) << 
                    " type=" << log_string(type) << 
                    " severity=" << log_string(severity) << 
                    " id=" << id << "]\n\n";
}

void Log::initializeLogGL() {
    glDebugMessageCallback((GLDEBUGPROC)log_gl, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

