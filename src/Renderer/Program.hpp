#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>

class Program {
public:
    // Constructors.
    Program(std::string const& vertex, std::string const& fragment);
    Program(std::string const& vertex, std::string const& geometry, std::string const& fragment);

    // Move constructor.
    Program(Program&& other);

    // Destructor.
    ~Program();

    Program& operator=(Program&& other);

    // Use the program.
    void use() const;

    // Return a uniform or attribute location.
    GLint uniform(GLchar const* name) const;
    GLint attribute(GLchar const* name) const;

    
    Program(Program const&) = delete;
    Program& operator=(Program const&) = delete;

private:
    class Shader;

    void create(std::vector<Shader> const& shaders);
    void check();

    GLuint m_identifier;
};