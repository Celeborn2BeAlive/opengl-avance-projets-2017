#include "GUI/GUI.hpp"
#include "GUI/Log.hpp"
#include "GUI/CameraController.hpp"
#include "Importer/Importer.hpp"
#include "Renderer/Deferred.hpp"
#include "Scene/Scene.hpp"

#include <iostream>
#include <glad/glad.h>

int main(int, char**) {
    // Open gui.
    GUI& gui = GUI::get();
    Log::initializeLogGL();

    // Create renderer.
    Deferred renderer;
    gui.attach(renderer);

    // Load data.
    Importer::Data data;
    Importer::verbose(false);
    Importer::load(data, "crytek-sponza/sponza.obj");
    // Importer::load(data, "room/Room.obj");

    // Bind data to program.
    for (auto& m: data.meshes) {
        m.configure(renderer);
    }

    // Create the scene.
    Scene scene(std::move(data.objects), std::move(data.meshes), std::move(data.materials), std::move(data.textures));
    // scene.camera().up(glm::vec3(0.f, 1.f, 0.f));
    // scene.camera().eye(glm::vec3(-10.f, 0.f, 0.f));
    scene.camera().up(glm::vec3(0.f, 1.f, 0.f));
    scene.camera().eye(glm::vec3(0.f, 0.f, 0.f));

    // Set lighting.
    // renderer.light(Light(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.8f, 0.8f, 0.8f)));
    renderer.light(Light(glm::vec3(0.f, 1.2f, 0.4f), 16.f * glm::vec3(1.f, 1.f, 1.f)));
    
    // Add camera controller.
    CameraController controller;
    controller.attach(scene.camera());
    gui.attach(controller);
    controller.speed(scene.size());

    // Main loop.
    while (gui.pollEvents()) {
        scene.update();
        renderer.computeRSM(scene);
        renderer.draw(scene);
        // renderer.drawRSM();

        // int status = glGetError();
        // if (status != GL_NO_ERROR) {
        //     std::cout << status << std::endl;
        // }

        gui.render();
    }

    return 0;
}