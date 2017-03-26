/**
 * @file Provides the implementation for the window manager
 * A window manager is responsible for initializing and updating a window for its application.
 *
 * @author Olivier Falconnet
 */

#pragma once

#include <ANGL/Common.hpp>
#include <ANGL/Core/CoreTypes.hpp>

#include <string>

namespace ANGL {

    class WindowManager {

    public:
        /**
         * WindowManager constructor
         */
        //WindowManager();

        /**
         * WindowManager constructor
         *
         * @param {std::string} pTitle The title for the window
         * @param {Uint32} pWidth The width for the window
         * @param {Uint32} pHeight The height for the window
         * @param {Uint32} pBitPerPixel The number of bits per pixel, i would recommend leaving this to 32
         */
        //WindowManager(std::string pTitle, unsigned intpWidth = 800, unsigned intpHeight = 600, unsigned intpBitPerPixel = 32);

        /**
         * WindowManager de-constructor
         */
        ~WindowManager();


        /**
        * Getters for window width and height
        */
        static glm::ivec2 getWindowSize();
        static glm::ivec2 getFramebufferSize();

        static GLFWwindow * getWindow();

        /**
         * Changes the title of the window
         *
         * @param {std::string} pTitle The new title for the window
         */
        static void setTitle(std::string pTitle);

        /**
         * Initializes SDL_VIDEO and GLEW. Opens a window.
         *
         * @param {std::string} pTitle The title for the window
         * @param {Uint32} pWidth The width for the window
         * @param {Uint32} pHeight The height for the window
         * @param {Uint32} pBitPerPixel The number of bits per pixel, i would recommend leaving this to 32
         */
        static void init(std::string pTitle, unsigned intpWidth = 800, unsigned intpHeight = 600);

        /**
         *
         */
        static bool isInitialized();


        /**
         *
         */
        static bool shouldClose();

        /**
         * Swap buffers and move to next frame.
         */
        static void swapBuffers();


        /**
         * Closes the window.
         */
        static void cleanup();

    private:

        static bool gInitialized;

        static GLFWwindow* gWindow;
    }; // class WindowManager
} // namespace ANGL
