/**
 * @file Provides the implementation for the state manager.
 * A state manager is responsible for holding and switching between the different states of its application.
 *
 * StateManager is the pivot element for the state pattern.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>

#include <ANGL/Core/IState.hpp>
#include <ANGL/Core/StateManager.hpp>

#include <map>
#include <string>

namespace ANGL {

    class StateManager {

    public:

        /**
         * StateManager constructor
         *
         * @param {IApp*} pApp a pointer to the app containing this state manager
         */
        StateManager(IApp* pApp);

        /**
         * StateManager de-constructor
         */
        ~StateManager();


        /**
         * @return {bool} true if there is no registered state for the app
         */
        bool isEmpty();

        /**
         * Inserts a new state into the known states for the app
         * If the app had no state, this state will be made the active state by default
         *
         * @param {IState*} pState A pointer to the state
         */
        void addState(IState* pState);

        /**
         * @returns {IState&} a reference to the currently active state
         */
        IState* getActiveState();

        /**
         * Set the state with pID as the new active state
         * Does nothing if the app has no state with this ID
         *
         * @param {std::string} pID The ID of the state to be activated
         */
        void setActiveState(std::string pID);

    private:

        IApp* mApp;

        std::map<std::string, IState*> mStates;
        IState* mActiveState;
    }; // class StateManager
} // namespace ANGL
