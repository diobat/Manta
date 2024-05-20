#pragma once

#include <memory>
#include <list>

class rendering_system;
class StrategyNode;

class StrategyChain
{
public:
    StrategyChain(rendering_system* core);

    bool add(std::shared_ptr<StrategyNode> node);
    void clear();
    virtual bool reserveResources() { return true;};

    void run();

    rendering_system* core() const;
    uint32_t currentFrame() const { return _currentFrame; };

    template<typename T>
    std::shared_ptr<T> getNode() const
    {
    for (auto& node : _nodes)
    {
        const auto casted = std::dynamic_pointer_cast<T>(node);
        if (casted)
        {
            return casted;
        }
    }
    return nullptr;
    };

protected:
    void reserveNodeResources();

    virtual void beginRenderPass();
    virtual void endRenderPass();
    uint32_t _currentFrame;

    std::list<std::shared_ptr<StrategyNode>> _nodes;

    // The engine currently running this chain
    rendering_system* _core;

private:
    bool _firstRun;
};

class PBSShadingStrategyChain : public StrategyChain
{
public:
    PBSShadingStrategyChain(rendering_system* engine);
    bool reserveResources() override;
private:
    void bakeDiffuseIrradiance();
    void bakeSpecularIrradiance();
    void bakeBRDFLUT();
};