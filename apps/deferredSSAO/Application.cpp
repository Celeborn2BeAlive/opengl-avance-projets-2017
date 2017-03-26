#include "Application.hpp"

#include <iostream>
#include <unordered_set>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/load_obj.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
	return a + f * (b - a);
}

int Application::run()
{
	float clearColor[3] = { 0, 0, 0 };
	int displayPass = GPosition;

	// Loop until the user closes the window
	for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
	{
		const auto seconds = glfwGetTime();

		// Put here rendering code
		const auto viewportSize = m_GLFWHandle.framebufferSize();
		
		//--------------------Start of geometry pass--------------------
		m_geomprogram.use();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

		glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f * m_SceneSize, m_SceneSize);
		const auto viewMatrix = m_viewController.getViewMatrix();

		const auto modelMatrix = glm::mat4();

		const auto mvMatrix = viewMatrix * modelMatrix;
		const auto mvpMatrix = projMatrix * mvMatrix;
		const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));

		glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		// Same sampler for all texture units
		glBindSampler(0, m_textureSampler);
		glBindSampler(1, m_textureSampler);
		glBindSampler(2, m_textureSampler);
		glBindSampler(3, m_textureSampler);

		// Set texture unit of each sampler
		glUniform1i(m_uKaSamplerLocation, 0);
		glUniform1i(m_uKdSamplerLocation, 1);
		glUniform1i(m_uKsSamplerLocation, 2);
		glUniform1i(m_uShininessSamplerLocation, 3);

		const auto bindMaterial = [&](const PhongMaterial & material)
		{
			glUniform3fv(m_uKaLocation, 1, glm::value_ptr(material.Ka));
			glUniform3fv(m_uKdLocation, 1, glm::value_ptr(material.Kd));
			glUniform3fv(m_uKsLocation, 1, glm::value_ptr(material.Ks));
			glUniform1fv(m_uShininessLocation, 1, &material.shininess);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material.KaTextureId);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material.KdTextureId);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material.KsTextureId);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, material.shininessTextureId);
		};

		glBindVertexArray(m_SceneVAO);

		const PhongMaterial * currentMaterial = nullptr;

		// We draw each shape by specifying how much indices it carries, and with an offset in the global index buffer
		for (const auto shape : m_shapes)
		{
			const auto & material = shape.materialID >= 0 ? m_SceneMaterials[shape.materialID] : m_DefaultMaterial;
			if (currentMaterial != &material)
			{
				bindMaterial(material);
				currentMaterial = &material;
			}
			glDrawElements(GL_TRIANGLES, shape.indexCount, GL_UNSIGNED_INT, (const GLvoid*) (shape.indexOffset * sizeof(GLuint)));
		}

		for (GLuint i : {0, 1, 2, 3})
			glBindSampler(i, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//--------------------End of geometry pass--------------------



		//--------------------Start of SSAO pass--------------------
		glBindSampler(0, m_textureSampler);
		glBindSampler(1, m_textureSampler);
		glBindSampler(2, m_textureSampler);

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			m_ssaoprogram.use();
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(m_uSSAOgPositionLocation, 0); // Set the uniform to 0 because we use texture unit 0
			glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);
			glActiveTexture(GL_TEXTURE1);
			glUniform1i(m_uSSAOgNormalLocation, 1);
			glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GNormal]);
			glActiveTexture(GL_TEXTURE2);
			glUniform1i(m_uSSAOtexNoiseLocation, 2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			//SendKernelSamplesToShader();
			for (GLuint i = 0; i < 64; ++i)
                glUniform3fv(glGetUniformLocation(m_ssaoprogram.glId(), ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssaoKernel[i][0]);

			glUniformMatrix4fv(m_uSSAOprojectionLocation, 1, GL_FALSE, glm::value_ptr(projMatrix));
			glUniform1f(m_uSSAOpowerLocation, ssaopower);

			glBindVertexArray(m_QuadVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		for (GLuint i : {0, 1, 2})
			glBindSampler(i, 0);


		glBindSampler(0, m_textureSampler);

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
			glClear(GL_COLOR_BUFFER_BIT);
			m_ssaoblurprogram.use();
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(m_uSSAOBlurinputLocation, 0);
			glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

			glBindVertexArray(m_QuadVAO);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//--------------------End of SSAO pass--------------------



		//--------------------Start of shading pass--------------------
		m_shadprogram.use();

		glViewport(0, 0, viewportSize.x, viewportSize.y);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
		glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));

		glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));
		glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));

		//texelFetch permet d'eviter d'avoir a lier des sampler ici
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(m_uGPositionLocation, 0); // Set the uniform to 0 because we use texture unit 0
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GPosition]);

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(m_uGNormalLocation, 1);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GNormal]);
		glActiveTexture(GL_TEXTURE2);
		glUniform1i(m_uGAmbientLocation, 2);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GAmbient]);
		glActiveTexture(GL_TEXTURE3);
		glUniform1i(m_uGDiffuseLocation, 3);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GDiffuse]);
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(m_uGlossyShininessLocation, 4);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[GGlossyShininess]);

		glActiveTexture(GL_TEXTURE5);
		glUniform1i(m_uSSAOTexLocation, 5);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

		//Dessiner un quad (nouv vbo, ibo, vao)
		glBindVertexArray(m_QuadVAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);

