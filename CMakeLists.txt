cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(Manta VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configure assets header file
configure_file(src/helpers/RootDir.h.in src/helpers/RootDir.hpp)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)

# Add static libraries folder
file(GLOB LIB_FILES 
    ${CMAKE_SOURCE_DIR}/lib/*.lib)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB_FILES})

# Fetch dependencies remotely
include(FetchContent)

# Supress dev warnings on subprojects
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

######################################## GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY 	https://github.com/glfw/glfw.git
  GIT_TAG        	3.3.2
)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

######################################## GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY 	https://github.com/g-truc/glm.git
  GIT_TAG 			0.9.9.0
  CONFIGURE_COMMAND ""
  BUILD_COMMAND 	""
)
FetchContent_MakeAvailable(glm)
target_link_libraries(${PROJECT_NAME} PUBLIC glm)

######################################## ENTT

FetchContent_Declare(
  entt
  GIT_REPOSITORY  https://github.com/skypjack/entt.git
  GIT_TAG         v3.13.x
)
FetchContent_MakeAvailable(entt)

target_link_libraries(${PROJECT_NAME} PUBLIC EnTT::EnTT)

######################################## STB_IMAGE
FetchContent_Declare(
  stb
  GIT_REPOSITORY 	https://github.com/nothings/stb.git
  GIT_TAG 			master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND 	""
)
set(STB_IMAGE_BUILD_TESTS OFF)
set(STB_IMAGE_BUILD_EXAMPLES OFF)
FetchContent_MakeAvailable(stb)
target_include_directories(${PROJECT_NAME} PUBLIC 
$<BUILD_INTERFACE:${stb_SOURCE_DIR}>)

######################################## ASSIMP
FetchContent_Declare(
  assimp
  GIT_REPOSITORY 	https://github.com/assimp/assimp.git
  GIT_TAG        	v5.3.0
)
option(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable Assimp's export functionality")
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "Disable Assimp's export functionality")
option(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
FetchContent_MakeAvailable(assimp)
target_link_libraries(${PROJECT_NAME} PUBLIC assimp)

######################################## glslang

FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG main  # Adjust the tag or commit hash as needed
)
set(GLSLANG_BUILD_TESTS OFF)
set(GLSLANG_BUILD_SPVTOOLS ON)
set(GLSLANG_BUILD_VULKAN ON)
set(GLSLANG_BUILD_SHARED OFF)
set(GLSLANG_BUILD_STATIC ON)
set(GLSLANG_VALIDATION_LAYER ON)
FetchContent_MakeAvailable(glslang)
target_link_libraries(${PROJECT_NAME} PUBLIC glslang SPIRV glslang-default-resource-limits)

######################################## SPIRV-Cross
FetchContent_Declare(
    spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG vulkan-sdk-1.3.280
)
FetchContent_MakeAvailable(spirv_cross)
target_link_libraries(${PROJECT_NAME} PUBLIC spirv-cross-core spirv-cross-glsl spirv-cross-hlsl spirv-cross-msl spirv-cross-reflect spirv-cross-util)

######################################## imGUI

# imGUI has no CMakeLists.txt, so we need to manually add the source files
file(GLOB IMGUI_SOURCE_FILES 
    ${CMAKE_SOURCE_DIR}/deps/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/deps/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/deps/imgui/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCE_FILES})
target_include_directories(imgui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/deps/imgui>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/deps/imgui/backends>
)

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

######################################## Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

######################################## Boost
# Boost is different than the others. Due to its size we don't want to build it, we want to download it
# as a pre-compiled library and link it to our project.
message(STATUS "Downloading pre-compiled boost library sources.")
set(FETCHCONTENT_QUIET TRUE) # Sets the output level of the build process to terminal
option(DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

set(BOOST_INCLUDE_LIBRARIES uuid filesystem)
set(BOOST_ENABLE_CMAKE ON)
set(Boost_NO_SYSTEM_PATHS ON)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)

FetchContent_Declare(
    Boost 
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
    USES_TERMINAL_DOWNLOAD TRUE 
    GIT_PROGRESS TRUE   
    DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::uuid Boost::filesystem)
install(TARGETS 
boost_uuid boost_assert boost_config boost_container_hash boost_core boost_io boost_move boost_numeric_conversion boost_predef boost_static_assert boost_throw_exception boost_tti boost_type_traits boost_winapi boost_describe boost_mp11 boost_random boost_mpl boost_preprocessor boost_conversion boost_function_types boost_array boost_dynamic_bitset boost_integer boost_range boost_utility 
boost_system boost_smart_ptr boost_detail boost_concept_check boost_iterator boost_optional boost_regex boost_tuple boost_variant2
boost_fusion boost_typeof boost_functional boost_function boost_bind boost_filesystem boost_atomic boost_align
EXPORT ${PROJECT_NAME}Config)

######################################## 
# Restore dev warnings
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)