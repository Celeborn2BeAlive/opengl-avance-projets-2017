#include "TestState.hpp"

#include <ANGL/Core/ILogger.hpp>
#include <ANGL/Core/WindowManager.hpp>
#include <ANGL/Core/IApp.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.hpp>

using namespace ANGL;

TestState::TestState(ANGL::IApp *pApp) :
    IState(pApp, "Test")
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::ctor()" << std::endl;
}

TestState::~TestState()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::dtor()" << std::endl;
}

void TestState::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    /*
    ILogger::getLogger()->getStream(SeverityDebug, "IState.cpp", __LINE__)
        << "IState::keyCallback() :  key = " << key << std::endl;
    //*/

    // Get a pointer on this state, which is the currently active one
    TestState* state = dynamic_cast<TestState*>(IApp::getApp()->mStateManager.getActiveState());

    if(key == GLFW_KEY_R && action == GLFW_RELEASE)
        state->mClearColor = {1.0, 0.0, 0.0};

    if(key == GLFW_KEY_G && action == GLFW_RELEASE)
        state->mClearColor = {0.0, 1.0, 0.0};

    if(key == GLFW_KEY_B && action == GLFW_RELEASE)
        state->mClearColor = {0.0, 0.0, 1.0};
}

void TestState::init() {
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::init()" << std::endl;

    if(!isInitialized()) {

        mClearColor = {1.0, 0.0, 0.0};
    }

    IState::init();

    // Override the default callbacks
    glfwSetKeyCallback(WindowManager::getWindow(), (GLFWkeyfun) &keyCallback);
}

void TestState::reinit()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::reinit()" << std::endl;
}

void TestState::updateFixed()
{
    /*
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::updateFixed()" << std::endl;
    //*/
}

void TestState::updateVariable(ANGL::Time elapsedTime)
{
    /*
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::updateVariable(" << elapsedTime.asSeconds() << ")" << std::endl;
    //*/

    //rotation += elapsedTime.asSeconds() * rotationSpeed;
}

void TestState::draw()
{
    glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // GUI code:
    ImGui_ImplGlfwGL3_NewFrame();

    {
        ImGui::Begin("GUI_Test");
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
        if (ImGui::ColorEdit3("clearColor", &mClearColor.r)) {
            glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], 1.f);
        }

        ImGui::End();
    }

    ImGui::Render();
}

void TestState::handleCleanup()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::handleCleanup()" << std::endl;
}