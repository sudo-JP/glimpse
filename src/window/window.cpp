#include "window.hpp"
#include <GLFW/glfw3.h>

namespace glimpse {
    std::expected<Window, std::string> Window::new_window(int width, int height, std::string title) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        const char *c_title = title.c_str();
        
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window{
            glfwCreateWindow(width, height, c_title, nullptr, nullptr),
            glfwDestroyWindow
        };
        if (!window) {
            return std::unexpected("Cannot create window ptr");
        }

        return Window(std::move(window));
    }

    Window::Window(std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window) 
        : m_window(std::move(window)) {}
}
