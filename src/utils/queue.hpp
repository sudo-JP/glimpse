#include <mutex>
#include <queue>

namespace glimpse::utils {
template <typename T>
class ConcurrentQueue {
public:
    void push(T item);
    T pop();
    std::optional<T> try_pop();
private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
};
} // namespace glimspe::utils
