#include "window.hpp"
#include <GLFW/glfw3.h>

namespace glimpse {
    std::expected<Window, std::string> Window::new_window(int width, int height, std::string title) {
        if (m_ref_count == 0 && !glfwInit()) {
            return std::unexpected("cannot init glfw");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        const char *c_title = title.c_str();
        
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window{
            glfwCreateWindow(width, height, c_title, nullptr, nullptr),
            glfwDestroyWindow
        };
        if (!window) {
            return std::unexpected("cannot create window ptr");
        }

        return Window(std::move(window));
    }

    Window::Window(std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window) 
        : m_window(std::move(window)) {
        m_ref_count++;
    }

    GLFWwindow *Window::get_window() {
        return m_window.get();
    }

    Window::~Window() {
        if (m_window) {
            m_ref_count--; 
            m_window.reset();
            if (m_ref_count == 0) {
                glfwTerminate();
            }
        }
    }

    Window::Window(Window&& other) noexcept
    : m_window(std::move(other.m_window)) {}

    Window& Window::operator=(Window&& other) noexcept {
        if (this != &other) {
            if (m_window) {
                m_ref_count--;
                m_window.reset();
                if (m_ref_count == 0) {
                    glfwTerminate();
                }
            }
            m_window = std::move(other.m_window);
        }
        return *this;
    }

}
