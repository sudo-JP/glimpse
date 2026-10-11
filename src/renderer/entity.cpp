#include "entity.hpp"
#include <algorithm>
namespace glimpse::renderer {
    template <typename T>
    Entity<T>::Entity(
        std::shared_ptr<const Mesh> mesh,
        std::shared_ptr<const Material> material,
        Uniform<T> uniform,
        DescriptorAllocator& allocator
    ) :
    m_descriptor_sets(std::move(
        allocator.attach_resources<T>(
            uniform.get_uniform_buffers(),
            *material.get()
        )
    )),
    m_mesh(std::move(mesh)),
    m_material(std::move(material)),
    m_uniform(std::move(uniform))
    {}

    template <typename T>
    Uniform<T>& Entity<T>::get_uniform() {
        return m_uniform;
    }

    template <typename T>
    const Mesh& Entity<T>::get_mesh() const {
        return *m_mesh.get();
    }

    template <typename T>
    const std::optional<std::reference_wrapper<const vk::raii::DescriptorSet>>
    Entity<T>::get_descriptor_set(size_t index) const {
        if (index >= m_descriptor_sets.size()) return std::nullopt;
        return m_descriptor_sets[index];
    }

    template <typename T>
    const Material& Entity<T>::get_material() const {
        return *m_material.get();
    }
    
    template class Entity<MVP>;
} // namespace glimpse::renderer
