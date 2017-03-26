#include <ANGL/Core/IState.hpp>

#include <ANGL/Core/ILogger.hpp>
#include <ANGL/Core/IApp.hpp>
#include <functional>


namespace ANGL {



    IState::IState(IApp* pApp, std::string pID):
        mApp(pApp),
        mInitialized(false),
        mCleanup(false),
        mID(pID),
        mElapsedTime(0),
        mTotalPausedTime(0),
        mElapsedClock(),
        mPausedClock()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::ctor()" << std::endl;
    }

    IState::~IState()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::dtor()" << std::endl;
    }

    std::string IState::getID() const
    {
        return mID;
    }

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        /*
        ILogger::getLogger()->getStream(SeverityDebug, "IState.cpp", __LINE__)
            << "IState::keyCallback() :  key = " << key << std::endl;
        //*/

        // Get a pointer on this state, which is the currently active one
        IState* state = IApp::getApp()->mStateManager.getActiveState();

        if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
            state->mApp->quit();
    }

    void IState::init()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::init() [\"" << mID << "\"]" << std::endl;

        if(mCleanup) {
            handleCleanup();
        }
        //*/

        if(!mInitialized) {

            mPaused = false;
            mElapsedTime = 0;
            mTotalPausedTime = 0;

            mElapsedClock.restart();
            mPausedClock.restart();

            glfwSetKeyCallback(WindowManager::getWindow(), &keyCallback);

            mInitialized = true;
        }
    }

    void IState::deinit()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::deinit() [" << mID << "]" << std::endl;

        if(mInitialized) {

            mCleanup = true;

            glfwSetKeyCallback(WindowManager::getWindow(), nullptr);

            mElapsedTime += mElapsedClock.getElapsedTime().asMilliseconds();

            if(mPaused) {
                mTotalPausedTime += mPausedClock.getElapsedTime().asMilliseconds();
            }

            mInitialized = false;
        }
    }

    bool IState::isInitialized() const
    {
         return mInitialized;
    }

    void IState::pause()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::pause() [" << mID << "]" << std::endl;

        if(!mPaused) {

            mPausedClock.restart();
            mPaused = true;
        }
    }

    void IState::resume()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::resume() [" << mID << "]" << std::endl;

        if(mPaused) {

            mTotalPausedTime += mPausedClock.getElapsedTime().asSeconds();
            mPaused = false;
        }
    }

    bool IState::isPaused()
    {
        return mPaused;
    }

    void IState::cleanup()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IState.cpp", __LINE__)
            << "IState::cleanup() [" << mID << "]" << std::endl;

        if(mCleanup) {

            handleCleanup();
            mCleanup = false;
        }
    }

    Time IState::getElapsedTime() const
    {
        return (mInitialized) ? mElapsedClock.getElapsedTime() : mElapsedTime;
    }
} // namespace ANGL