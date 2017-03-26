#include <ANGL/Graphics/Renderer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <imgui.h>
#include <random>

namespace ANGL {

    Renderer* Renderer::gInstance = NULL;

    Renderer::Renderer() :
        mViewController {WindowManager::getWindow(), m_ViewControllerSpeed, m_ViewControllerRotationSpeed}
    {
        gInstance = this;
    }

    Renderer *Renderer::getRenderer()
    {
        return gInstance;
    }


    void Renderer::UpdatePointLightSSBO(const std::vector<PointLight> & pointLights, const glm::mat4 & viewMatrix)
    {
        std::vector<glm::vec4> pointLightsPositionsStorageBuffer;
        std::vector<glm::vec4> pointLightsColorsStorageBuffer;
        std::vector<float> pointLightsIntensitiesStorageBuffer;
        std::vector<int> pointLightsEnabledStorageBuffer;

        for(int l = 0; l < pointLights.size(); ++l) {
            const PointLight &light = pointLights[l];

            pointLightsPositionsStorageBuffer.push_back(viewMatrix * glm::vec4(light.position, 1.f));
            pointLightsColorsStorageBuffer.push_back(glm::vec4(light.color, 1.f));
            pointLightsIntensitiesStorageBuffer.push_back(light.intensity);
            pointLightsEnabledStorageBuffer.push_back(light.enabled);
        }

        GLvoid *p;

        p = glMapNamedBuffer(m_pointLightPositionSSBO, GL_WRITE_ONLY);
        memcpy(p, pointLightsPositionsStorageBuffer.data(), pointLightsPositionsStorageBuffer.size() * sizeof(glm::vec4));
        glUnmapNamedBuffer(m_pointLightPositionSSBO);

        p = glMapNamedBuffer(m_pointLightColorSSBO, GL_WRITE_ONLY);
        memcpy(p, pointLightsColorsStorageBuffer.data(), pointLightsColorsStorageBuffer.size() * sizeof(glm::vec4));
        glUnmapNamedBuffer(m_pointLightColorSSBO);

        p = glMapNamedBuffer(m_pointLightIntensitySSBO, GL_WRITE_ONLY);
        memcpy(p, pointLightsIntensitiesStorageBuffer.data(), pointLightsIntensitiesStorageBuffer.size() * sizeof(float));
        glUnmapNamedBuffer(m_pointLightIntensitySSBO);

        p = glMapNamedBuffer(m_pointLightEnabledSSBO, GL_WRITE_ONLY);
        memcpy(p, pointLightsEnabledStorageBuffer.data(), pointLightsEnabledStorageBuffer.size() * sizeof(int));
        glUnmapNamedBuffer(m_pointLightEnabledSSBO);
    }


    void Renderer::Init()
    {
        mShadersRootPath = ANGL::IApp::getApp()->getPath().parent_path() / "shaders";

        ILogger::getLogger()->getStream(SeverityInfo, "Renderer.cpp", __LINE__)
        << "Renderer::Init()   "
        << ", mShadersRootPath = " << mShadersRootPath << std::endl;

        // Create texture sampler
        glCreateSamplers(1, &mTextureSampler);
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access

        glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.0);

        glEnable(GL_DEPTH_TEST);
//      glEnable (GL_BLEND);
//      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Compile shaders programs
        CompileGeometry();
        CompileSSAO();
        CompileShading();
        CompileDepth();
        CompilePosition();

        CreateSSBOs();

        CreateGBuffers();
        CreateSSAOTextures();

        // Create FrameBuffers
        CreateFrameBuffer_Geometry();
        CreateFrameBuffer_SSAO();

        CreateSSAOKernel();
        CreateSSAONoise();

