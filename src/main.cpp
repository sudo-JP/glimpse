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
    
    const auto window = std::move(window_result).value();
    const GLFWwindow *window_ptr = window.get_window();

    glimpse::Version version {
        1, 0, 0
    };
    glimpse::ContextAppInfo app_info {
        "app", version
    };

    glimpse::ContextAppInfo engine_ctx {
        "engine", version
    };

    auto vk_ctx_res = glimpse::renderer::VulkanContext::new_vk_context(
        app_info, engine_ctx, window, true
    );
    if (!vk_ctx_res) {
        std::println(stderr, "{}", vk_ctx_res.error());
        return -1; 
    }
    glimpse::renderer::VulkanContext vk_ctx = std::move(*vk_ctx_res);
    

    /*glfwMakeContextCurrent(window_ptr);
    glfwShowWindow(window_ptr);

    while (!glfwWindowShouldClose(window_ptr)) {
        if (glfwGetKey(window_ptr, GLFW_KEY_Q)) {
            glfwSetWindowShouldClose(window_ptr, true);
        }

        glfwPollEvents();
    }*/
}
