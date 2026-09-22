#pragma once

#include <atomic>
#include <thread>

#include "bounded_queue.h"

class EventsProcessor final
{
    EventsProcessor(const EventsProcessor&) = delete;
    EventsProcessor& operator=(const EventsProcessor&) = delete;


public:
    explicit EventsProcessor(BoundedQueue<std::string>& queue);
    ~EventsProcessor();

    void Stop() noexcept;

private:
    void worker();

private:
    BoundedQueue<std::string>& _queue;
    std::thread _worker;
    std::atomic<bool> _need_stop{false};
};