/**
 * @file Provides the interface for an application class.
 * An application is responsible for running the main loop, and holding a variety of managers.
 *
 * IApp implements the Singleton pattern.
 * IApp makes use of the state pattern through StateManager.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>

#include <ANGL/Core/utililty/Time.hpp>
#include <ANGL/Core/WindowManager.hpp>
#include <ANGL/Core/StateManager.hpp>

#include <glmlv/filesystem.hpp>

namespace ANGL {

    class IApp {

    public:
        /**
         * IApp deconstructor
         */
        virtual ~IApp();


        /**
         *  @return {IApp*} a pointer to the most recently instantiated application
         */
        static IApp* getApp();

        /**
         * @return {std::string} the name of the application
         */
        std::string getName();

        /**
         * @return {std::string} the name of the application
         */
        glmlv::fs::path getPath();

        /*/
         * handles command line arguments
         *
         * @param {int} pArgc The number of arguments
         * @param {char**} pArgv The actual arguments
         */
        virtual void processArguments(int pArgc, char** pArgv);


        /**
         * Initializes and starts the application main loop,
         * it will also handle cleaning up once the main loop has ended
         *
         * This functions relies on the loop, init and cleanup methods,
         * which are defined by derived classes
         */
        int run();

        /**
         *  @return {bool} true if the application is running, false if not
         */
        bool isRunning();

        /**
         * @return {float} the update rate in Hz of the application
         */
        float getUpdateRate();

        /**
         * Sets the update rate in Hz for the application
         *
         * @param {float} pUpdateRate The new update rate in Hz for the application
         */
        void setUpdateRate(float pUpdateRate);

        /**
         * Sets the maximum consecutive updates for the application
         *
         * @param {float} pMaxUpdates The new maximum number of consecutive updates in Hz for the application
         */
        void setMaxUpdates(unsigned int pMaxUpdates);


        /**
         *
         */
        void quit(int pExitCode = StatusAppOK);


        StateManager mStateManager;
        // TODO : implement windowManager
        //WindowManager mWindowManager;
        //EventManager mEventManager;

    protected:

        /**
         * IApp constructor
         * Is protected because we only allow derived classes to instantiate this interface
         */
        IApp();

        /**
         * initializes the application components such as the WindowManager or the StateManager
         * is called before the main loop
         */
        virtual void init() = 0;

        /**
         * Contains the logic for the main loop of the application,
         * which involves updating its components,
         * handling input from the user, and giving him output (display)
         */
        virtual void loop();

        /**
         * Handles input from the user through GLFW
         */
        virtual void processInput();

        /**
         * performs some custom clean before exiting the application
         * is called before the main loop
         */
        virtual void handleCleanUp() = 0;

    private:

        static IApp* gInstance;

        std::string mName;
        glmlv::fs::path mPath;

        bool mRunning;

        float mUpdateRate;
        Time mUpdateInterval;
        unsigned int mMaxUpdates;

        int mExitCode;

        /**
         *
         */
        void cleanup();

        /**
         * IApp assignment operator
         * is private because we do not allow copies of a Singleton
         */
        IApp(const IApp&);               // Intentionally undefined

        /**
         * IApp assignment operator
         * is private because we do not allow copies of a Singleton
         */
        IApp& operator=(const IApp&);    // Intentionally undefined
    }; // class IApp
} // namespace ANGL