/*
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + displayPass);
		glBlitFramebuffer(0,0,m_nWindowWidth,m_nWindowHeight, 0,0,m_nWindowWidth,m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
*/
		// GUI code:
		ImGui_ImplGlfwGL3_NewFrame();

		{
			ImGui::Begin("GUI");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
			if (ImGui::ColorEdit3("clearColor", clearColor)) {
				glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
			}
			if (ImGui::Button("Sort shapes wrt materialID"))
			{
				std::sort(begin(m_shapes), end(m_shapes), [&](auto lhs, auto rhs)
				{
					return lhs.materialID < rhs.materialID;
				});
			}
			ImGui::DragFloat("SSAO Occlusion power", &ssaopower, 1.0f, 1.0f, 8.0f);
			if (ImGui::CollapsingHeader("Directional Light"))
			{
				ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
				ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.f, 100.f);
				if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
					ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
					m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
				}
			}

			if (ImGui::CollapsingHeader("Point Light"))
			{
				ImGui::ColorEdit3("PointLightColor", glm::value_ptr(m_PointLightColor));
				ImGui::DragFloat("PointLightIntensity", &m_PointLightIntensity, 0.1f, 0.f, 16000.f);
				ImGui::InputFloat3("Position", glm::value_ptr(m_PointLightPosition));
			}

			if (ImGui::CollapsingHeader("Textures")) {
				ImGui::RadioButton("GPosition", &displayPass, 0); ImGui::SameLine();
				ImGui::RadioButton("GNormal", &displayPass, 1); ImGui::SameLine();
				ImGui::RadioButton("GAmbient", &displayPass, 2); ImGui::SameLine();
				ImGui::RadioButton("GDiffuse", &displayPass, 3); ImGui::SameLine();
				ImGui::RadioButton("GGlossyShininess", &displayPass, 4);
			}

			ImGui::End();
		}

		
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
	m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
	m_AssetsRootPath { m_AppPath.parent_path() / "assets" }

