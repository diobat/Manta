#include "user_input/user_input.hpp"

#include <map>

// GLFW includes
#include "wrapper/glfw.hpp"

#include "ECS/components/camera.hpp"

//Because openGL runs in C code it has no idea what classes and this-> are.
//Its much more convoluted to implement callbacks as non-member functions but 
//I have little choice. 
namespace
{
	GLFWwindow* renderWindow;
	std::shared_ptr<Scene> boundScene;
	std::map<int, bool> keyMap;
	bool captureMouse = true;
	user_input_system* scanner;

	std::array<double, 2> lastMousePos = {0.0, 0.0};

	void resetCursorPos()
	{
		int width, height;
		glfwGetWindowSize(renderWindow, &width, &height);
		glfwSetCursorPos(renderWindow, width / 2, height / 2);
		glfwSetCursorPos(renderWindow, lastMousePos[0], lastMousePos[1]);
	}

	void setLastMousePosToCurrent()
	{
		glfwGetCursorPos(renderWindow, &lastMousePos[0], &lastMousePos[1]);
	}

	void toggleMouseMode()
	{
		captureMouse = !captureMouse;

		if (captureMouse)
		{
			glfwSetInputMode(renderWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			setLastMousePosToCurrent();
		}
		else
		{
			glfwSetInputMode(renderWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		keyMap[key] = action;

		if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		{
			toggleMouseMode();
		}

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	void mouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (!captureMouse)	// We only capture input if the setting in toggled
		{
			return;
		}

		double xPos_delta = lastMousePos[0] - xpos;
		double yPos_delta = lastMousePos[1] - ypos;

		// Get and update the camera
		auto cam = boundScene->getActiveCamera();
		rotateCamera(boundScene->getRegistry(), cam, {static_cast<float>(xPos_delta), static_cast<float>(yPos_delta)});
		lastMousePos = {xpos, ypos};
	}
}

user_input_system::user_input_system(GLFWwindow* window) :
	_window(window)
{
	renderWindow = window;

	scanner = this;

	// Keyboard callbacks
	glfwSetKeyCallback(_window, keyboardCallback);
	// Mouse callbacks
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouseCallback);
	resetCursorPos();

}

void user_input_system::bindToScene(std::shared_ptr<Scene> scene)
{
	boundScene = scene;

	callbackMap.clear();

	// Initialize the default callbackMap
	// WASD
	bindKey(GLFW_KEY_W, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::FORWARD)));
	bindKey(GLFW_KEY_A, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::LEFT)));
	bindKey(GLFW_KEY_S, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::BACKWARD)));
	bindKey(GLFW_KEY_D, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::RIGHT)));
	bindKey(GLFW_KEY_Q, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::UP)));
	bindKey(GLFW_KEY_E, std::bind(&Scene::moveActiveCamera, boundScene , static_cast<unsigned int>(relativeDirections::DOWN)));

}

void user_input_system::invokeCallback(int key)
{
	auto it = callbackMap.find(key);
	if (it != callbackMap.end()) 
	{
		it->second(); // Call the callback function
	}
}

void user_input_system::executeCurrentInputs()
{
	for (auto& key : keyMap)
	{
		if (key.second != GLFW_RELEASE)
		{
			invokeCallback(key.first);
		}
	}
}

bool user_input_system::bindKey(int key, std::function<void()> callback)
{

	if (callbackMap.count(key) == 0)
	{
		callbackMap[key] = callback;
		return true;
	}

	return false;
}

void user_input_system::unbindKey(int key)
{
	if (callbackMap.count(key) != 0)
	{
		callbackMap.erase(key);
	}
}