#pragma once

#include <entt/entt.hpp>
#include "wrapper/glfw.hpp"
// For shader compiling
// #include <glslang/Include/Initialize.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/spirv.hpp>

#include <unordered_map>
#include <string>
#include <vector>

class rendering_system;


enum class shaderType
{
    VERTEX,
    FRAGMENT,
    GEOMETRY,
    TESSELATION_CONTROL,
    TESSELATION_EVALUATION
};

struct shaderModule
{
    std::string name;
    shaderType type;
    VkShaderModule VKmodule;
    std::vector<uint32_t> code;
};

// For clarification, a shader is a singular shader file, while a shader program is a collection of shaders that are linked together

class shader_system
{
public:
    shader_system(rendering_system* core);

    void scanFolderRecursive(const std::string& path);

    shaderModule compileShader(const std::string& shaderFilename);
    std::string readFile(const std::string& filename);

private:
    bool deleteOldShader(const std::string& path);
    bool isCompileNecessary(const std::string& path);

    shaderType getShaderType(const std::string& path);

    rendering_system* _core;
};