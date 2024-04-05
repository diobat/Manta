#include "rendering/shaderManager.hpp"

#include "rendering/rendering.hpp"

#include <filesystem>
#include <fstream>
#include <set>  
#include <sstream>


namespace
{
    const std::set<std::string> shaderExtensions = { ".vert", ".frag", ".geom", ".tesc", ".tese"};

    const std::unordered_map<shaderType, std::string> shaderFileNames = {
        {shaderType::VERTEX, "vert.spv"},
        {shaderType::FRAGMENT, "frag.spv"},
        {shaderType::GEOMETRY, "geom.spv"},
        {shaderType::TESSELATION_CONTROL, "tesc.spv"},
        {shaderType::TESSELATION_EVALUATION, "tese.spv"}
    };

}

shader_system::shader_system(rendering_system* core)  :
    _core(core)
{
    ;
}

void shader_system::scanFolderRecursive(const std::string& path)
{
    std::string shaderProgramName = getShaderName(path);

    std::vector<std::string> shaderFilenames;
    std::set<std::string> unusedExtensions = shaderExtensions;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {

        if (entry.is_directory())
        {
            scanFolderRecursive(entry.path().string());
        }
        else if(entry.is_regular_file())
        {
            std::string extension = entry.path().extension().string();

            if(unusedExtensions.find(extension) != unusedExtensions.end())
            {
                unusedExtensions.erase(extension);
                shaderFilenames.push_back(entry.path().string());
            }
            else
            {
                std::runtime_error("Multiple shaders of the same type (" + extension +  ") found for shader program: " + shaderProgramName);
            }
            
        }
    }

    // After evaluating all the entries in the directory, 
    // ch-eck if at least vertex and fragment shaders are present
    if(     unusedExtensions.size() <  2 || 
            unusedExtensions.find(".vert") != unusedExtensions.end() || 
            unusedExtensions.find(".frag") != unusedExtensions.end())
    {
        std::stringstream ss;
        ss << "Missing required shaders for shader program: " << shaderProgramName << std::endl;
        for(const auto& ext : unusedExtensions)
        {
            ss << ext << std::endl;
        }
        return;
    }

    shaderProgram program;

    for(const auto& shaderFilename : shaderFilenames)
    {
        shaderModule module;

        if(isCompileNecessary(shaderFilename))
        {
            module = compileShader(shaderFilename);
        }
        else
        {
            module = loadShader(shaderFilename);
        }

        // Put it into the program struct
        program.shaders[static_cast<int>(module.type)] = module;
    }

    _shaderPrograms[shaderProgramName] = program;
}

