#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

#include <limits>
#include <random>

class Application
{
public:
	Application(int argc, char** argv);

	int run();
private:
	//Calculer les coord cart a partir de coord spheriques
	static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
	{
		const auto cosPhi = glm::cos(phiRadians);
		const auto sinPhi = glm::sin(phiRadians);
		const auto sinTheta = glm::sin(thetaRadians);
		return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
	}

	//Fenetre
	const size_t m_nWindowWidth = 1280;
	const size_t m_nWindowHeight = 720;
	glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

	//Executable et fichiers auxiliaires
	const glmlv::fs::path m_AppPath;
	const std::string m_AppName;
	const std::string m_ImGuiIniFilename;
	const glmlv::fs::path m_ShadersRootPath;
	const glmlv::fs::path m_AssetsRootPath;

	// Scene data in GPU:
	GLuint m_SceneVBO = 0;
	GLuint m_SceneIBO = 0;
	GLuint m_SceneVAO = 0;
	GLuint m_FBO = 0; //FrameBuffer
	GLuint m_QuadVBO = 0;
	GLuint m_QuadIBO = 0;
	GLuint m_QuadVAO = 0;

	// Required data about the scene in CPU in order to send draw calls
	struct ShapeInfo
	{
		uint32_t indexCount; // Number of indices
		uint32_t indexOffset; // Offset in GPU index buffer
		int materialID = -1;
	};

	std::vector<ShapeInfo> m_shapes; // For each shape of the scene, its number of indices
	float m_SceneSize = 0.f; // Used for camera speed and projection matrix parameters

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

	GLuint m_WhiteTexture; // A white 1x1 texture
	PhongMaterial m_DefaultMaterial;
	std::vector<PhongMaterial> m_SceneMaterials;

	//GBuffer, meme layout que dans le shader, on a 6 textures (dont depth, sinon 5, de 0 a 4)
	enum GBufferTextureType {
		GPosition = 0,
		GNormal,
		GAmbient,
		GDiffuse,
		GGlossyShininess,
		GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
		GBufferTextureCount
	};
	const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };
	//Une texture par indice de l'enum
	GLuint m_GBufferTextures[GBufferTextureCount];
	
	GLuint m_textureSampler = 0; // Only one sampler object since we will use the same sampling parameters for all textures

	//Shader programs
	glmlv::GLProgram m_geomprogram;
	glmlv::GLProgram m_shadprogram;

	//ViewController
	glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };

	//Shaders Uniform variables
	GLint m_uModelViewProjMatrixLocation;
	GLint m_uModelViewMatrixLocation;
	GLint m_uNormalMatrixLocation;

	GLint m_uKaLocation;
	GLint m_uKdLocation;
	GLint m_uKsLocation;
	GLint m_uShininessLocation;
	GLint m_uKaSamplerLocation;
	GLint m_uKdSamplerLocation;
	GLint m_uKsSamplerLocation;
	GLint m_uShininessSamplerLocation;

	GLint m_uDirectionalLightDirLocation;
	GLint m_uDirectionalLightIntensityLocation;

	GLint m_uPointLightPositionLocation;
	GLint m_uPointLightIntensityLocation;

	GLint m_uGPositionLocation;
	GLint m_uGNormalLocation;
	GLint m_uGAmbientLocation;
	GLint m_uGDiffuseLocation;
	GLint m_uGlossyShininessLocation;

	//Lights base parameters
	float m_DirLightPhiAngleDegrees = 90.f;
	float m_DirLightThetaAngleDegrees = 45.f;
	glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
	glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
	float m_DirLightIntensity = 1.f;

	glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
	glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
	float m_PointLightIntensity = 5.f;

	//SSAO variables
	std::uniform_real_distribution<GLfloat> randomFloats = std::uniform_real_distribution<GLfloat>(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;

	GLuint noiseTexture;
	GLuint ssaoFBO;
	GLuint ssaoColorBuffer;
	GLuint ssaoBlurFBO;
	GLuint ssaoColorBufferBlur;
	float ssaopower = 1.0f;

	GLuint m_uSSAOTexLocation; //in the shading pass frag shader

	glmlv::GLProgram m_ssaoprogram;
	GLuint m_uSSAOgPositionLocation;
	GLuint m_uSSAOgNormalLocation;
	GLuint m_uSSAOtexNoiseLocation;
	GLuint m_uSSAOsamplesLocation;
	GLuint m_uSSAOprojectionLocation;
	GLuint m_uSSAOpowerLocation;

	glmlv::GLProgram m_ssaoblurprogram;
	GLuint m_uSSAOBlurinputLocation;
};