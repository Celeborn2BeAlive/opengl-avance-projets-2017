//
// Created by olivier on 23/12/15.
//

#ifndef PROJET_OPENGL_TESTSTATE_HPP
#define PROJET_OPENGL_TESTSTATE_HPP

#include <ANGL/Core/IState.hpp>



class TestState : public ANGL::IState {

    friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    /**
     * TestState constructor
     */
    TestState(ANGL::IApp* pApp);

    /**
     *
     */
    ~TestState();


    /**
     * Load textures and object in a string from a txt file
     */
    //virtual std::vector< std::vector< std::string > > loadFromFile();

    /**
     * Initializes the state and make it do some clean-up if needed
     */
    virtual void init();

    /**
        * Re-Initializes a state without reallocating everything inside.
        * This function should be called whenever a state is reset.
        */
    virtual void reinit();


    /**
     *
     */
    virtual void updateFixed();

    /**
     *
     */
    virtual void updateVariable(ANGL::Time elapsedTime);

    /**
     *
     */
    //virtual void handleEvents(SDL_Event pEvent);

    /**
     *
     */
    virtual void draw();

    /**
     *
     */
    virtual void handleCleanup();

private:

    glm::vec3 mClearColor;
};


#endif //PROJET_OPENGL_TESTSTATE_HPP
