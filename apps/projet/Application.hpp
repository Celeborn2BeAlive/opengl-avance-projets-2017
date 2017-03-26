#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
	struct ShapeInfo
	{
		uint32_t indexCount; // Number of indices
		uint32_t indexOffset; // Offset in GPU index buffer
		int materialID = -1;
	};


	struct PhongMaterial
	{
		glm::vec3 Ka = glm::vec3(0); // Ambient multiplier
		glm::vec3 Kd = glm::vec3(0); // Diffuse multiplier
		glm::vec3 Ks = glm::vec3(0); // Glossy multiplier
		float shininess = 1.f; // Glossy exponent

							   // OpenGL texture ids:
		GLuint KaTextureId = 0;
		GLuint KdTextureId = 0;
		GLuint KsTextureId = 0;
		GLuint shininessTextureId = 0;
	};

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

	void initScene();

	void initShadersData();

	const glmlv::fs::path m_AppPath;
	const std::string m_AppName;
	const std::string m_ImGuiIniFilename;
	const glmlv::fs::path m_ShadersRootPath;
	const glmlv::fs::path m_AssetsRootPath;


	GLuint m_WhiteTexture;

	GLuint m_SceneVBO = 0;
	GLuint m_SceneIBO = 0;
	GLuint m_SceneVAO = 0;

	GLuint DepthBuffer = 0;
	const GLenum DepthFormat = GL_DEPTH_COMPONENT;
	GLuint DepthTextures = 0;
	GLuint m_BufferFBO;
	std::vector<ShapeInfo> m_shapes; // For each shape of the scene, its number of indices
	float m_SceneSize = 0.f; 

	glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
	GLint m_uModelViewProjMatrixLocation;
	GLint m_uModelViewMatrixLocation;
	GLint m_uNormalMatrixLocation;

	PhongMaterial m_DefaultMaterial;
	std::vector<PhongMaterial> m_SceneMaterials;

	GLuint m_textureSampler = 0; // Only one sampler object since we will use the same sampling parameters for all textures


};