#pragma once

#include <atomic>
#include <thread>

#include "bounded_queue.h"
#include "check_too_frequent_spawn.h"
#include "tcp_client.h"

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
    void ProcessEvent(std::string&& event);

private:
    BoundedQueue<std::string>& _queue;
    std::thread _worker;
    std::atomic<bool> _need_stop{false};
    CheckTooFrequentSpawn _detect_rule;
    TCPClient _tcp_client;
};