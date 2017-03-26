#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/load_obj.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        // Put here rendering code
        

        // GUI code:
        ImGui_ImplGlfwGL3_NewFrame();

        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
            if (ImGui::ColorEdit3("clearColor", clearColor)) {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
            }
            ImGui::End();
        }

        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        ImGui::Render();

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus) {
			m_viewController.update(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" }
{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	initScene();
	initShadersData();

	glEnable(GL_DEPTH_TEST);
	m_viewController.setSpeed(m_SceneSize * 0.1f); // Let's travel 10% of the scene per second

	glGenTextures(1, &DepthTextures);
	glBindTexture(GL_TEXTURE_2D, DepthTextures);
	glTexStorage2D(GL_TEXTURE_2D, 1, DepthTextures, m_nWindowWidth, m_nWindowHeight);

	glGenFramebuffers(1, &m_BufferFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BufferFBO);

}


void Application::initScene()
{
	glGenBuffers(1, &m_SceneVBO);
	glGenBuffers(1, &m_SceneIBO);

	{
		const auto objPath = m_AssetsRootPath / "glmlv" / "models" / "crytek-sponza" / "sponza.obj";
		glmlv::ObjData data;
		loadObj(objPath, data);
		m_SceneSize = glm::length(data.bboxMax - data.bboxMin);

		std::cout << "# of shapes    : " << data.shapeCount << std::endl;
		std::cout << "# of materials : " << data.materialCount << std::endl;
		std::cout << "# of vertex    : " << data.vertexBuffer.size() << std::endl;
		std::cout << "# of triangles    : " << data.indexBuffer.size() / 3 << std::endl;

		// Fill VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_SceneVBO);
		glBufferStorage(GL_ARRAY_BUFFER, data.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f2f), data.vertexBuffer.data(), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Fill IBO
		glBindBuffer(GL_ARRAY_BUFFER, m_SceneIBO);
		glBufferStorage(GL_ARRAY_BUFFER, data.indexBuffer.size() * sizeof(uint32_t), data.indexBuffer.data(), 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Init shape infos
		uint32_t indexOffset = 0;
		for (auto shapeID = 0; shapeID < data.indexCountPerShape.size(); ++shapeID)
		{
			m_shapes.emplace_back();
			auto & shape = m_shapes.back();
			shape.indexCount = data.indexCountPerShape[shapeID];
			shape.indexOffset = indexOffset;
			shape.materialID = data.materialIDPerShape[shapeID];
			indexOffset += shape.indexCount;
		}

		glGenTextures(1, &m_WhiteTexture);
		glBindTexture(GL_TEXTURE_2D, m_WhiteTexture);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 1, 1);
		glm::vec4 white(1.f, 1.f, 1.f, 1.f);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Upload all textures to the GPU
		std::vector<GLint> textureIds;
		for (const auto & texture : data.textures)
		{
			GLuint texId = 0;
			glGenTextures(1, &texId);
			glBindTexture(GL_TEXTURE_2D, texId);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, texture.width(), texture.height());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.data());
			glBindTexture(GL_TEXTURE_2D, 0);

			textureIds.emplace_back(texId);
		}

		for (const auto & material : data.materials)
		{
			PhongMaterial newMaterial;
			newMaterial.Ka = material.Ka;
			newMaterial.Kd = material.Kd;
			newMaterial.Ks = material.Ks;
			newMaterial.shininess = material.shininess;
			newMaterial.KaTextureId = material.KaTextureId >= 0 ? textureIds[material.KaTextureId] : m_WhiteTexture;
			newMaterial.KdTextureId = material.KdTextureId >= 0 ? textureIds[material.KdTextureId] : m_WhiteTexture;
			newMaterial.KsTextureId = material.KsTextureId >= 0 ? textureIds[material.KsTextureId] : m_WhiteTexture;
			newMaterial.shininessTextureId = material.shininessTextureId >= 0 ? textureIds[material.shininessTextureId] : m_WhiteTexture;

			m_SceneMaterials.emplace_back(newMaterial);
		}

		m_DefaultMaterial.Ka = glm::vec3(0);
		m_DefaultMaterial.Kd = glm::vec3(1);
		m_DefaultMaterial.Ks = glm::vec3(1);
		m_DefaultMaterial.shininess = 32.f;
		m_DefaultMaterial.KaTextureId = m_WhiteTexture;
		m_DefaultMaterial.KdTextureId = m_WhiteTexture;
		m_DefaultMaterial.KsTextureId = m_WhiteTexture;
		m_DefaultMaterial.shininessTextureId = m_WhiteTexture;
	}

	// Fill VAO
	glGenVertexArrays(1, &m_SceneVAO);
	glBindVertexArray(m_SceneVAO);

	const GLint positionAttrLocation = 0;
	const GLint normalAttrLocation = 1;
	const GLint texCoordsAttrLocation = 2;

	// We tell OpenGL what vertex attributes our VAO is describing:
	glEnableVertexAttribArray(positionAttrLocation);
	glEnableVertexAttribArray(normalAttrLocation);
	glEnableVertexAttribArray(texCoordsAttrLocation);

	glBindBuffer(GL_ARRAY_BUFFER, m_SceneVBO); // We bind the VBO because the next 3 calls will read what VBO is bound in order to know where the data is stored

	glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, position));
	glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, normal));
	glVertexAttribPointer(texCoordsAttrLocation, 2, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f2f), (const GLvoid*)offsetof(glmlv::Vertex3f3f2f, texCoords));

	glBindBuffer(GL_ARRAY_BUFFER, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_SceneIBO); // Binding the IBO to GL_ELEMENT_ARRAY_BUFFER while a VAO is bound "writes" it in the VAO for usage when the VAO will be drawn

	glBindVertexArray(0);

	// Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
	glGenSamplers(1, &m_textureSampler);
	glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Application::initShadersData()
{
	m_geometryPassProgram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "depthPass.vs.glsl", m_ShadersRootPath / m_AppName / "depthPass.fs.glsl" });

	m_uModelViewProjMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewProjMatrix");
	m_uModelViewMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uModelViewMatrix");
	m_uNormalMatrixLocation = glGetUniformLocation(m_geometryPassProgram.glId(), "uNormalMatrix");

}
