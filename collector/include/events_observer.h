#pragma once

#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <array>
#include <atomic>

#include "bounded_queue.h"

class EventsObserver final
{
    EventsObserver(const EventsObserver&) = delete;
    EventsObserver& operator=(const EventsObserver&) = delete;

    static const size_t NREADERS = 2;

public:
    EventsObserver(const std::filesystem::path& path_to_events_source, 
                    BoundedQueue<std::string>& queue);
    ~EventsObserver();

    void Stop() noexcept;

private:
    void reader();

private:
    std::ifstream _ifs;
    BoundedQueue<std::string>& _queue;

    std::mutex _m;
    std::array<std::thread, NREADERS> _readers;
    std::atomic<bool> _need_stop{false};
};