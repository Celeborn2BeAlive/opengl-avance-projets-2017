#include "TestApp.hpp"
#include "states/TestState.hpp"

#include <ANGL/Core/WindowManager.hpp>
#include <ANGL/Core/ILogger.hpp>

using namespace ANGL;

TestApp::TestApp()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestApp.cpp", __LINE__)
        << "TestApp::ctor()" << std::endl;
}

TestApp::~TestApp()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestApp.cpp", __LINE__)
        << "TestApp::dtor()" << std::endl;
}

void TestApp::init()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestApp.cpp", __LINE__)
        << "TestApp::init()" << std::endl;

    WindowManager::init("ANGL", 1280, 720);
    mStateManager.addState(new TestState(this));
}

void TestApp::handleCleanUp()
{
    ILogger::getLogger()->getStream(SeverityInfo, "TestApp.cpp", __LINE__)
        << "TestApp::cleanup()" << std::endl;

    WindowManager::cleanup();
}
