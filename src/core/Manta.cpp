#include "core/Manta.hpp"

Manta::Manta()
{
    init();
}

void Manta::run()
{
    while (!glfwWindowShouldClose(m_rendering->getWindow())) {
        glfwPollEvents();
        m_user_input->executeCurrentInputs();
        m_rendering->drawFrame();
    }

    m_rendering->cleanup();
}

void Manta::init()
{
    m_scene = std::make_shared<Scene>();
    m_rendering = std::make_shared<rendering_system>();
    m_user_input = std::make_shared<user_input_system>(m_rendering->getWindow());
    m_user_input->bindToScene(m_scene);

    entt::entity camera = m_scene->addCamera();	
    m_scene->setActiveCamera(camera);
    m_rendering->setScene(m_scene);
}

std::weak_ptr<Scene> Manta::getScene() const
{
    return m_scene;
}

std::weak_ptr<user_input_system> Manta::getUserInput() const
{
    return m_user_input;
}

std::weak_ptr<rendering_system> Manta::getRendering() const
{
    return m_rendering;
}

