#include <iostream>
#include <exception>
#include <filesystem>
#include <thread>

#include <csignal>
#include <cstring>

#include "bounded_queue.h"
#include "events_observer.h"
#include "events_processor.h"
#include "integrity_checker.h"

// perhaps should be atomic,
// but for simplicity use plain bool
bool running = true;

void sighandler(int s)
{
    if (s == SIGINT || s == SIGTERM) {
        running = false;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "usage: " << argv[0] 
            << " <path-to-events-log>"
            << " <path-to-watched-dir>"
            << " <path-to-baseline-file>" << std::endl;
        return -1;
    }

    // if (argc != 2) {
    //     std::cerr << "usage: " << argv[0] 
    //         << "<path-to-events-log> " << std::endl;
    //     return -1;
    // }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    try {
        static const size_t MAX_QUEUE_LENGTH = 256;
        // BoundedQueue<std::string> queue(MAX_QUEUE_LENGTH);
        // EventsObserver events_observer(std::filesystem::path{argv[1]}, queue);
        IntegrityChecker integrity_checker(std::filesystem::path{argv[2]}, 
                                            std::filesystem::path{argv[3]});

        // EventsProcessor events_processor(queue);

        while (running) {
            std::this_thread::yield();
        }
    } catch (const std::exception& ex) {
        std::cerr << "exception: " << ex.what() << std::endl;
    }
    
    return 0;
}