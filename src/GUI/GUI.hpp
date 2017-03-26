#pragma once

#include <vector>

class CameraController;
class ResizeListener;

class GUI {
public:
    // Singleton accessor.
    static GUI& get();
    static void size(int& width, int& height);

    // Destructor.
    ~GUI();

    // Poll the events.
    bool pollEvents();

    // Render the window.
    void render() const;

    // Attach a camera controller to the GUI.
    void attach(CameraController& controller);
    void attach(ResizeListener& listener);


    GUI(GUI const&) = delete;
    GUI& operator=(GUI const&) = delete;

private:
    // Constructor.
    GUI();

    // The camera controller.
    CameraController* m_controller;

    // Resize listener.
    std::vector<ResizeListener*> m_lresize;
};


