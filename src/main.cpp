#include <cstdlib>
#include <print>
#include "renderer/renderer.hpp"
#include <GLFW/glfw3.h>

int main() {
    auto renderer_res = glimpse::renderer::Renderer::new_renderer();
    if (!renderer_res) {
        std::println(stderr, "{}", std::move(renderer_res).error());
        return EXIT_FAILURE;
    }
    auto renderer = std::move(renderer_res).value();
    renderer.run();
}
