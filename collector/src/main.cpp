#include <iostream>
#include <exception>
#include <filesystem>

#include "bounded_queue.h"
#include "events_observer.h"
#include "events_processor.h"
#include "integrity_checker.h"

int main(int argc, char* argv[])
{
    // if (argc != 4) {
    //     std::cerr << "usage: " << argv[0] 
    //         << "<path-to-events-log> "
    //         << "<path-to-watched-dir> "
    //         << "<path-to-baseline-file>" << std::endl;
    //     return -1;
    // }

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] 
            << "<path-to-events-log> " << std::endl;
        return -1;
    }


    try {
        static const size_t MAX_QUEUE_LENGTH = 256;
        BoundedQueue<std::string> queue(MAX_QUEUE_LENGTH);
        EventsObserver events_observer(std::filesystem::path(argv[1]), queue);
        // IntegrityChecker integrity_checker(std::filesystem::path(argv[2]), 
        //                                     std::filesystem::path(argv[3]));

        EventsProcessor events_processor(queue);

        while (1) {
            ;
        }
    } catch (const std::exception& ex) {
        std::cerr << "exception: " << ex.what() << std::endl;
    }
    return 0;
}