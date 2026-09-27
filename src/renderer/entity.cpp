#include "entity.hpp"
#include "renderer/types.hpp"

namespace glimpse::renderer {
    
    template <typename T>
    EntityBuilder<T>::EntityBuilder() {}

    template <typename T>
    Entity<T> EntityBuilder<T>::build(
        glimpse::renderer::DescriptorAllocator& descriptor_allocator
    ) {
        descriptor_allocator.attach_resources<T>(
            m_uniform_buffer.get_uniform_buffers(), 
            m_texture     
        );
        return Entity<T>(

        );
    }

    template <typename T>
    EntityBuilder<T>& EntityBuilder<T>::with_mesh(glimpse::renderer::Mesh mesh) {
        m_mesh = std::move(mesh);
        return *this;
    }

    /*template Entity glimpse::renderer::EntityBuilder::build<glimpse::renderer::MVP>( 
        glimpse::renderer::DescriptorAllocator&
    );*/
}
