#include <ANGL/Core/WindowManager.hpp>

#include <ANGL/Core/ILogger.hpp>
#include <imgui_impl_glfw_gl3.hpp>

namespace ANGL {

    bool WindowManager::gInitialized = false;
    GLFWwindow* WindowManager::gWindow = nullptr;

    /*
    WindowManager::WindowManager() :
        gInitialized(false),
        gTitle("LIMACE Is Merely A Correct Engine"),
        gWidth(800),
        gHeight(600),
        gBitPerPixel(32)
    {
    }
    //*/

    /*
    WindowManager::WindowManager(std::string pTitle, unsigned int pWidth, unsigned int pHeight, unsigned int pBitPerPixel) :
        gInitialized(false),
        gTitle(pTitle),
        gWidth(pWidth),
        gHeight(pHeight),
        gBitPerPixel(pBitPerPixel)
    {
    }
    //*/

    WindowManager::~WindowManager()
    {
    }

    glm::ivec2 WindowManager::getWindowSize()
    {
        int displayWidth, displayHeight;
        glfwGetWindowSize(gWindow, &displayWidth, &displayHeight);
        return glm::ivec2(displayWidth, displayHeight);
    }

    glm::ivec2 WindowManager::getFramebufferSize()
    {
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(gWindow, &displayWidth, &displayHeight);
        return glm::ivec2(displayWidth, displayHeight);
    }

    GLFWwindow* WindowManager::getWindow()
    {
        return gWindow;
    }


    void WindowManager::setTitle(std::string pTitle)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "WindowManager.cpp", __LINE__)
            << "WindowManager::setTitle(" << pTitle << ")" << std::endl;

        if(!isInitialized()) {
            ILogger::getLogger()->getStream(SeverityWarning, "WindowManager.cpp", __LINE__)
                << "WindowManager::setTitle() : The window isn't initialized" << std::endl;

            return;
        }

        glfwSetWindowTitle(gWindow, pTitle.c_str());
    }


    void WindowManager::init(std::string pTitle, unsigned int pWidth, unsigned int pHeight)
    {
        ILogger::getLogger()->getStream(SeverityInfo, "WindowManager.cpp", __LINE__)
            << "WindowManager::init(\"" << pTitle << "\", " << pWidth << ", " << pHeight << ")" << std::endl;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        // Open the window
        gWindow = glfwCreateWindow(int(pWidth), int(pHeight), pTitle.c_str(), nullptr, nullptr);
        if (!gWindow) {
            ILogger::getLogger()->getStream(SeverityFatal, "WindowManager.cpp", __LINE__, StatusAppInitFailed)
                << "WindowManager::init() : Couldn't create window" << std::endl;
        }

        glfwMakeContextCurrent(gWindow);

        glfwSwapInterval(0); // No VSync

        // Initialize OpenGL
        if (!gladLoadGL()) {
            ILogger::getLogger()->getStream(SeverityFatal, "WindowManager.cpp", __LINE__, StatusAppInitFailed)
                << "WindowManager::init() : Couldn't initialize OpenGL" << std::endl;
        }

        ILogger::getLogger()->getStream(SeverityInfo, "WindowManager.cpp", __LINE__)
            << "WindowManager::init() : OpenGL Version : " << glGetString(GL_VERSION) << std::endl;

        //glmlv::initGLDebugOutput();

        // TODO Setup ImGui binding
        ImGui_ImplGlfwGL3_Init(gWindow, true);

        gInitialized = true;
    }

    bool WindowManager::isInitialized()
    {
        return gInitialized;
    }

    bool WindowManager::shouldClose()
    {
        return (bool) glfwWindowShouldClose(gWindow);
    }

    void WindowManager::swapBuffers()
    {
        /*
        ILogger::getLogger()->getStream(SeverityInfo, "WindowManager.cpp", __LINE__)
            << "WindowManager::swapBuffers()" << std::endl;
        //*/

        if(!isInitialized()) {
            ILogger::getLogger()->getStream(SeverityWarning, "WindowManager.cpp", __LINE__)
                << "WindowManager::swapBuffers() : The window isn't initialized" << std::endl;

            return;
        }

        glfwSwapBuffers(gWindow);
    }

    void WindowManager::cleanup()
    {
        ILogger::getLogger()->getStream(SeverityInfo, "WindowManager.cpp", __LINE__)
            << "WindowManager::cleanup()" << std::endl;

        ImGui_ImplGlfwGL3_Shutdown();

        gInitialized = false;
    }
} // namespace ANGL
