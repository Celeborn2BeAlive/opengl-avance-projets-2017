#include "GUI.hpp"
#include "CameraController.hpp"
#include "Listener.hpp"

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <stdexcept>

static int const INITIAL_WIDTH = 1000;
static int const INITIAL_HEIGHT = 700;

static SDL_Window* m_window = nullptr;
static SDL_Renderer* m_renderer = nullptr;
static SDL_GLContext m_context = nullptr;

static bool m_left = false;
static bool m_right = false;

static CameraController defaultController;

static unsigned int m_last = 0;


GUI& GUI::get() {
    static GUI gui;
    return gui;
}

void GUI::size(int& width, int& height) {
    SDL_GetWindowSize(m_window, &width, &height);
}

GUI::GUI() 
:   m_controller(&defaultController), m_lresize()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(std::string("Unable to initialize SDL: ") + SDL_GetError() + "\n");
    }

    m_window = SDL_CreateWindow("ReflectiveShadowMap", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INITIAL_WIDTH, INITIAL_HEIGHT, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!m_window) {
        throw std::runtime_error(std::string("Unable to create the window: ") + SDL_GetError() + "\n");
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        throw std::runtime_error(std::string("Unable to create the renderer: ") + SDL_GetError() + "\n");
    }

    SDL_GL_LoadLibrary(nullptr);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context) {
        throw std::runtime_error(std::string("Unable to create OpenGL context: ") + SDL_GetError() + "\n");
    }

    gladLoadGLLoader(SDL_GL_GetProcAddress);
    glEnable(GL_DEPTH_TEST);

    SDL_GL_SetSwapInterval(1);
}

GUI::~GUI() {
    if (m_window)   SDL_DestroyWindow(m_window);
    if (m_context)  SDL_GL_DeleteContext(m_context);
    SDL_Quit();
}

bool GUI::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)         m_left = event.button.state;
            else if (event.button.button == SDL_BUTTON_RIGHT)   m_right = event.button.state;
            break;

        case SDL_MOUSEWHEEL:
            m_controller->inputForward(event.wheel.y * 0.005f);
            break;

        case SDL_MOUSEMOTION:
            if (m_left)             { m_controller->inputYaw(event.motion.xrel * 0.002f); m_controller->inputPitch(event.motion.yrel * 0.002f); }
            if (m_right)            { m_controller->inputLateral(-event.motion.xrel * 0.0002f); m_controller->inputVertical(event.motion.yrel * 0.0002f); }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                glViewport(0, 0, event.window.data1, event.window.data2);
                m_controller->resize(event.window.data1, event.window.data2);

                for (auto& l: m_lresize) {
                    l->onResize(event.window.data1, event.window.data2);
                }
            }
            break;
        }
    }

    return true;
}

void GUI::render() const {
    static unsigned int const framerate = 1000 / 60;

    SDL_GL_SwapWindow(m_window);

    unsigned int elapsed = SDL_GetTicks() - m_last;
    if (elapsed < framerate) {
        SDL_Delay(framerate - elapsed);
    }
    
    m_last = SDL_GetTicks();
}

void GUI::attach(CameraController& controller) {
    m_controller = &controller;

    int w, h;
    SDL_GetWindowSize(m_window, &w, &h);
    m_controller->resize(w, h);
}

void GUI::attach(ResizeListener& listener) {
    m_lresize.emplace_back(&listener);
}

