#include "events_processor.h"

#include <iostream>
#include <exception>

#include <nlohmann/json.hpp>


namespace
{

std::string CreateSuspiciousActivityAlert(unsigned pid,
                const std::vector<unsigned>& child_pids)
{
    using json = nlohmann::json;
    json jobject{ {"type", "SuspiciousActivityAlert"}, 
            {"pid", pid}, {"child_pids", child_pids} };
    return jobject.dump();
}

}


EventsProcessor::EventsProcessor(BoundedQueue<std::string>& queue)
    : _queue(queue)
{
    if (!_tcp_client.Connect("127.0.0.1", 5000)) {
        throw std::runtime_error("Instance of EventsProcessor couldn't connect to events' monitoring service");
    }

    for (size_t i = 0; i < NWORKERS; i++) {
        _workers[i] = std::move(std::thread(&EventsProcessor::Worker, this));
    }    

    _send_thread = std::move(std::thread(&EventsProcessor::SendWorker, this));
}

EventsProcessor::~EventsProcessor()
{
    Stop();
    try {
        for (size_t i = 0; i < NWORKERS; i++) {
            _workers[i].join();
        }

        _stop_send = true;
        _send_queue_cv.notify_one();        

        _send_thread.join();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsProcessor::~EventsProcessor() - "
            << ex.what() << std::endl;
    }

    std::cout << "EventsProcessor d-tor, queue size=" << _queue.Size() << std::endl;
}

void EventsProcessor::Stop() noexcept
{
    _need_stop = true;
}

void EventsProcessor::Worker()
{
    try {
        while (true) {
            std::string item;
            if (_queue.GetWithTimeout(item, 1)) {
                ProcessEvent(std::move(item));
            }
            
            if (_need_stop) {
                break;
            }
        }

        while (_queue.Size() != 0) {
            std::string item;
            _queue.Get(item);
            ProcessEvent(std::move(item));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsProcessor::worker() - "
            << ex.what() << std::endl;
    }
}

void EventsProcessor::ProcessEvent(std::string&& event)
{
    if (event.empty()) {
        return;
    }

    try {
        nlohmann::json doc(nlohmann::json::parse(event));

        const std::string type{doc.at("type")};
        if (type == "ProcessStarted") {

            const unsigned ppid = doc.at("ppid");
            const unsigned pid = doc.at("pid");
            const unsigned ts = doc.at("ts");

            const auto child_pids{_detect_rule.CheckProcess(ppid, pid, ts)};
            if (!child_pids.empty()) {
                std::string alert{CreateSuspiciousActivityAlert(ppid, child_pids)};
                ScheduleEventToSend(std::move(alert));
            }
        } else {
            ScheduleEventToSend(std::move(event));
        }

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Exception when parse event - "
            << e.what() << std::endl;
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Exception when access JSON item(s) - "
            << e.what() << std::endl;
    }
}

void EventsProcessor::SendWorker()
{
    try {
        while (true) {
            std::string event;
            {
                std::unique_lock<std::mutex> lock(_send_queue_mx);
                       while (_events_to_send.empty() && !_stop_send) {
                    _send_queue_cv.wait(lock);
                }

                if (!_events_to_send.empty()) {
                    event = std::move(_events_to_send.front());
                    _events_to_send.pop_front();
                }
            }

            if (!event.empty()) {
                event.push_back('\n');  // EOL is messages' delimiter, expected by reciver
                if (!_tcp_client.Send(event)) {
                    std::cerr << "Instance of EventsProcessor couldn't send event to monitoring service" << std::endl;
                }
            } else {
                if (_stop_send) {
                    break;
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsProcessor::SendWorker() - "
            << ex.what() << std::endl;
    }
}

void EventsProcessor::ScheduleEventToSend(std::string&& event)
{
    std::lock_guard<std::mutex> lock(_send_queue_mx);
    _events_to_send.emplace_back(event);
    _send_queue_cv.notify_one();
}
