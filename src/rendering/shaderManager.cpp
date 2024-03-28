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

    std::string shaderProgramName;

    std::string vertexShaderFilename = "";
    std::string fragmentShaderFilename = "";
    std::string geometryShaderFilename = "";
    std::string tessellationControlShaderFilename = "";
    std::string tessellationEvaluationShaderFilename = "";

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
            shaderProgramName = entry.path().parent_path().stem().string();
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
        // The files have now been gathered and are also unique (no two shaders of the same type in the same folder)

        // Now we verify for each file if it is necessary to compile it
        // We do this by checking if the .spv file exists and if it has a newer timestamp than the source file
        // If the .spv file does not exist, we compile the shader

        for(const auto& shaderFilename : shaderFilenames)
        {
            if(isCompileNecessary(shaderFilename))
            {

            }
            else
            {
                // Load the shader from the .spv file

            }

        }




    }
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


std::string shader_system::readFile(const std::string& filename)
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

bool shader_system::deleteOldShader(const std::string& path)
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

shaderModule shader_system::compileShader(const std::string& path)
{
    shaderModule module;

    // Delete the old shader
    deleteOldShader(path);

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

    std::string shaderCode = readFile(path);
    const char* shaderCodeCStr = shaderCode.data();

    shader.setStrings(&shaderCodeCStr, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClient::EShClientOpenGL, glslang::EShTargetClientVersion::EShTargetOpenGL_450);
    shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_3);
    shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_3);

    auto includer = glslang::TShader::ForbidIncluder{};

    TBuiltInResource resources;
    // Compile the shader
    if(!shader.parse(   GetDefaultResources(),
                        glslang::EShTargetClientVersion::EShTargetOpenGL_450, 
                        false, 
                        EShMsgDefault,
                        includer                        
                    )
        )
    {
        glslang::FinalizeProcess();

        std::cerr << "Shader info log:" << shader.getInfoLog() << std::endl;
        std::cerr << "Shader debug log:" << shader.getInfoDebugLog() << std::endl;

        // std::string s1 = shader.getInfoLog();
        // std::string s2 = shader.getInfoDebugLog();
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



    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = module.code.size() * sizeof(uint32_t);
    createInfo.pCode = module.code.data();

    if(vkCreateShaderModule(_core->getLogicalDevice(), &createInfo, nullptr, &module.VKmodule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return module;
}

shaderType shader_system::getShaderType(const std::string& path)
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