#include "events_processor.h"

#include <iostream>
#include <exception>


EventsProcessor::EventsProcessor(BoundedQueue<std::string>& queue)
    : _queue(queue)
    , _worker(&EventsProcessor::worker, this)
{
}

EventsProcessor::~EventsProcessor()
{
    Stop();
    try {
        _worker.join();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsProcessor::~EventsProcessor() - "
            << ex.what() << std::endl;
    }
}

void EventsProcessor::Stop() noexcept
{
    _need_stop = true;
}

#include <sstream>
#include <fstream>

void EventsProcessor::worker()
{
    std::ostringstream oss;
    oss << "worker_" << std::this_thread::get_id() << ".log";
    std::ofstream log(oss.str());
    unsigned event_count = 0;

    try {

        while (true) {
            std::string item;
            _queue.Get(item);
            if (!item.empty()) {
                event_count++;
                log << item << std::endl;
            }

            if (event_count % 1000 == 0) {
                std::cout << "processed " << event_count << " events " << std::endl;
            }
            
            if (_need_stop) {
                break;
            }
        }

        while (_queue.Size() != 0) {
            std::string item;
            _queue.Get(item);
            log << item << std::endl;
        }

    } catch (const std::exception& ex) {
        std::cerr << "Exception in EventsProcessor::worker() - "
            << ex.what() << std::endl;
    }
}
