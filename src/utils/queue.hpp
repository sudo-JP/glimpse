#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace glimpse::utils {
template <typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue<T>() = default;
    void push(T item);
    T pop();
    std::optional<T> try_pop();
private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};
} // namespace glimpse::utils
