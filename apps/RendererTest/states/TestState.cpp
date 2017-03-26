#include "TestState.hpp"

#include <ANGL/Core/ILogger.hpp>
#include <ANGL/Core/WindowManager.hpp>
#include <ANGL/Core/IApp.hpp>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.hpp>

using namespace ANGL;

TestState::TestState(ANGL::IApp *pApp) :
    IState(pApp, "Test"),
    mRenderer()
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
}

void TestState::init() {
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::init()" << std::endl;

    if(!isInitialized()) {


        mAssetsRootPath = ANGL::IApp::getApp()->getPath().parent_path() / "assets";

        ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::init()   "
        << ", mAssetsRootPath = " << mAssetsRootPath << std::endl;

        mRenderer.Init();

        std::string basedir = (mAssetsRootPath / IApp::getApp()->getName() / "models/crytek-sponza/").string();
        std::string inputfile = basedir + "sponza.obj";
        ModelLoader::Load(mModel, inputfile, basedir);
        mModel.BuildBuffers();
    }

    IState::init();

    // Override the default callbacks
    glfwSetKeyCallback(WindowManager::getWindow(), &keyCallback);
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

    auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    if (!guiHasFocus) {
        Renderer::getRenderer()->mViewController.update(elapsedTime.asSeconds());
    }
}

void TestState::draw()
{
    mRenderer.Draw(mModel);
}

void TestState::handleCleanup()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestState.cpp", __LINE__)
        << "TestState::handleCleanup()" << std::endl;
}