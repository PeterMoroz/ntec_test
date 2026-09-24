#include "events_observer.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <random>
#include <chrono>


EventsObserver::EventsObserver(
        const std::filesystem::path& path_to_events_source, 
        BoundedQueue<std::string>& queue)
    : _ifs(path_to_events_source)
    , _queue(queue)
{
    if (!std::filesystem::exists(path_to_events_source)) {
        throw std::logic_error("path to events source (events log) not exist");
    }

    if (!_ifs.is_open()) {
        throw std::runtime_error("could not open events log");
    }

    for (size_t i = 0; i < NREADERS; i++) {
        _readers[i] = std::move(std::thread(&EventsObserver::reader, this));
    }
}

EventsObserver::~EventsObserver()
{
    Stop();
    try {
        for (size_t i = 0; i < NREADERS; i++) {
            _readers[i].join();
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsObserver::~EventsObserver() - " 
            << ex.what() << std::endl;
    }

    std::cout << "EventsObserver d-tor, queue size=" << _queue.Size() << std::endl;
}

void EventsObserver::Stop() noexcept
{
    _need_stop = true;
}


void EventsObserver::reader()
{
    unsigned line_count = 0;
    std::mt19937_64 eng(std::random_device{}());
    std::uniform_int_distribution<> dist(1, 10);

    try {
        while (true) {
            std::string line;

            {
                std::lock_guard<std::mutex> lg(_m);
                std::getline(_ifs, line);
            }

            if (!line.empty())
                line_count++;            

            if (line_count % 1000 == 0) {
                std::cout << "Read " << line_count << " lines " << std::endl;
            }

            if (!line.empty()) {
                _queue.Put(std::move(line));
            }

            if (_need_stop || _ifs.eof()) {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(dist(eng)));            
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsObserver::reader() - " << ex.what() << std::endl;
    }

    std::cout << "thread id=" << std::this_thread::get_id() 
        << " read " << line_count << " lines " << std::endl;
}
