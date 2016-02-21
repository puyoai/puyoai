#ifndef BASE_BLOCKING_QUEUE_H_
#define BASE_BLOCKING_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>

namespace base {

template<typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(size_t capacity);

    bool empty() const;
    size_t size() const;
    size_t available() const { return capacity_ - size(); }
    size_t capacity() const { return capacity_; }

    void push(const T& v);
    T take();

private:
    mutable std::mutex mu_;
    mutable std::condition_variable push_cond_var_;
    mutable std::condition_variable take_cond_var_;
    const size_t capacity_;
    std::queue<T> q_;
};

template<typename T>
BlockingQueue<T>::BlockingQueue(size_t capacity) :
    capacity_(capacity)
{
}

template<typename T>
bool BlockingQueue<T>::empty() const
{
    std::unique_lock<std::mutex> lock(mu_);
    return q_.empty();
}

template<typename T>
size_t BlockingQueue<T>::size() const
{
    std::unique_lock<std::mutex> lock(mu_);
    return q_.size();
}

template<typename T>
void BlockingQueue<T>::push(const T& v)
{
    std::unique_lock<std::mutex> lock(mu_);
    while (capacity_ <= q_.size()) {
        push_cond_var_.wait(lock);
    }

    q_.push(v);
    take_cond_var_.notify_one();
}

template<typename T>
T BlockingQueue<T>::take()
{
    std::unique_lock<std::mutex> lock(mu_);
    while (q_.empty()) {
        take_cond_var_.wait(lock);
    }

    T t = std::move(q_.front());
    q_.pop();
    push_cond_var_.notify_one();
    return t;
}

} // namespace base

#endif // BASE_BLOCKING_QUEUE_H_
