#include "rendering/frameManager.hpp"

#include "ECS/components/camera.hpp"
#include "ECS/components/spatial.hpp"
#include "rendering/rendering.hpp"
#include "rendering/resources/memory.hpp"
#include "core/settings.hpp"


frame_manager::frame_manager(rendering_system* core) : 
    _core(core)
{
    ;
}


void frame_manager::allocateUniformBuffers(bufferType type, uint32_t count)
{

    uint32_t framesInFlight = getSettingsData(_core->getRegistry()).framesInFlight;

    switch (type)
    {
        case bufferType::MODEL_MATRICES:
            _modelMatrices.buffers.resize(framesInFlight);
            _modelMatrices.buffers = _core->getMemorySystem().createUniformBuffers(sizeof(glm::mat4) * count, framesInFlight);
            break;
    }
}

void frame_manager::updateUniformBuffers(uint32_t currentImage)
{
    updateModelMatrices(currentImage);
    updateMVPMatrix(currentImage);
}

memoryBuffers& frame_manager::getModelMatrices()
{
    return _modelMatrices;
}

void frame_manager::cleanup()
{
    _core->getMemorySystem().freeBuffer(_modelMatrices);
}



void frame_manager::updateModelMatrices(uint32_t currentImage)
{
   // Update the model matrices
    std::vector<glm::mat4> modelMatrices;

    auto& registry = _core->getRegistry();
    auto entities = registry.view<Model>();

    for(auto& entity : entities)
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        position* pos = registry.try_get<position>(entity);
        if(pos != nullptr)
        {
            modelMatrix = glm::translate(modelMatrix, pos->value);
        }

        modelMatrices.push_back(modelMatrix);
    }

    memcpy(_modelMatrices.buffers[currentImage].mappedTo, modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));

}

void frame_manager::updateMVPMatrix(uint32_t currentImage)
{
    MVPMatrix ubo{};

    entt::entity activeCamera = _core->getScene()->getActiveCamera();

    const MVPMatrix& mvp = recalculateMVP(_core->getScene()->getRegistry(), activeCamera);

    ubo.view = mvp.view;
    ubo.model = mvp.model;
    ubo.projection = mvp.projection;
    ubo.projection[1][1] *= -1;

    auto& cameraUBOs = _core->getScene()->getRegistry().get<memoryBuffers>(activeCamera);

    memcpy(cameraUBOs.buffers[currentImage].mappedTo, &ubo, sizeof(ubo));
}