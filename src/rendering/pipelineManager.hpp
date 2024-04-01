#pragma once 

#include "wrapper/glfw.hpp"

#include <unordered_map>

struct shaderPipeline
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
};


class pipeline_system
{
public:
    pipeline_system();

private:

    std::unordered_map<size_t, shaderPipeline> _pipelines;

};


// Descriptor Pool

//////// Descriptor Set Layout
/* 
    Describes the layout of a descriptor set. This is used to describe how the descriptor set is going to be used in the pipeline. For example, in the vertex shader, we might have a uniform buffer object that we want to pass to the shader, while in the fragment shader we might have a uniform image sampler. We would create a descriptor set layout that describes the layout of the descriptor set for the vertex shader and another descriptor set layout for the fragment shader.
*/

// Descriptor Set

// Descriptor