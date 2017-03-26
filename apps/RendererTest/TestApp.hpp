//
// Created by olivier on 18/12/15.
//

#ifndef PROJET_OPENGL_TESTAPP_HPP
#define PROJET_OPENGL_TESTAPP_HPP

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>
//#include <ANGL/Graphics/GraphicTypes.hpp>

#include <ANGL/Core/IApp.hpp>

class TestApp : public ANGL::IApp {

public:

    /**
     * TestApp constructor
     */
    TestApp();

    /**
     * TestApp de-constructor
     */
    ~TestApp();

protected:

    /**
     * initializes the application components such as the WindowManager or the StateManager
     * is called before the main loop
     */
    virtual void init();


    /**
     * performs some custom clean before exiting the application
     * is called before the main loop
     */
    virtual void handleCleanUp();

private:

};


#endif //PROJET_OPENGL_TESTAPP_HPP
