#include "queue.hpp"

namespace glimpse::utils {
template <typename T>
void ConcurrentQueue<T>::push(T item) {
    std::lock_guard<std::mutex> lock(m_mtx);
    m_queue.push(item);
}

template <typename T>
T ConcurrentQueue<T>::pop() {
    
}

template <typename T>
std::optional<T> ConcurrentQueue<T>::try_pop() {
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_queue.empty()) return std::nullopt;
    auto item = m_queue.back();
    m_queue.pop();
    return item;
}
} // namespace glimspe::utils
