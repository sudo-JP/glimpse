#include "entity.hpp"
#include <algorithm>
namespace glimpse::renderer {
    template <typename T>
    Entity<T>::Entity(
        std::shared_ptr<const Mesh> mesh,
        std::shared_ptr<const Material> material,
        Uniform<T> uniform,
        const DescriptorAllocator& allocator
    ) :
    m_desciptor_sets(std::move(
        allocator.attach_resources<T>(
            uniform.get_uniform_buffers(),
            material.get()
        )
    )),
    m_mesh(std::move(mesh)),
    m_material(std::move(material)),
    m_uniform(std::move(uniform))
    {}

    template <typename T>
    Uniform<T>& Entity<T>::get_uniform() const {
        return m_uniform;
    }

    template <typename T>
    const Mesh& Entity<T>::get_mesh() const {
        return *m_mesh.get();
    }

} // namespace glimpse::renderer
