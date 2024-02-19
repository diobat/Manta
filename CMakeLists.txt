cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(Vox VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
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

######################################## STB_IMAGE
FetchContent_Declare(
  stb
  GIT_REPOSITORY 	https://github.com/nothings/stb.git
  GIT_TAG 			master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND 	""
)
FetchContent_MakeAvailable(stb)
target_include_directories(${PROJECT_NAME} PUBLIC 
$<BUILD_INTERFACE:${stb_SOURCE_DIR}>)

######################################## tinyobjloader
FetchContent_Declare(
  tiny
  GIT_REPOSITORY 	https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG 			release
  CONFIGURE_COMMAND ""
  BUILD_COMMAND 	""
)
FetchContent_MakeAvailable(tiny)
target_include_directories(${PROJECT_NAME} PUBLIC 
$<BUILD_INTERFACE:${tiny_SOURCE_DIR}>)

######################################## Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
######################################## 
# Restore dev warnings
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)