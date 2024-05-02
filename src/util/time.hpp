#pragma once

// GLFW
#include "wrapper/glfw.hpp"

// STD
#include <vector>


namespace FPSCounter
{

    int timeBufferSize = 20;


    std::vector<float> timeBuffer(timeBufferSize);
    float lastClick = 0.0;


    int getArraySize()
    {
        return timeBuffer.size();
    }

    void start()
    {
        timeBuffer.reserve(timeBufferSize);
    }

    void click()
    {
        float time = glfwGetTime();
        float deltaTime = time - lastClick;

        timeBuffer.push_back(deltaTime);
        if (timeBuffer.size() > timeBufferSize)
        {
            timeBuffer.erase(timeBuffer.begin());
        }

        lastClick = time;
    }

    const float* getTimes()
    {
        return timeBuffer.data();
    }


}