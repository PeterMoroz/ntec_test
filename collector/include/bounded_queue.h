#pragma once

#include <deque>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>


template <typename T>
class BoundedQueue final
{
    BoundedQueue(const BoundedQueue&) = delete;
    BoundedQueue& operator=(const BoundedQueue&) = delete;

public:
    explicit BoundedQueue(size_t max_length) 
        : _max_length(max_length) {}
    ~BoundedQueue() = default;

    void Put(T&& item)
    {
        {
            std::unique_lock<std::mutex> lock(_m);
            _not_full.wait(lock, [this]() { return _q.size() < _max_length; });
            _q.emplace_back(item);
        }
        _not_empty.notify_all();
    }
    
    void Get(T& item)
    {
        {
            std::unique_lock<std::mutex> lock(_m);
            _not_empty.wait(lock, [this]() { return !_q.empty(); });
            item = std::move(_q.front());
            _q.pop_front();
        }
        _not_full.notify_all();
    } 

    bool GetWithTimeout(T& item, unsigned wait_sec)
    {
        {
            std::unique_lock<std::mutex> lock(_m);
            if (_not_empty.wait_for(lock, std::chrono::seconds(wait_sec),
                                    [this]() { return !_q.empty(); })) {
                item = std::move(_q.front());
                _q.pop_front();
            } else {
                return false;
            }
        }
        _not_full.notify_all();
        return true;
    }    

    size_t Size()
    {
        std::lock_guard<std::mutex> lock(_m);
        return _q.size();
    }


private:
    size_t _max_length;
    std::deque<T> _q;
    std::mutex _m;
    std::condition_variable _not_empty;
    std::condition_variable _not_full;
};
