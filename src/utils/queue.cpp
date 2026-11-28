#include "queue.hpp"
#include <mutex>

namespace glimpse::utils {
template <typename T>
void ConcurrentQueue<T>::push(T item) {
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_queue.push(std::move(item));
    }
    m_cv.notify_one();
}

template <typename T>
T ConcurrentQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_cv.wait(lock, [this]() {
        return !m_queue.empty();
    });
    T item = std::move(m_queue.front());
    m_queue.pop();
    return item; 
}

template <typename T>
std::optional<T> ConcurrentQueue<T>::try_pop() {
    std::lock_guard<std::mutex> lock(m_mtx);
    if (m_queue.empty()) return std::nullopt;
    auto item = std::move(m_queue.front());
    m_queue.pop();
    return item;
}
} // namespace glimspe::utils
