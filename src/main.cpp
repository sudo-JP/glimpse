#include <print>
#include "renderer/context.hpp"
#include "window/window.hpp"
#include <GLFW/glfw3.h>

int main() {
    auto window_result = glimpse::Window::new_window(800, 600, "Glimpse of ..."); 
    if (!window_result) {
        std::println("{}", window_result.error());
        return -1;
    }
    
    glimpse::Window &window = *window_result;
    GLFWwindow *window_ptr = window.get_window();

    glfwMakeContextCurrent(window_ptr);
    glfwShowWindow(window_ptr);

    while (!glfwWindowShouldClose(window_ptr)) {
        if (glfwGetKey(window_ptr, GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window_ptr, true);
        }

        glfwPollEvents();
    }
}
