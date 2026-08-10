#include <print>
#include "window/window.hpp"
#include <GLFW/glfw3.h>

int main() {
    glfwInit();
    auto window_result = glimpse::Window::new_window(800, 600, "Glimpse of ..."); 
    if (!window_result) {
        std::println("{}", window_result.error());
    }
    glimpse::Window &window = *window_result;
    glfwTerminate();
    return 0;
}
