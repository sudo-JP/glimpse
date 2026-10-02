#include "entity.hpp"
#include <algorithm>
namespace glimpse::renderer {
    template <typename T>
    Entity<T>::Entity(
        std::shared_ptr<const Mesh> mesh,
        std::shared_ptr<const Material> material,
        UniformBuffer<T> uniform_buffer,
        const DescriptorAllocator& allocator
    ) : 
    m_desciptor_sets(std::move(
        allocator.attach_resources<T>(
            uniform_buffer.get_uniform_buffers(), 
            material.get()
        )
    )),
    m_mesh(std::move(mesh)),
    m_material(std::move(material)),
    m_uniform_buffer(std::move(uniform_buffer))
    {}

} // namespace glimpse::renderer
