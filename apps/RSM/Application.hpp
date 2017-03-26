#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;

    //Buffers de la scène
    glmlv::SimpleGeometry m_scene;

    GLuint m_sceneVBO = 0;
    GLuint m_sceneIBO = 0;
    GLuint m_sceneVAO = 0;

    GLuint m_FBO;

    //shader et uniformes liés au rendu
    glmlv::GLProgram m_program;
    

    glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
    //vs
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uLightMVPMatrixLocation;
    GLint m_uLightPosLocation;
    //fs
    GLint m_uRandMapLocation;
    GLint m_uDepthMapLocation;
    GLint m_uPositionMapLocation;
    GLint m_uNormalMapLocation;
    GLint m_uDiffuseMapLocation;
    



    //shader et uniformes liés à la RSM
    glmlv::GLProgram m_RSMprogram;
    //gs
    GLint m_uProjMatLocation;
    GLint m_umvMatLocation;




    glm::mat4 m_cubeViewMatrix[6];
    glm::vec3 m_PointLightPosition = glm::vec3(0, 0, 0);

    //Textures
    const GLenum m_GBufferTextureFormat[4] = { GL_DEPTH_COMPONENT32F,GL_RGB32F, GL_RGB32F, GL_RGB32F  };
    GLuint m_GBufferTexture[4];
    GLuint randTex;
};