{
	ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

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



	//SSAO variables initialisation
	//ssaoKernel
	for (GLuint i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator)
		);
		sample  = glm::normalize(sample);
		sample *= randomFloats(generator);
		GLfloat scale = GLfloat(i) / 64.0;
		scale   = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	//ssaoNoise
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			0.0f); 
		ssaoNoise.push_back(noise);
	}



	//Textures for defered
	glGenTextures(GBufferTextureCount, m_GBufferTextures);
	for (int i=0; i<GBufferTextureCount; i++) {
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
			//Parametres supplementaires sur les textures pour le SSAO
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GPosition, GL_TEXTURE_2D, m_GBufferTextures[GPosition], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GNormal, GL_TEXTURE_2D, m_GBufferTextures[GNormal], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GAmbient, GL_TEXTURE_2D, m_GBufferTextures[GAmbient], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GDiffuse, GL_TEXTURE_2D, m_GBufferTextures[GDiffuse], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GGlossyShininess, GL_TEXTURE_2D, m_GBufferTextures[GGlossyShininess], 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *(m_GBufferTextures+GDepth), 0);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(5, drawBuffers);
		/*if (glCheckFramebufferStatus(drawBuffers) != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Frame buffer glDrawBuffers failed." << std::endl;
			exit(EXIT_FAILURE);
		}*/

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//SSAO textures and frame buffer textures
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nWindowWidth, m_nWindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glGenTextures(1, &ssaoColorBufferBlur);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_nWindowWidth, m_nWindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	


	// Fill VAO
	glGenVertexArrays(1, &m_SceneVAO);
	glBindVertexArray(m_SceneVAO);

	//Variables in pour le vertex shader
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

	glEnable(GL_DEPTH_TEST);

	m_geomprogram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
	m_shadprogram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
	m_ssaoprogram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "SSAO.vs.glsl", m_ShadersRootPath / m_AppName / "SSAO.fs.glsl" });
	m_ssaoblurprogram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "SSAOBlur.vs.glsl", m_ShadersRootPath / m_AppName / "SSAOBlur.fs.glsl" });

	//Geometry pass uniforms
	m_uModelViewProjMatrixLocation = glGetUniformLocation(m_geomprogram.glId(), "uModelViewProjMatrix");
	m_uModelViewMatrixLocation = glGetUniformLocation(m_geomprogram.glId(), "uModelViewMatrix");
	m_uNormalMatrixLocation = glGetUniformLocation(m_geomprogram.glId(), "uNormalMatrix");

	m_uKaLocation = glGetUniformLocation(m_geomprogram.glId(), "uKa");
	m_uKdLocation = glGetUniformLocation(m_geomprogram.glId(), "uKd");
	m_uKsLocation = glGetUniformLocation(m_geomprogram.glId(), "uKs");
	m_uShininessLocation = glGetUniformLocation(m_geomprogram.glId(), "uShininess");
	m_uKaSamplerLocation = glGetUniformLocation(m_geomprogram.glId(), "uKaSampler");
	m_uKdSamplerLocation = glGetUniformLocation(m_geomprogram.glId(), "uKdSampler");
	m_uKsSamplerLocation = glGetUniformLocation(m_geomprogram.glId(), "uKsSampler");
	m_uShininessSamplerLocation = glGetUniformLocation(m_geomprogram.glId(), "uShininessSampler");

	//Shading pass uniforms
	m_uDirectionalLightDirLocation = glGetUniformLocation(m_shadprogram.glId(), "uDirectionalLightDir");
	m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_shadprogram.glId(), "uDirectionalLightIntensity");
	m_uPointLightPositionLocation = glGetUniformLocation(m_shadprogram.glId(), "uPointLightPosition");
	m_uPointLightIntensityLocation = glGetUniformLocation(m_shadprogram.glId(), "uPointLightIntensity");
	
	m_uGPositionLocation = glGetUniformLocation(m_shadprogram.glId(), "uGPosition");
	m_uGNormalLocation = glGetUniformLocation(m_shadprogram.glId(), "uGNormal");
	m_uGAmbientLocation = glGetUniformLocation(m_shadprogram.glId(), "uGAmbient");
	m_uGDiffuseLocation = glGetUniformLocation(m_shadprogram.glId(), "uGDiffuse");
	m_uGlossyShininessLocation = glGetUniformLocation(m_shadprogram.glId(), "uGlossyShininess");

	m_uSSAOTexLocation = glGetUniformLocation(m_shadprogram.glId(), "uSSAO");

	//SSAO pass uniforms
	m_uSSAOgPositionLocation = glGetUniformLocation(m_ssaoprogram.glId(), "gPosition");
	m_uSSAOgNormalLocation = glGetUniformLocation(m_ssaoprogram.glId(), "gNormal");
	m_uSSAOtexNoiseLocation = glGetUniformLocation(m_ssaoprogram.glId(), "texNoise");
	m_uSSAOsamplesLocation = glGetUniformLocation(m_ssaoprogram.glId(), "samples");
	m_uSSAOprojectionLocation = glGetUniformLocation(m_ssaoprogram.glId(), "projection");
	m_uSSAOpowerLocation = glGetUniformLocation(m_ssaoprogram.glId(), "power");

	m_uSSAOBlurinputLocation = glGetUniformLocation(m_ssaoblurprogram.glId(), "ssaoInput");



	m_viewController.setSpeed(m_SceneSize * 0.1f); // Let's travel 10% of the scene per second

	//Generate quad data and buffers
	glGenBuffers(1, &m_QuadVBO);

	glm::vec2 quadVertices[] = {
		glm::vec2(-1, -1), 
		glm::vec2(1, -1), 
		glm::vec2(1, 1), 
		glm::vec2(-1, 1)
	};

	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);

		glBufferStorage(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_QuadIBO);

	glBindBuffer(GL_ARRAY_BUFFER, m_QuadIBO);

		GLuint quadIndices[] = {
			0, 1, 2, // First triangle
			0, 2, 3 // Second triangle
		};

		glBufferStorage(GL_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_QuadVAO);
	glBindVertexArray(m_QuadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);

			//Position 0 dans le shad.vs
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIBO);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
