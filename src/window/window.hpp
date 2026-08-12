#pragma once

#include <GLFW/glfw3.h>
#include <expected>
#include <memory>

namespace glimpse {
    class Window {
    public: 
        // Constructor like rust cuz i hate this language
        static std::expected<Window, std::string> new_window(
            int width, int height, std::string title
        );

        ~Window(); 

        Window(Window&& other) noexcept;
        Window& operator=(Window&& other) noexcept; 

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        GLFWwindow *get_window(); 
    private: 
        static inline size_t m_ref_count = 0; 
        Window(std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window);
        std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> m_window{nullptr, glfwDestroyWindow};
    };
}
