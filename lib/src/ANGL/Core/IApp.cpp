#include <ANGL/Core/IApp.hpp>

#include <ANGL/Core/ILogger.hpp>

namespace ANGL {

    IApp* IApp::gInstance = NULL;

    IApp::IApp() :
        mStateManager(this),
        mRunning(false),
        mUpdateRate(50),
        mMaxUpdates(5),
        mExitCode(0)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::ctor()"<< std::endl;

        gInstance = this;
        mUpdateInterval = (1000.0 / mUpdateRate);
    }

    IApp::~IApp()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::dtor()"<< std::endl;

        mRunning = false;

        if(this == gInstance) {
            gInstance = NULL;
        }
    }


    IApp *IApp::getApp()
    {
        return gInstance;
    }

    std::string IApp::getName()
    {
        return mName;
    }

    glmlv::fs::path IApp::getPath()
    {
        return mPath;
    }

    void IApp::processArguments(int pArgc, char **pArgv)
    {
        mPath = glmlv::fs::path{ pArgv[0] };
        mName = mPath.stem().string();

        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
        << "IApp::processArguments(" << pArgv[0] << ")"
        << ", mPath = " << mPath << ", mName = " << mName << std::endl;

        if(pArgc == 1) {
            ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                << "IApp::processArguments(" << pArgv[0] << ") command line : (none)" << std::endl;
        }
        else {
            ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                << "IApp::processArguments(" << pArgv[0] << ") command line :" << std::endl;
            for(int i = 0; i < pArgc; ++i) {
                ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                    << "Argument " << i << " = " << pArgv[i] << std::endl;
            }
        }
    }

    int IApp::run()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::run()" << std::endl;

        mRunning = true;

        // Initialize GLFW
        if(!glfwInit()) {
            ILogger::getLogger()->getStream(SeverityFatal, "IApp.cpp", __LINE__, StatusAppInitFailed)
                << "IApp::run() : failed to initialize GFLW" << std::endl;
        }

        // Do registrations here for the managers and helper classes
        // Do initializations here for the managers ans helper classes
        init();

        // Start the main loop
        loop();

        // Clean up the app
        cleanup();

        mRunning = false;

        if(mExitCode < 0) {
            ILogger::getLogger()->getStream(SeverityError, "IApp.cpp", __LINE__)
                << "IApp::run() : exitCode = " << mExitCode << std::endl;
        }
        else {
            ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                << "IApp::run() : exitCode = " << mExitCode << std::endl;
        }

        return mExitCode;
    }

    bool IApp::isRunning()
    {
        return mRunning;
    }

    float IApp::getUpdateRate()
    {
        return mUpdateRate;
    }

    void IApp::setUpdateRate(float pUpdateRate)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::setUpdateRate(" << pUpdateRate << ")"<< std::endl;

        mUpdateRate = pUpdateRate;
        mUpdateInterval = (1000.0 / mUpdateRate);
    }

    void IApp::setMaxUpdates(unsigned int pMaxUpdates)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::setMaxUpdates(" << pMaxUpdates<< ")"<< std::endl;

        if( 1 <= pMaxUpdates && pMaxUpdates <= 200)
        {
            mMaxUpdates = pMaxUpdates;
        }
    }

    void IApp::quit(int pExitCode)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::quit(" << pExitCode << ")"<< std::endl;

        mRunning = false;
        mExitCode = pExitCode;
    }

    void IApp::loop()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
            << "IApp::loop()" << std::endl;

        // Clocks to keep track of the time elapsed since last update and last frame
        Clock aUpdateClock;
        Clock aFrameClock;

        aUpdateClock.restart();

        // Time for the next update
        Time aNextUpdate = aUpdateClock.getElapsedTime();

        // Make sure we have at least one state active
        if(mStateManager.isEmpty())
        {
            ILogger::getLogger()->getStream(SeverityFatal, "IApp.cpp", __LINE__, StatusAppInitFailed)
                << "IApp::loop() : the application doesnt have any state." << std::endl;
        }
        //*/

        // Loop while the application is running
        //while(isRunning() && !mStateManager.isEmpty()) {
        while(isRunning())
        {
            // Get the currently active state
            IState* aState = mStateManager.getActiveState();

            // Reset the number of consecutive updates
            unsigned int aUpdates = 0;

            // Event loop
            processInput();


            Time aUpdateTime = aUpdateClock.getElapsedTime();

            while ((aUpdateTime - aNextUpdate) >= mUpdateInterval && aUpdates++ < mMaxUpdates)
            {
                aState->updateFixed();

                /*
                ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                    << "IApp::loop() : aUpdates = " << aUpdates
                    << ", aUpdateTime = " << aUpdateTime.asMilliseconds() << ", aNextUpdate = " << aNextUpdate.asMilliseconds()
                    << ", mUpdateRate = " << mUpdateRate << ", mUpdateIntervall = " << mUpdateInterval.asMilliseconds() << std::endl;
                //*/

                aNextUpdate += mUpdateInterval;
            }



            /*
            ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
                << "IApp::loop() : ElapsedFrame =" << aFrameClock.restart().asMilliseconds() << std::endl;
            //*/

            aState->updateVariable(aFrameClock.restart());

            aState->draw();

            if(WindowManager::isInitialized()) {
                WindowManager::swapBuffers();
            }

            if(WindowManager::shouldClose())
                quit();
        }
    }

    void IApp::processInput()
    {
        /*
        ILogger::getLogger()->getStream(SeverityDebug, "IApp.cpp", __LINE__)
            << "IApp::processInput()" << std::endl;
        //*/

        glfwPollEvents();
    }

    void IApp::cleanup()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "IApp.cpp", __LINE__)
        << "IApp::cleanup()" << std::endl;

        handleCleanUp();

        glfwTerminate();

        // Do de-initializations here for the managers ans helper classes
    }
} // namespace ANGL