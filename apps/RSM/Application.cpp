#include "Application.hpp"

#include <iostream>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();

        // Put here rendering code




///////////////////////////////////////////////////////////SHADOW MAP COMPUTATION///////////////////////////////////////////////////////////
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, 512*4, 512*3);//SHADOWMAP RESOLUTION

        


        m_RSMprogram.use();


        //PREPARE TO UPDATE FBO
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_FBO);


        const auto projMatrix = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.01f,10.0f);
        


        const auto mvMatrix = glm::mat4(1);
        const auto mvpMatrix = projMatrix * mvMatrix;

        const glm::vec3 cubeMapDir[6] = 
        {
          glm::vec3(1, 0, 0),
            glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0),
            glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, -1)
        };

        const glm::vec3 cubeMapUpDirs[6] =
        {
            glm::vec3(0, -1, 0),
            glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, -1),
            glm::vec3(0, -1, 0),
            glm::vec3(0, -1, 0),
        };

        //1 VIEW/FACE
        for(int i=0;i<6;i++){
            m_cubeViewMatrix[i] = glm::lookAt(m_PointLightPosition, m_PointLightPosition + cubeMapDir[i], cubeMapUpDirs[i]);
        }

        //SET UNIFORMS        
        glUniformMatrix4fv(m_uProjMatLocation,1,GL_FALSE,glm::value_ptr(projMatrix));
        glUniformMatrix4fv(m_umvMatLocation,6,GL_FALSE,glm::value_ptr(m_cubeViewMatrix[0]));


        glBindVertexArray(m_sceneVAO);
        glDrawElements(GL_TRIANGLES, m_scene.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);

        //DRAW THE NEW SHADOW MAP (DEPTH + POSITION + NORMAL + DIFFUS)
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);








