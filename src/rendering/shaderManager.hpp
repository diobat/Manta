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
    VkShaderModule VKmodule = VK_NULL_HANDLE;
    std::vector<uint32_t> code;
};


struct shaderProgram
{
    std::string name;
    std::array<shaderModule, 5> shaders;
};

// For clarification, a shader is a singular shader file, while a shader program is a collection of shaders that are linked together

class shader_system
{
public:
    shader_system(rendering_system* core);

    void scanFolderRecursive(const std::string& path);

    shaderModule compileShader(const std::string& shaderFilename);
    shaderModule loadShader(const std::string& shaderFilename);

    const shaderProgram& getShaderProgram(const std::string& name) const;
    const std::unordered_map<std::string, shaderProgram>& getShaderPrograms() const;

    VkShaderStageFlagBits getVkShaderStageFlagBits(shaderType type) const;

    void cleanup();
private:

    std::string readGLSLFile(const std::string& filename) const;
    std::vector<char> readSPIRVFile(const std::string& filename) const ;

    bool deleteOldShaderFile(const std::string& path);
    bool isCompileNecessary(const std::string& path);

    shaderType getShaderType(const std::string& path) const;
    std::string getShaderName(const std::string& path) const;

    std::unordered_map<std::string, shaderProgram> _shaderPrograms;

    rendering_system* _core;
};