shaderModule shader_system::compileShader(const std::string& path)
{
    shaderModule module;

    // Delete the old shader
    deleteOldShaderFile(path);

    glslang::InitializeProcess();

    EShLanguage stage;

    module.type = getShaderType(path);

    switch (module.type)
    {
    case shaderType::VERTEX:
        stage = EShLangVertex;
        break;
    case shaderType::FRAGMENT:
        stage = EShLangFragment;
        break;
    case shaderType::GEOMETRY:
        stage = EShLangGeometry;
        break;
    case shaderType::TESSELATION_CONTROL:
        stage = EShLangTessControl;
        break;
    case shaderType::TESSELATION_EVALUATION:
        stage = EShLangTessEvaluation;
        break;
    default:
        throw std::runtime_error("Unknown shader type for file: " + path);
        break;
    }

    // Create glslang shader object
    glslang::TShader shader{stage};
    shader.setDebugInfo(true);

    std::string shaderCode = readGLSLFile(path);
    const char* shaderCodeCStr = shaderCode.data();

    shader.setStrings(&shaderCodeCStr, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_4);

    auto includer = glslang::TShader::ForbidIncluder{};

    TBuiltInResource resources;
    // Compile the shader
    if(!shader.parse(   GetDefaultResources(),
                        glslang::EShTargetClientVersion::EShTargetVulkan_1_3, 
                        false, 
                        EShMsgDefault,
                        includer                        
                    )
        )
    {
        glslang::FinalizeProcess();

        std::cerr << "Shader info log:" << shader.getInfoLog() << std::endl;
        std::cerr << "Shader debug log:" << shader.getInfoDebugLog() << std::endl;
        throw std::runtime_error("Shader compilation failed. Check the error logs for details.");
        ;
    }

    // Convert the shader to SPIR-V
    glslang::TIntermediate* intermediate = shader.getIntermediate();
    spv::SpvBuildLogger logger;
    glslang::GlslangToSpv(*intermediate, module.code, &logger);

    //Check for errors
    if(logger.getAllMessages().length() > 0)
    {
        std::stringstream ss;

        ss << "GLSL to SPIR-V compilation failed for file: " << path;
        ss << logger.getAllMessages();
                
        glslang::FinalizeProcess();
        throw std::runtime_error(ss.str());
    }

    // Save the SPIR-V to a file
    std::filesystem::path spvPath(path.c_str());
    module.name = spvPath.stem().string();

    spvPath = spvPath.parent_path();
    spvPath /= shaderFileNames.at(module.type);

    std::ofstream file(spvPath, std::ios::binary);
    if(!file.is_open())
    {
        throw std::runtime_error("Failed to create file: " + spvPath.string());
    }
    file.write(reinterpret_cast<const char*>(module.code.data()), module.code.size() * sizeof(uint32_t));
    file.close();

    glslang::FinalizeProcess();


    // Create the vulkan shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = module.code.size() * sizeof(uint32_t);
    createInfo.pCode = module.code.data();

    if(vkCreateShaderModule(_core->getLogicalDevice(), &createInfo, nullptr, &module.VKmodule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return module;
}

shaderModule shader_system::loadShader(const std::string& path)
{
    shaderModule module;

    // Check file extension
    if(path.find(".spv") == std::string::npos)
    {
        throw std::runtime_error("Non-standard extension for file: " + path);
    }

    // Load the shader from the .spv file
    std::vector<char> SPVcode = readSPIRVFile(path);

    // Calculate the number of uint32_t elements needed to hold the data
    size_t numUint32Elements = SPVcode.size() / sizeof(uint32_t);
    
    // Create a vector to hold the converted data
    module.code.resize(numUint32Elements);

    // Copy the data from the char vector, interpreting it as uint32_t
    std::memcpy(module.code.data(), SPVcode.data(), SPVcode.size());

    // Create the vulkan shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = module.code.size() * sizeof(uint32_t);
    createInfo.pCode = module.code.data();

    if(vkCreateShaderModule(_core->getLogicalDevice(), &createInfo, nullptr, &module.VKmodule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return module;
}

const shaderProgram& shader_system::getShaderProgram(const std::string& name) const
{
    auto it = _shaderPrograms.find(name);
    if(it == _shaderPrograms.end())
    {
        throw std::runtime_error("Shader program not found: " + name);
    }

    return it->second;
}

const std::unordered_map<std::string, shaderProgram>& shader_system::getShaderPrograms() const
{
    return _shaderPrograms;
}

std::string shader_system::readGLSLFile(const std::string& filename) const
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = (size_t) file.tellg();
    std::string buffer(fileSize, ' ');
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

std::vector<char> shader_system::readSPIRVFile(const std::string& filename) const
{
        std::ifstream file;

    std::string fullpath = filename;

    file.open(fullpath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}


bool shader_system::deleteOldShaderFile(const std::string& path)
{
    std::filesystem::path sourcePath(path);
    std::filesystem::path spvPath(sourcePath.stem().string() + ".spv");

    if(std::filesystem::exists(spvPath))
    {
        std::filesystem::remove(spvPath);
        return true;
    }
    return false;
}

bool shader_system::isCompileNecessary(const std::string& path)
{
    std::filesystem::path sourcePath(path);
    std::filesystem::path spvPath(sourcePath.stem().string() + ".spv");

    if(!std::filesystem::exists(spvPath))
    {
        return true;
    }

    if(std::filesystem::last_write_time(spvPath) < std::filesystem::last_write_time(sourcePath))
    {
        return true;
    }

    return false;
}


shaderType shader_system::getShaderType(const std::string& path) const
{
    if(path.find(".vert") != std::string::npos)
    {
        return shaderType::VERTEX;
    }
    else if(path.find(".frag") != std::string::npos)
    {
        return shaderType::FRAGMENT;
    }
    else if(path.find(".geom") != std::string::npos)
    {
        return shaderType::GEOMETRY;
    }
    else if(path.find(".tesc") != std::string::npos)
    {
        return shaderType::TESSELATION_CONTROL;
    }
    else if(path.find(".tese") != std::string::npos)
    {
        return shaderType::TESSELATION_EVALUATION;
    }
    else
    {
        throw std::runtime_error("Non-standard extension / unknown shader type for file: " + path);
    }
}

std::string shader_system::getShaderName(const std::string& path) const
{

    // check if last character is a '/', if so, remove it
    // This is because filesystem::path.stem() does not work if the path ends with a '/'
    if(path.back() == '/')
    {
        return getShaderName(path.substr(0, path.size() - 1));
    }

    std::filesystem::path p(path);
    return p.stem().string();
}

VkShaderStageFlagBits shader_system::getVkShaderStageFlagBits(shaderType type) const
{
    switch (type)
    {
    case shaderType::VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case shaderType::FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    case shaderType::GEOMETRY:
        return VK_SHADER_STAGE_GEOMETRY_BIT;
        break;
    case shaderType::TESSELATION_CONTROL:
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        break;
    case shaderType::TESSELATION_EVALUATION:
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        break;
    default:
        return VK_SHADER_STAGE_ALL;
        break;
    }
}
