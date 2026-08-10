#pragma once

#include <GLFW/glfw3.h>
#include <expected>
#include <memory>

namespace glimpse {
    class Window {
    public: 
        static std::expected<Window, std::string> new_window(int width, int height, std::string title);
        ~Window() = default; 

        Window(Window&& other) noexcept = default;
        Window& operator=(Window&& other) noexcept = default; 

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    private: 
        Window(std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window);
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> m_window;
    };
}
