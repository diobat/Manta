#include "core/Manta.hpp"

#include "core/settings.hpp"
#include "ECS/ECS.hpp"
#include "rendering/rendering.hpp"
#include "user_input/user_input.hpp"

Manta::Manta() 
{
    init();
}

void Manta::run()
{
    m_rendering->firstTimeSetup();

    while (!glfwWindowShouldClose(m_rendering->getWindow())) {
        glfwPollEvents();
        m_user_input->executeCurrentInputs();
        m_rendering->drawFrame();
    }

    m_rendering->cleanup();
}

void Manta::init()
{
    m_scene = std::make_shared<Scene>(this, _registry);
    initializeSettingsData(m_scene->getRegistry());
    
    m_rendering = std::make_shared<rendering_system>(m_scene);
    m_user_input = std::make_shared<user_input_system>(m_rendering->getWindow());
    m_user_input->bindToScene(m_scene);

    entt::entity camera = m_scene->addCamera();	
    m_scene->setActiveCamera(camera);
    m_rendering->setScene(m_scene);

    m_rendering->initRender();
}

entt::registry& Manta::getRegistry()
{
    return _registry;
}

std::weak_ptr<settingsData> Manta::getSettings() const
{
    return _settings;
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

