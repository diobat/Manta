#pragma once

#include <boost/uuid/uuid.hpp>  // UUID's for descriptor sets

class StrategyChain;

class StrategyNode
{
public:
    StrategyNode(const StrategyChain* chain);
    virtual void run() = 0;
    virtual void prepare() {};
protected:
    const StrategyChain* _chain;
};


class RenderSkyboxNode : public StrategyNode
{
public:
    RenderSkyboxNode(const StrategyChain* chain);
    void run() override;
    void prepare() override;
};

class RenderOpaqueNode : public StrategyNode
{
public:
    RenderOpaqueNode(const StrategyChain* chain);
    void run() override;
    void prepare() override;
private:
    boost::uuids::uuid _ds; // One descriptor set per frame in flight
};

class renderGUIOnFrameStartNode : public StrategyNode
{
    renderGUIOnFrameStartNode(const StrategyChain* chain);
    void run() override;
    void prepare() override;
};

class renderGUIOnFrameEndNode : public StrategyNode
{
    renderGUIOnFrameEndNode(const StrategyChain* chain);
    void run() override;
    void prepare() override;
};