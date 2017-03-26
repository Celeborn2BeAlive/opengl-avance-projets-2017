#include "Program.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

class Program::Shader {
public:
    Shader(std::string const& path, GLenum type)
    :   m_identifier(0)
    {
        m_identifier = glCreateShader(type);

        std::string src = source(path);
        GLchar const* cstr = src.c_str();  
        glShaderSource(m_identifier, 1, &cstr, 0);

        glCompileShader(m_identifier);
        check();
    }

    Shader(Shader&& other)
    :   m_identifier(0)
    {
        std::swap(m_identifier, other.m_identifier);
    }

    ~Shader() {
        if (m_identifier) glDeleteShader(m_identifier);
    }

    Shader& operator=(Shader&& other) {
        std::swap(m_identifier, other.m_identifier);
        return *this;
    }

    GLuint identifier() const { return m_identifier; }


    Shader(Shader const&) = delete;
    Shader& operator=(Shader const&) = delete;

private:
    std::string source(std::string const& path) {
        std::ifstream input(path);
        if (!input) {
            glDeleteShader(m_identifier);
            m_identifier = 0;

            throw std::runtime_error("Cannot open shader " + path + ".");
        }

        std::stringstream buffer;
        buffer << input.rdbuf();

        return std::string(buffer.str());
    }

    void check() {
        GLint status;
        glGetShaderiv(m_identifier, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) {
            GLint length;
            glGetShaderiv(m_identifier, GL_INFO_LOG_LENGTH, &length);

            std::vector<char> buffer(length);
            glGetShaderInfoLog(m_identifier, length, 0, buffer.data());

            std::cerr << std::string(buffer.data()) << std::endl;

            glDeleteShader(m_identifier);
            m_identifier = 0;

            throw std::runtime_error(buffer.data());
        }
    }

    GLuint m_identifier;
};

Program::Program(std::string const& vertex, std::string const& fragment)
:   m_identifier(0)
{
    std::vector<Shader> shaders;
    shaders.emplace_back(vertex, GL_VERTEX_SHADER);
    shaders.emplace_back(fragment, GL_FRAGMENT_SHADER);

    create(shaders);
}

Program::Program(std::string const& vertex, std::string const& geometry, std::string const& fragment)
:   m_identifier(0)
{
    std::vector<Shader> shaders;
    shaders.emplace_back(vertex, GL_VERTEX_SHADER);
    shaders.emplace_back(geometry, GL_GEOMETRY_SHADER);
    shaders.emplace_back(fragment, GL_FRAGMENT_SHADER);

    create(shaders);
}

Program::Program(Program&& other)
:   m_identifier(0)
{
    std::swap(m_identifier, other.m_identifier);
}

Program::~Program() {
    if (m_identifier) glDeleteProgram(m_identifier);
}

Program& Program::operator=(Program&& other) {
    std::swap(m_identifier, other.m_identifier);
    return *this;
}

void Program::use() const {
    glUseProgram(m_identifier);
}

GLint Program::uniform(GLchar const* name) const {
    return glGetUniformLocation(m_identifier, name);
}

GLint Program::attribute(GLchar const* name) const {
    return glGetAttribLocation(m_identifier, name);
}

void Program::create(std::vector<Shader> const& shaders) {
    m_identifier = glCreateProgram();

    for (auto const& s: shaders) {
        glAttachShader(m_identifier, s.identifier());
    }

    glLinkProgram(m_identifier);
    check();
}

void Program::check() {
    GLint status;
    glGetProgramiv(m_identifier, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        GLint length;
        glGetProgramiv(m_identifier, GL_INFO_LOG_LENGTH, &length);

        std::vector<char> buffer(length);
        glGetProgramInfoLog(m_identifier, length, 0, buffer.data());

        glDeleteProgram(m_identifier);
        m_identifier = 0;

        throw std::runtime_error(buffer.data());
    }
}