///////////////////////////////////////////////////////////RENDER PHASE///////////////////////////////////////////////////////////

        glViewport(0, 0, viewportSize.x, viewportSize.y);

        m_program.use();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //SEND TEXTURE FROM THE SHADOW MAP
        for(int i=0;i<4;i++){
            glActiveTexture(GL_TEXTURE0+i);
            glBindTexture(GL_TEXTURE_2D,m_GBufferTexture[i]);
        }

        //RANDOM 1D TEXTURE TO COMPUTE IRRADIANCE (see shader)
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_1D,randTex);

        const auto viewMatrix = m_viewController.getViewMatrix();

        glUniformMatrix4fv(m_uModelViewMatrixLocation,1,GL_FALSE,glm::value_ptr(viewMatrix));
        
        //BUG à cet envoi
        glUniformMatrix4fv(m_uModelViewProjMatrixLocation,1,GL_FALSE,glm::value_ptr(glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.f) * viewMatrix));
        glUniform3fv(m_uLightPosLocation,1,glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(m_PointLightPosition, 1))));

        glUniform1i(m_uDepthMapLocation, 0);
        glUniform1i(m_uPositionMapLocation, 1);
        glUniform1i(m_uNormalMapLocation, 2);
        glUniform1i(m_uDiffuseMapLocation, 3);
        glUniform1i(m_uRandMapLocation,4);
        glUniformMatrix4fv(m_uLightMVPMatrixLocation,1,GL_FALSE,glm::value_ptr(mvpMatrix));



        
        glDrawElements(GL_TRIANGLES, m_scene.indexBuffer.size(), GL_UNSIGNED_INT, nullptr);



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

    const GLint vboBindingIndex = 0; // Arbitrary choice between 0 and glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS)

    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint colorAttrLocation = 2;

    /////////////////////////////////////////////////////TEXTURES///////////////////////////////////////////////////////////


    //PREPARE 4 TEXTURES FROM THE FRAGMENT
    glGenTextures(4,m_GBufferTexture);

    for(int i=0;i<4;++i){
        glBindTexture(GL_TEXTURE_2D, m_GBufferTexture[i]);
        glTexStorage2D(GL_TEXTURE_2D,1,m_GBufferTextureFormat[i],512*4,512*3);
    }

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,m_FBO);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,m_GBufferTexture[0],0);
    for(int i=1;i<4;++i){
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+i,GL_TEXTURE_2D,m_GBufferTexture[i],0);
    }

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);


    //CREATE 1 TEXTURE FOR IRRADIANCE COMPUTATION (see fragment shader)
    float randValues[64*2];
    srand((unsigned long)time(0));
    for(int i=0;i<64*2;i++){
        randValues[i]=rand()/(float)RAND_MAX;
    }
    glTexImage1D(GL_TEXTURE_1D,0,GL_RED,64*2,0,GL_RED,GL_FLOAT,randValues);
    
    



    /////////////////////////////////////////////////////BUFFERS///////////////////////////////////////////////////////////

    //PREPARE BUFFERS AND FILL THEM
    m_scene = glmlv::makeScene();

    glGenBuffers(1, &m_sceneVBO);
    glGenBuffers(1, &m_sceneIBO);

    glBindBuffer(GL_ARRAY_BUFFER, m_sceneVBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_scene.vertexBuffer.size() * sizeof(glmlv::Vertex3f3f3f), m_scene.vertexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_sceneIBO);
    glBufferStorage(GL_ARRAY_BUFFER, m_scene.indexBuffer.size() * sizeof(uint32_t), m_scene.indexBuffer.data(), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    
    
    glGenVertexArrays(1, &m_sceneVAO);
    glBindVertexArray(m_sceneVAO);

    // We tell OpenGL what vertex attributes our VAO is describing:
    glEnableVertexAttribArray(positionAttrLocation);
    glEnableVertexAttribArray(normalAttrLocation);
    glEnableVertexAttribArray(colorAttrLocation);

    glBindBuffer(GL_ARRAY_BUFFER, m_sceneVBO); // We bind the VBO because the next 3 calls will read what VBO is bound in order to know where the data is stored

    glVertexAttribPointer(positionAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f3f), (const GLvoid*)offsetof(glmlv::Vertex3f3f3f, position));
    glVertexAttribPointer(normalAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f3f), (const GLvoid*)offsetof(glmlv::Vertex3f3f3f, normal));
    glVertexAttribPointer(colorAttrLocation, 3, GL_FLOAT, GL_FALSE, sizeof(glmlv::Vertex3f3f3f), (const GLvoid*)offsetof(glmlv::Vertex3f3f3f, Color));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // We can unbind the VBO because OpenGL has "written" in the VAO what VBO it needs to read when the VAO will be drawn

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sceneIBO); // Binding the IBO to GL_ELEMENT_ARRAY_BUFFER while a VAO is bound "writes" it in the VAO for usage when the VAO will be drawn

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);


    ///////////////////////////////////////////////////////////SHADERS///////////////////////////////////////////////////////////

    //PREPARE PROGRAMS & SHADERS : COMPILE + LOCATION
    m_program = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shading.vs.glsl", m_ShadersRootPath / m_AppName / "shading.fs.glsl" });
    m_program.use();

    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_program.glId(), "umpvMat");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_program.glId(), "umvMat");
    m_uLightPosLocation = glGetUniformLocation(m_program.glId(),"ulightPos");
    m_uLightMVPMatrixLocation = glGetUniformLocation(m_program.glId(), "ulightMvpMat");

    m_uDepthMapLocation = glGetUniformLocation(m_program.glId(), "udepthMap");
    m_uPositionMapLocation = glGetUniformLocation(m_program.glId(), "upositionMap");
    m_uNormalMapLocation = glGetUniformLocation(m_program.glId(), "unormalMap");
    m_uDiffuseMapLocation = glGetUniformLocation(m_program.glId(), "udiffuseMap");


    m_uRandMapLocation = glGetUniformLocation(m_program.glId(), "urandMap");
    


    m_viewController.setViewMatrix(glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));



    m_RSMprogram = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "rsm.vs.glsl",m_ShadersRootPath / m_AppName / "rsm.gs.glsl" ,  m_ShadersRootPath / m_AppName / "rsm.fs.glsl" });
    m_RSMprogram.use();

    m_uProjMatLocation = glGetUniformLocation(m_RSMprogram.glId(), "uprojMat");
    m_umvMatLocation = glGetUniformLocation(m_RSMprogram.glId(), "umvMat");

}
