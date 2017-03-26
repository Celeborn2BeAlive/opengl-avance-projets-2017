//
// Created by olivier on 23/12/15.
//

#ifndef PROJET_OPENGL_TESTSTATE_HPP
#define PROJET_OPENGL_TESTSTATE_HPP

#include <ANGL/Core/IState.hpp>

#include <ANGL/Graphics/Renderer.hpp>
#include <ANGL/Graphics/ModelGeometry.hpp>
#include <ANGL/Graphics/ModelLoader.hpp>
#include <ANGL/Core/WindowManager.hpp>
#include <glmlv/ViewController.hpp>


class TestState : public ANGL::IState {

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

    ANGL::Renderer mRenderer;


    ANGL::ModelGeometry mModel;

    //glmlv::fs::path mShadersRootPath;
    glmlv::fs::path mAssetsRootPath;

};


#endif //PROJET_OPENGL_TESTSTATE_HPP