        // Create display surface
        CreateDisplayMesh();
    }


    void Renderer::Draw(ModelGeometry pModel)
    {
        const glm::ivec2 viewportSize = WindowManager::getFramebufferSize();
        const glm::ivec2 windowSize = WindowManager::getWindowSize();

        glm::mat4 aViewMatrix = mViewController.getViewMatrix();
        glm::mat4 aProjMatrix = glm::perspective(70.f, (float) windowSize.x / windowSize.y, m_near, m_far);


        // Geometry Pass
        {
            m_geometryProgram.use();
            glBindFramebuffer(GL_FRAMEBUFFER, m_GBufferFBO);

            glViewport(0, 0, viewportSize.x, viewportSize.y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Send uniforms
            // Matrices
            glm::mat4 modelMatrix = glm::mat4();
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -5));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1, 0.1, 0.1));

            glm::mat4 modelViewMatrix = aViewMatrix * modelMatrix;
            glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelViewMatrix));
            glm::mat4 modelViewProjMatrix = aProjMatrix * modelViewMatrix;
            glUniformMatrix4fv(m_uModelViewProjMatrix_location, 1, GL_FALSE, &modelViewProjMatrix[0][0]);
            glUniformMatrix4fv(m_uModelViewMatrix_location, 1, GL_FALSE, &modelViewMatrix[0][0]);
            glUniformMatrix4fv(m_uNormalMatrix_location, 1, GL_FALSE, &normalMatrix[0][0]);

            // Textures
            glBindSampler(0, mTextureSampler);
            glBindSampler(1, mTextureSampler);
            glBindSampler(2, mTextureSampler);
            glBindSampler(3, mTextureSampler);
            glBindSampler(4, mTextureSampler);
            glBindSampler(5, mTextureSampler);

            glUniform1i(m_uKaSampler_location, 0);
            glUniform1i(m_uKdSampler_location, 1);
            glUniform1i(m_uKsSampler_location, 2);
            glUniform1i(m_uNsSampler_location, 3);
            glUniform1i(m_udSampler_location, 4);
            glUniform1i(m_uNormalSampler_location, 5);

            // Display the model
            pModel.Draw();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Ambient Occlusion pass
        {
            // use G-buffer to render SSAO texture
            m_SSAOProgram.use();
            glBindFramebuffer(GL_FRAMEBUFFER, m_SSAOFBO);

            glViewport(0, 0, viewportSize.x, viewportSize.y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            glBindSampler(0, mTextureSampler);
            glBindSampler(1, mTextureSampler);
            glBindSampler(2, mTextureSampler);

            glUniform1i(m_uGPositionSampler_location_SSAO, 0);
            glUniform1i(m_uGNormalSampler_location_SSAO, 1);
            glUniform1i(m_uNoiseSampler_location_SSAO, 2);

            glBindTextureUnit(0, m_GBufferTextures[GPosition]);
            glBindTextureUnit(1, m_GBufferTextures[GNormal]);
            glBindTextureUnit(2, m_SSAONoiseTexture);


            for (GLuint i = 0; i < 64; ++i)
                glUniform3fv(glGetUniformLocation(m_SSAOProgram.glId(), ("uKernel[" + std::to_string(i) + "]").c_str()), 1, &m_SSAOKernel[i][0]);
            glUniformMatrix4fv(glGetUniformLocation(m_SSAOProgram.glId(), "uProjMatrix"), 1, GL_FALSE, glm::value_ptr(aProjMatrix));

            glUniform2iv(m_uFramebufferSize_location_SSAO, 1, &viewportSize[0]);
            glUniform2iv(m_uFramebufferSize_location_SSAO, 1, &m_SSAONoiseTextureSize[0]);

            glUniform1f(m_uRadius_location_SSAO, mSSAORadius);
            glUniform1f(m_uBias_location_SSAO, mSSAOBias);


            glBindVertexArray(m_displayVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glViewport(0, 0, viewportSize.x, viewportSize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(m_CurrentlyDisplayed == Shaded) {
            // Shading pass
            {
                m_shadingProgram.use();

                // Lighting
                glm::vec4 viewDirectionalLightDir = aViewMatrix * glm::vec4(m_DirLightDirection, 0);
                viewDirectionalLightDir = glm::normalize(viewDirectionalLightDir);

                glUniform3f(m_uDirectionalLightDir_location, viewDirectionalLightDir[0], viewDirectionalLightDir[1], viewDirectionalLightDir[2]);
                glUniform3f(m_uDirectionalLightColor_location, directionalLightColor[0], directionalLightColor[1], directionalLightColor[2]);
                glUniform1f(m_uDirectionalLightIntensity_location, directionalLightIntensity);

                glUniform1i(m_uEnablePointLights_location, enablePointLights);
                glUniform1i(m_uNbPointLights_location, pointLights.size());

                // Build SSBO
                UpdatePointLightSSBO(pointLights, aViewMatrix);

                glBindSampler(0, mTextureSampler);
                glBindSampler(1, mTextureSampler);
                glBindSampler(2, mTextureSampler);
                glBindSampler(3, mTextureSampler);
                glBindSampler(4, mTextureSampler);
                glBindSampler(5, mTextureSampler);
                glBindSampler(6, mTextureSampler);

                glUniform1i(m_uGPositionSampler_location_shading, 0);
                glUniform1i(m_uGNormalSampler_location_shading, 1);
                glUniform1i(m_uGAmbientSampler_location, 2);
                glUniform1i(m_uGDiffuseSampler_location, 3);
                glUniform1i(m_uGlossyShininessSampler_location, 4);
                glUniform1i(m_uGDepthSampler_location_shading, 5);
                glUniform1i(m_uSSAOSampler_location_shading, 6);

                for (int32_t i = GPosition; i < GBufferTextureCount; ++i)
                    glBindTextureUnit(i, m_GBufferTextures[i]);

                glBindTextureUnit(6, m_SSAOTexture);

                glBindVertexArray(m_displayVAO);
                glDrawArrays(GL_TRIANGLES, 0, 3);
                glBindVertexArray(0);
            }
        }
        else if(m_CurrentlyDisplayed == GPosition) {
            // Position pass

            m_positionProgram.use();

            glBindSampler(0, mTextureSampler);
            glUniform1i(m_uGPositionSampler_location_position, 0);
            glBindTextureUnit(0, m_GBufferTextures[GPosition]);

            const auto rcpProjMat = glm::inverse(aProjMatrix);

            const glm::vec4 frustrumTopRight(1, 1, 1, 1);
            const auto frustrumTopRight_view = rcpProjMat * frustrumTopRight;

            glUniform3fv(m_uAdjustment_location_position, 1, glm::value_ptr(frustrumTopRight_view / frustrumTopRight_view.w));

            glBindVertexArray(m_displayVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

        }
        else if(m_CurrentlyDisplayed == GDepth) {
            // Depth pass
            m_depthProgram.use();

            glBindSampler(0, mTextureSampler);
            glUniform1i(m_uGDepthSampler_location_depth, 0);
            glBindTextureUnit(0, m_GBufferTextures[GDepth]);

            glUniform1f(m_uAdjustment_location_depth, m_far - m_near);

            glBindVertexArray(m_displayVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }
        else if(m_CurrentlyDisplayed == SSAO) {
            // Blit SSAO texture

            glNamedFramebufferReadBuffer(m_SSAOFBO, GL_COLOR_ATTACHMENT0);
            glBlitNamedFramebuffer(m_SSAOFBO, 0,
                                   0, 0, windowSize.x, windowSize.y,
                                   0, 0, windowSize.x, windowSize.y,
                                   GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }
        else {
            // Blit GBuffer texture
            glNamedFramebufferReadBuffer(m_GBufferFBO, GL_COLOR_ATTACHMENT0 + m_CurrentlyDisplayed);
            glBlitNamedFramebuffer(m_GBufferFBO, 0,
                                   0, 0, windowSize.x, windowSize.y,
                                   0, 0, windowSize.x, windowSize.y,
                                   GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        DrawGUI();
    }



    void Renderer::CompileGeometry()
    {
        std::string aAppName = IApp::getApp()->getName();

        m_geometryProgram = glmlv::compileProgram({ mShadersRootPath / aAppName / "GeometryPass.vs.glsl", mShadersRootPath / aAppName / "GeometryPass.fs.glsl" });

        m_uModelViewProjMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uModelViewProjMatrix");
        m_uModelViewMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uModelViewMatrix");
        m_uNormalMatrix_location = glGetUniformLocation(m_geometryProgram.glId(), "uNormalMatrix");

        m_uKa_location = glGetUniformLocation(m_geometryProgram.glId(), "uKa");
        m_uKaSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "uKaSampler");
        m_uKaMap_location = glGetUniformLocation(m_geometryProgram.glId(), "uKaMap");

        m_uKd_location = glGetUniformLocation(m_geometryProgram.glId(), "uKd");
        m_uKdSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "uKdSampler");
        m_uKdMap_location = glGetUniformLocation(m_geometryProgram.glId(), "uKdMap");

        m_uKs_location = glGetUniformLocation(m_geometryProgram.glId(), "uKs");
        m_uKsSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "uKsSampler");
        m_uKsMap_location = glGetUniformLocation(m_geometryProgram.glId(), "uKsMap");

        m_uNs_location = glGetUniformLocation(m_geometryProgram.glId(), "uNs");
        m_uNsSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "uNsSampler");
        m_uNsMap_location = glGetUniformLocation(m_geometryProgram.glId(), "uNsMap");

        m_ud_location = glGetUniformLocation(m_geometryProgram.glId(), "ud");
        m_udSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "udSampler");
        m_udMap_location = glGetUniformLocation(m_geometryProgram.glId(), "udMap");

        m_uNormalSampler_location = glGetUniformLocation(m_geometryProgram.glId(), "uNormalSampler");
        m_uNormalMap_location = glGetUniformLocation(m_geometryProgram.glId(), "uNormalMap");

        ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
        << "Renderer::CompilePosition()" << std::endl
        << "    m_uModelViewProjMatrix_location = " << m_uModelViewProjMatrix_location << std::endl
        << "    m_uModelViewMatrix_location = " << m_uModelViewMatrix_location << std::endl
        << "    m_uKa_location = " << m_uKa_location << std::endl
        << "    m_uKaSampler_location = " << m_uKaSampler_location << std::endl
        << "    m_uKaMap_location = " << m_uKaMap_location << std::endl
        << "    m_uKd_location = " << m_uKd_location << std::endl
        << "    m_uKdSampler_location = " <<  m_uKdSampler_location << std::endl
        << "    m_uKdMap_location = " << m_uKdMap_location << std::endl
        << "    m_uKs_location = " << m_uKs_location << std::endl
        << "    m_uKsSampler_location = " <<  m_uKsSampler_location << std::endl
        << "    m_uKsMap_location = " << m_uKsMap_location << std::endl
        << "    m_uNs_location = " << m_uNs_location << std::endl
        << "    m_uNsSampler_location = " <<  m_uNsSampler_location << std::endl
        << "    m_uNsMap_location = " << m_uNsMap_location << std::endl
        << "    m_ud_location = " << m_ud_location << std::endl
        << "    m_udSampler_location = " <<  m_udSampler_location << std::endl
        << "    m_udMap_location = " << m_udMap_location << std::endl
        << "    m_uNormalSampler_location = " << m_uNormalSampler_location << std::endl
        << "    m_uNormalMap_location = " <<  m_uNormalMap_location << std::endl;

    }

    void Renderer::CompileSSAO()
    {
        std::string aAppName = IApp::getApp()->getName();

        m_SSAOProgram = glmlv::compileProgram({ mShadersRootPath / aAppName / "ShadingPass.vs.glsl", mShadersRootPath / aAppName / "SSAOPass.fs.glsl" });

        m_uGPositionSampler_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uGPositionSampler");
        m_uGNormalSampler_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uGNormalSampler");
        m_uNoiseSampler_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uNoiseSampler");

        m_uKernel_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uKernel");
        m_uProjMatrix_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uProjMatrix");

        m_uFramebufferSize_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uFramebufferSize");
        m_uNoiseTileSize_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uNoiseTileSize");

        m_uRadius_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uRadius");
        m_uBias_location_SSAO = glGetUniformLocation(m_SSAOProgram.glId(), "uBias");

        ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
        << "Renderer::CompileSSAO()" << std::endl
        << "    m_uGPositionSampler_location_SSAO = " << m_uGPositionSampler_location_SSAO << std::endl
        << "    m_uGNormalSampler_location_SSAO = " << m_uGNormalSampler_location_SSAO << std::endl
        << "    m_uNoiseSampler_location_SSAO = " << m_uNoiseSampler_location_SSAO << std::endl

        << "    m_uKernel_location_SSAO = " << m_uKernel_location_SSAO << std::endl
        << "    m_uProjMatrix_location_SSAO = " << m_uProjMatrix_location_SSAO << std::endl

        << "    m_uFramebufferSize_location_SSAO = " << m_uFramebufferSize_location_SSAO << std::endl
        << "    m_uNoiseTileSize_location_SSAO = " << m_uNoiseTileSize_location_SSAO << std::endl;
    }

    void Renderer::CompileShading()
    {
        std::string aAppName = IApp::getApp()->getName();

        m_shadingProgram = glmlv::compileProgram({ mShadersRootPath / aAppName / "ShadingPass.vs.glsl", mShadersRootPath / aAppName / "ShadingPass.fs.glsl" });

        m_uGPositionSampler_location_shading = glGetUniformLocation(m_shadingProgram.glId(), "uGPositionSampler");
        m_uGNormalSampler_location_shading = glGetUniformLocation(m_shadingProgram.glId(), "uGNormalSampler");
        m_uGAmbientSampler_location = glGetUniformLocation(m_shadingProgram.glId(), "uGAmbientSampler");
        m_uGDiffuseSampler_location = glGetUniformLocation(m_shadingProgram.glId(), "uGDiffuseSampler");
        m_uGlossyShininessSampler_location = glGetUniformLocation(m_shadingProgram.glId(), "uGlossyShininessSampler");
        m_uGDepthSampler_location_shading = glGetUniformLocation(m_shadingProgram.glId(), "uGDepthSampler");
        m_uSSAOSampler_location_shading = glGetUniformLocation(m_shadingProgram.glId(), "uSSAOSampler");


        m_uDirectionalLightDir_location = glGetUniformLocation(m_shadingProgram.glId(), "uDirectionalLightDir");
        m_uDirectionalLightColor_location = glGetUniformLocation(m_shadingProgram.glId(), "uDirectionalLightColor");
        m_uDirectionalLightIntensity_location = glGetUniformLocation(m_shadingProgram.glId(), "uDirectionalLightIntensity");

        m_uEnablePointLights_location = glGetUniformLocation(m_shadingProgram.glId(), "uEnablePointLights");
        m_uNbPointLights_location = glGetUniformLocation(m_shadingProgram.glId(), "uNbPointLights");
    }

    void Renderer::CompileDepth()
    {
        std::string aAppName = IApp::getApp()->getName();

        m_depthProgram = glmlv::compileProgram({ mShadersRootPath / aAppName / "ShadingPass.vs.glsl", mShadersRootPath / aAppName / "DepthPass.fs.glsl" });

        m_uAdjustment_location_depth = glGetUniformLocation(m_depthProgram.glId(), "uAdjustment");
        m_uGDepthSampler_location_depth = glGetUniformLocation(m_depthProgram.glId(), "uGDepthSampler");
    }

    void Renderer::CompilePosition()
    {
        std::string aAppName = IApp::getApp()->getName();

        m_positionProgram = glmlv::compileProgram({ mShadersRootPath / aAppName / "ShadingPass.vs.glsl", mShadersRootPath / aAppName / "PositionPass.fs.glsl" });

        m_uAdjustment_location_position = glGetUniformLocation(m_positionProgram.glId(), "uAdjustment");
        m_uGPositionSampler_location_position = glGetUniformLocation(m_positionProgram.glId(), "uGPositionSampler");

        ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
        << "Renderer::CompilePosition()" << std::endl
        << "    m_uAdjustment_location_position = " << m_uAdjustment_location_position << std::endl
        << "    m_uGPositionSampler_location_position = " << m_uGPositionSampler_location_position << std::endl;
    }


    void Renderer::CreateSSBOs()
    {
        GLuint block_index;
        GLuint binding_point_index;
        glCreateBuffers(1, &m_pointLightPositionSSBO);
        glNamedBufferData(m_pointLightPositionSSBO, 1024, nullptr, GL_DYNAMIC_DRAW);
        block_index = glGetProgramResourceIndex(m_shadingProgram.glId(), GL_SHADER_STORAGE_BLOCK, "aPointLightPosition");
        //glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, pointLightPosition_binding);
        binding_point_index = 0;
        glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightPositionSSBO);
        //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightPositionSSBO, 0, 1024);

        glCreateBuffers(1, &m_pointLightColorSSBO);
        glNamedBufferData(m_pointLightColorSSBO, 1024, nullptr, GL_DYNAMIC_DRAW);
        block_index = glGetProgramResourceIndex(m_shadingProgram.glId(), GL_SHADER_STORAGE_BLOCK, "aPointLightColor");
        //glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, pointLightColor_binding);
        binding_point_index = 1;
        glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightColorSSBO);
        //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightColorSSBO, 0, 1024);

        glCreateBuffers(1, &m_pointLightIntensitySSBO);
        glNamedBufferData(m_pointLightIntensitySSBO, 1024, nullptr, GL_DYNAMIC_DRAW);
        block_index = glGetProgramResourceIndex(m_shadingProgram.glId(), GL_SHADER_STORAGE_BLOCK, "aPointLightIntensity");
        //glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, pointLightIntensity_binding);
        binding_point_index = 2;
        glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightIntensitySSBO);
        //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightColorSSBO, 0, 1024);

        glCreateBuffers(1, &m_pointLightEnabledSSBO);
        glNamedBufferData(m_pointLightEnabledSSBO, 1024, nullptr, GL_DYNAMIC_DRAW);
        block_index = glGetProgramResourceIndex(m_shadingProgram.glId(), GL_SHADER_STORAGE_BLOCK, "aPointLightEnabled");
        //glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, pointLightEnabled_binding);
        binding_point_index = 3;
        glShaderStorageBlockBinding(m_shadingProgram.glId(), block_index, binding_point_index);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightEnabledSSBO);
        //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point_index, m_pointLightColorSSBO, 0, 1024);
    }


    void Renderer::CreateGBuffers()
    {
        glm::ivec2 windowSize = WindowManager::getWindowSize();

        glCreateTextures(GL_TEXTURE_2D, GBufferTextureCount, m_GBufferTextures);
        for(int i = GPosition; i < GBufferTextureCount; ++i)
            glTextureStorage2D(m_GBufferTextures[i], 1, m_GBufferTextureFormat[i], windowSize.x, windowSize.y);

        ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
        << "Renderer::CreateGBuffers(), windowSize = (" << windowSize.x << ", " << windowSize.y << ")" << std::endl;


        glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAOTexture);
        glTextureStorage2D(m_SSAOTexture, 1, m_SSAOTextureFormat, windowSize.x, windowSize.y);
    }

    void Renderer::CreateSSAOTextures()
    {
        glm::ivec2 windowSize = WindowManager::getWindowSize();

        glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAOTexture);
        glTextureStorage2D(m_SSAOTexture, 1, m_SSAOTextureFormat, windowSize.x, windowSize.y);

        ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
        << "Renderer::CreateSSAOTextures(), windowSize = (" << windowSize.x << ", " << windowSize.y << ")" << std::endl;
    }

    void Renderer::CreateFrameBuffer_Geometry()
    {
        glCreateFramebuffers(1, &m_GBufferFBO);

        for(int i = GPosition; i < GBufferTextureCount - 1; ++i)
            glNamedFramebufferTexture(m_GBufferFBO, GL_COLOR_ATTACHMENT0 + i, m_GBufferTextures[i], 0);

        glNamedFramebufferTexture(m_GBufferFBO, GL_DEPTH_ATTACHMENT, m_GBufferTextures[GDepth], 0);

        GLenum drawBuffers[] =
                { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                  GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};

        glNamedFramebufferDrawBuffers(m_GBufferFBO, 5, drawBuffers);

        switch (glCheckNamedFramebufferStatus(m_GBufferFBO, GL_DRAW_FRAMEBUFFER)) {
            case GL_FRAMEBUFFER_COMPLETE :
                ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() Frame Buffer status : complete" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNDEFINED :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() Frame Buffer status : undefined, does not exist" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() Frame Buffer status : incomplete attachment" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() Frame Buffer status : incomplete missing attachment" << std::endl;
                break;
            default:
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() Frame Buffer status : other/unknowm error" << std::endl;
                break;
        }
    }

    void Renderer::CreateFrameBuffer_SSAO()
    {
        glCreateFramebuffers(1, &m_SSAOFBO);
        glNamedFramebufferTexture(m_SSAOFBO, GL_COLOR_ATTACHMENT0, m_SSAOTexture, 0);

        GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glNamedFramebufferDrawBuffers(m_SSAOFBO, 1, drawBuffers);
        //*/

        switch (glCheckNamedFramebufferStatus(m_SSAOFBO, GL_DRAW_FRAMEBUFFER)) {
            case GL_FRAMEBUFFER_COMPLETE :
                ILogger::getLogger()->getStream(SeverityDebug, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() SSAO Frame Buffer status : complete" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNDEFINED :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() SSAO Frame Buffer status : undefined, does not exist" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() SSAO Frame Buffer status : incomplete attachment" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() SSAO Frame Buffer status : incomplete missing attachment" << std::endl;
                break;
            default:
                ILogger::getLogger()->getStream(SeverityError, "Renderer.cpp", __LINE__)
                << "Renderer::CreateFrameBuffer_Geometry() SSAO Frame Buffer status : other/unknowm error" << std::endl;
                break;
        }
    }

    void Renderer::CreateSSAOKernel()
    {
        // Create the hemisphere kernel
        // The coordinates of the samples are in tangent space
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;
        for (GLuint i = 0; i < 64; ++i)
        {
            glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);
            GLfloat scale = GLfloat(i) / 64.0;

            /*
            GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
            {
                return a + f * (b - a);
            }
            // Scale samples s.t. they're more aligned to center of kernel
            //scale = lerp(0.1f, 1.0f, scale * scale);
            //*/
            sample *= scale;
            m_SSAOKernel.push_back(sample);
        }
    }

    void Renderer::CreateSSAONoise()
    {
        m_SSAONoiseTextureSize = glm::ivec2(4, 4);

        // Create the z-axis(in tangent space) noise texture for our kernels
        std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
        std::default_random_engine generator;
        // Noise texture
        std::vector<glm::vec3> ssaoNoise;
        for (GLuint i = 0; i < m_SSAONoiseTextureSize.x; i++)
            for (GLuint j = 0; j < m_SSAONoiseTextureSize.y; j++)
            {
                glm::vec3 noise(
                        randomFloats(generator) * 2.0 - 1.0,
                        randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
                ssaoNoise.push_back(noise);
            }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_SSAONoiseTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }


    void Renderer::CreateDisplayMesh(bool half)
    {
        glCreateBuffers(1, &m_displayVB0);

        float displayVBOData [] = {
                -1.f, -1.f, 0.f,
                3.f, -1.f, 0.f,
                -1.f, 3.f, 0.f
        };

        if(half) {
            displayVBOData[3] = 1.f;
            displayVBOData[7] = 1.f;
        }

        glNamedBufferStorage(m_displayVB0, sizeof(displayVBOData), displayVBOData, 0);

        glCreateVertexArrays(1, &m_displayVAO);
        glVertexArrayVertexBuffer(m_displayVAO, gVBOBindingIndex, m_displayVB0, 0, 3 * sizeof(float));

        glVertexArrayAttribBinding(m_displayVAO, gPositionAttr_binding_shading, gVBOBindingIndex);
        glEnableVertexArrayAttrib(m_displayVAO, gPositionAttr_binding_shading);
        glVertexArrayAttribFormat(m_displayVAO, gPositionAttr_binding_shading, 3, GL_FLOAT, GL_FALSE, 0);

        glVertexArrayElementBuffer(m_displayVAO,m_displayVB0);
    }



    void Renderer::DrawGUI_Renderer()
    {
        ImGui::Begin("GUI_Renderer");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::ColorEditMode(ImGuiColorEditMode_RGB);


        if (ImGui::ColorEdit3("clearColor", mClearColor)) {
            glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.f);
        }

        if(ImGui::DragFloat("cameraSpeed", &m_ViewControllerSpeed, 1.f, 0.1f, 100000.f, "%.3f", 1.4f)) {
            mViewController.setSpeed(m_ViewControllerSpeed);
        }

        if(ImGui::SliderFloat("cameraRotationSpeed", &m_ViewControllerRotationSpeed, 0.001f, 0.1f)) {
            mViewController.setRotationSpeed(m_ViewControllerRotationSpeed);
        }

        ImGui::DragFloat("near", &m_near, 1.f, 0.01f, 1.f, "%.3f", 1.2f);
        ImGui::DragFloat("far", &m_far, 1.f, 100.f, 1000000.f, "%.3f", 1.2f);

        ImGui::SliderFloat("SSAORadius", &mSSAORadius, 0.1f, 100.f);
        ImGui::SliderFloat("SSAOBias", &mSSAOBias, 0.001f, 0.1f);

        if(ImGui::CollapsingHeader("GBuffer"))
        {
            if (ImGui::RadioButton("Shaded", m_CurrentlyDisplayed == Shaded)){
                m_CurrentlyDisplayed = Shaded;
            }

            if (ImGui::RadioButton("SSAO", m_CurrentlyDisplayed == SSAO)){
                m_CurrentlyDisplayed = SSAO;
            }

            for (int32_t i = 0; i < GBufferTextureCount; ++i)
            {
                if (ImGui::RadioButton(m_GBufferTexNames[i], m_CurrentlyDisplayed == i)){
                    m_CurrentlyDisplayed = GBufferTextureType(i);
                }
            }
        }

        ImGui::End();
    }

    void Renderer::DrawGUI_Lights()
    {
        {
            ImGui::Begin("GUI_Lights");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::ColorEditMode(ImGuiColorEditMode_RGB);

            //ImGui::DragFloat3("directionalLightDir", &directionalLightDir[0]);
            if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.f) ||
                ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.f)) {
                m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
            }

            ImGui::ColorEdit3("directionalLightColor", &directionalLightColor[0]);
            ImGui::DragFloat("directionalLightIntensity", &directionalLightIntensity, 0.001f, 0.f, 2.f, "%.3f", 1.3f);

            ImGui::Checkbox("Enable point lights", &enablePointLights);
            for(int l = 0; l < pointLights.size(); ++l) {

                // TODO Check impact performance of ostringstream
                ImGui::Text("PointLight_%d", l);

                std::ostringstream prefix;
                prefix << "PointLight_" << l;
                std::ostringstream positionLabel, colorLabel, intensityLabel, enableLabel;
                positionLabel << prefix.str() << "_Position";
                colorLabel << prefix.str() << "_Color";
                intensityLabel << prefix.str() << "_Intensity";
                enableLabel << prefix.str() << "_Enabled";

                PointLight &light = pointLights[l];
                ImGui::Checkbox(enableLabel.str().c_str(), &(light.enabled));
                ImGui::DragFloat(intensityLabel.str().c_str(), &(light.intensity));
                ImGui::ColorEdit3(colorLabel.str().c_str(), &(pointLights[l].color[0]));
                ImGui::DragFloat3(positionLabel.str().c_str(), &(light.position[0]));
            }

            ImGui::End();
        }
    }

    void Renderer::DrawGUI()
    {
        ImGui_ImplGlfwGL3_NewFrame();

        DrawGUI_Renderer();
        DrawGUI_Lights();

        ImGui::Render();
    }
} // namespace ANGL



