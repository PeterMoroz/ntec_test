#pragma once

#include <atomic>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>


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
    void Worker();
    void ProcessEvent(std::string&& event);

    void SendWorker();
    void ScheduleEventToSend(std::string&& event);

private:
    BoundedQueue<std::string>& _queue;
    std::thread _worker;
    std::atomic<bool> _need_stop{false};
    CheckTooFrequentSpawn _detect_rule;
    TCPClient _tcp_client;

    std::thread _send_thread;
    std::atomic<bool> _stop_send{false};
    std::deque<std::string> _events_to_send;
    std::mutex _send_queue_mx;
    std::condition_variable _send_queue_cv;
};