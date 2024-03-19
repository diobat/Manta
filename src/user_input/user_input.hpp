#pragma once

// STD library includes
#include <memory>
#include <unordered_map>

// First-party includes
#include "ECS/ECS.hpp"

struct GLFWwindow;

class user_input_system
{
public:
	user_input_system(GLFWwindow* window);

	void executeCurrentInputs();
	void invokeCallback(int key);
	void bindToScene(std::shared_ptr<Scene> scene);

	bool bindKey(int key, std::function<void()> callback);
	void unbindKey(int key);

private:
	GLFWwindow* _window;

	std::unordered_map<int, std::function<void()>> callbackMap;
};



