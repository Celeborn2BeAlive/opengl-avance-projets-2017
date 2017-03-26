/**
 * @file Provides the interface for an application state.
 * An application state is responsible for holding relevant information, and providing methods needed for the main loop.
 *
 * IState is the basic element for the state pattern.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>

#include <ANGL/Core/utililty/Time.hpp>
#include <ANGL/Core/utililty/Clock.hpp>

#include <string>

namespace ANGL {

    class IState {

    friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    public:

        /**
         * IState de-constructor
         */
        ~IState();


        /**
         * @return {std:string} the ID of the state
         */
        std::string getID() const ;

        /**
         * Initializes the state and make it do some clean-up if needed
         */
        virtual void init();

        /**
         * Re-Initializes a state without reallocating everything inside.
         * This function should be called whenever a state is reset.
         */
        virtual void reinit() = 0;

        /**
         * Un-Initializes a state and marks it for clean-up
         * This function should be called whenever a state becomes inactive.
         */
        void deinit();

        /**
         *
         */
        bool isInitialized() const;


        /**
         *
         */
        void pause();

        /**
         *
         */
        void resume();

        /**
         *
         */
        bool isPaused();

        /**
         *
         */
        virtual void updateFixed() = 0;

        /**
         *
         */
        virtual void updateVariable(Time elapsedTime) = 0;

        /**
         *
         */
        virtual void draw() = 0;


        /**
         *
         */
        void cleanup();

        /**
         *
         */
        Time getElapsedTime() const;


    protected:
        /**
         * IState constructor
         * Is protected because we only allow derived classes to instantiate this interface
         *
         * @param {std::string} pId The ID for the state
         * @param {IApp*} pApp The address of the app to which pertains this state
         */
        IState(IApp* pApp, std::string pID = "");

        // Address of the app to which pertains this state
        IApp* mApp;

        /**
         *
         */
        virtual void handleCleanup() = 0;

    private:

        bool mInitialized;
        bool mPaused;
        bool mCleanup;

        const std::string mID;

        Time mElapsedTime;
        Time mTotalPausedTime;

        Clock mElapsedClock;
        Clock mPausedClock;

        /**
         * IState assignment operator
         * is private because we do not allow copies of a Singleton
         */
        IState(const IState&);               // Intentionally undefined

        /**
         * IState assignment operator
         * is private because we do not allow copies of a Singleton
         */
        IState& operator=(const IState&);    // Intentionally undefined

    }; // class IState
} // namespace ANGL
