#include <ANGL/Core/StateManager.hpp>

#include <ANGL/Core/ILogger.hpp>

namespace ANGL {

    StateManager::StateManager(IApp *pApp) :
        mApp(pApp),
        mActiveState(NULL)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "StateManager.cpp", __LINE__)
            << "StateManager::ctor()" << std::endl;
    }

    StateManager::~StateManager()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "StateManager.cpp", __LINE__)
            << "StateManager::dtor()" << std::endl;
    }


    bool StateManager::isEmpty()
    {
        return mStates.empty();
    }

    void StateManager::addState(IState *pState)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "StateManager.cpp", __LINE__)
            << "StateManager::addState(\"" << pState->getID() << "\")" << std::endl;

        mStates.insert(std::pair<std::string, IState*>(pState->getID(), pState));

        // If there is no active state, the inserted state also becomes teh active state
        if(NULL == mActiveState) {
            setActiveState(pState->getID());
        }
    }

    IState* StateManager::getActiveState()
    {
        return mActiveState;
    }

    void StateManager::setActiveState(std::string pID)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "StateManager.cpp", __LINE__)
            << "StateManager::setActiveState(\"" << pID << "\")" << std::endl;

        IState* aState = NULL;
        // Look for the state in our state map
        try {
            aState = mStates.at(pID);
        }
        catch (std::out_of_range) {
            ILogger::getLogger()->getStream(SeverityWarning, "StateManager.cpp", __LINE__)
                << "StateManager::setActiveState() : unknown state" << std::endl;

            return;
        }

        if(NULL != mActiveState) {
            mActiveState->deinit();
        }

        mActiveState = aState;
        mActiveState->init();
    }
} // namespace ANGL