#include "entity.hpp"
#include "renderer/types.hpp"

namespace glimpse::renderer {

template <typename T> EntityBuilder<T>::EntityBuilder() {}

template <typename T>
Entity<T> EntityBuilder<T>::build(DescriptorAllocator &descriptor_allocator) {
    const auto &layout = m_material->get_descriptor_set_layout();

    descriptor_allocator.attach_resources<T>(
        m_uniform_buffer.get_uniform_buffers(), m_material);
    return Entity<T>(

    );
}

template <typename T> EntityBuilder<T> &EntityBuilder<T>::with_mesh(Mesh mesh) {
    m_mesh = std::move(mesh);
    return *this;
}

/*template Entity EntityBuilder::build<MVP>(
    DescriptorAllocator&
);*/
} // namespace glimpse::renderer
