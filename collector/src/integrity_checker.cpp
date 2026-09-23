#include "integrity_checker.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <chrono>

#include <ctime>

#include <nlohmann/json.hpp>
#include "sha256.h"


namespace
{

std::string CreateEvent(const std::string& kind, 
    const std::filesystem::path& path, std::time_t ts)
{
    using json = nlohmann::json;
    json jobject{ {"type", kind}, {"path", path.c_str()}, {"ts", ts} };
    return jobject.dump();
}

}

IntegrityChecker::IntegrityChecker(
        const std::filesystem::path& path_to_watched_dir,
        const std::filesystem::path& path_to_baseline)
    : _path_to_watched_dir(path_to_watched_dir)
{
    if (!std::filesystem::exists(path_to_watched_dir)) {
        throw std::logic_error("the directory to watch not exist");
    }

    if (!std::filesystem::exists(path_to_baseline)) {
        throw std::logic_error("baseline file not exist");
    }

    std::ifstream ifs(path_to_baseline);
    if (!ifs.is_open()) {
        throw std::runtime_error("could not open baseline file");
    }

    try {
        nlohmann::json doc(nlohmann::json::parse(ifs));
        for (auto& [key, value] : doc.items()) {
            // std::cout << "key: " << key << ", value: " << value << std::endl;
            std::filesystem::path watched_path(path_to_watched_dir);
            watched_path /= std::filesystem::path(key);
            // std::cout << "watched path: " << watched_path << std::endl;
            _baseline_files.insert(watched_path);
            _files_hashes.emplace(watched_path, value);
            // std::cout << std::endl;
        }
    } catch (const nlohmann::json::parse_error& ex) {
        std::cerr << "Exception when parse baseline file - "
            << ex.what() << std::endl;
        throw std::runtime_error("could not parse baseline file");
    }

    // std::cout << "files hashes: " << std::endl;
    // for (const auto& [key, value] : _files_hashes) {
    //     std::cout << "path: " << key << "hash: " << value << std::endl;
    // }

    // for (const auto& [key, value] : _files_hashes) {
    //     try {
    //         if (!std::filesystem::exists(key)) {
    //             std::cout << "file " << key << "\tNOT exist" << std::endl;
    //         } else {
    //             std::ifstream ifs(key, std::ios::binary);
    //             if (!ifs.is_open()) {
    //                 std::cerr << "couldn't open file: " << key << std::endl;
    //                 continue;
    //             }
    //             const size_t fsize = std::filesystem::file_size(key);
    //             std::vector<unsigned char> filedata;
    //             filedata.reserve(fsize);
    //             std::copy(std::istream_iterator<unsigned char>(ifs),
    //                     std::istream_iterator<unsigned char>(),
    //                     std::back_inserter(filedata));

    //             SHA256 sha256;
    //             const std::string filehash(sha256(filedata.data(), fsize));
    //             std::cout << "filepath: " << key;
    //             if (filehash == value) {
    //                 std::cout << " - hash OK" << std::endl;
    //             } else {
    //                 std::cout << " - hash mismatch"
    //                     << "\nexpected: " << value 
    //                     << "\tactual: " << filehash << std::endl;
    //             }
    //         }
    //     } catch (const std::exception& ex) {
    //         std::cerr << "Exception when check file's hash: "
    //             << ex.what() << std::endl;
    //     }
    // }

    // std::set<std::filesystem::path> files;
    // std::cout << "scan observed directory: " << std::endl;
    // for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_watched_dir)) {
    //     if (entry.is_regular_file()) {
    //         files.insert(entry.path());
    //     }
    // }

    // std::set<std::filesystem::path> files_diff;
    // std::set_difference(files.cbegin(), files.cend(), 
    //         _baseline_files.cbegin(), _baseline_files.cend(),
    //         std::inserter(files_diff, files_diff.begin()));
    
    // std::cout << "find differences: \n";
    // for (const auto& entry :files_diff) {
    //     std::cout << entry << std::endl;
    // }

    if (!_tcp_client.Connect("127.0.0.1", 5000)) {
        throw std::runtime_error("couldn't connect to events' monitoring service");
    }

    _scan_thread = std::move(std::thread(&IntegrityChecker::ScanWorker, this));
    _send_thread = std::move(std::thread(&IntegrityChecker::SendWorker, this));
}

IntegrityChecker::~IntegrityChecker()
{
    Stop();
    try {
        _scan_thread.join();
        _send_thread.join();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in IntegrityChecker::~IntegrityChecker() - "
            << ex.what() << std::endl;
    }

    std::cout << "IntegrityChecker d-tor: the size of events queue = "
        << _events_to_send.size() << std::endl;
}

void IntegrityChecker::Stop() noexcept
{
    _stop_scan = true;
}

void IntegrityChecker::ScanWorker()
{
    try {
        while (!_stop_scan) {
            CheckExpectedFiles();
            CheckUnexpectedFiles();
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in IntegrityChecker::ScanWorker() - "
            << ex.what() << std::endl;
    }

    std::cout << "ScanWorker finished " << std::endl;

    // enforce threads stopping order to avoid missing events left in queue
    _stop_send = true;
    _cv_events_queue.notify_one();    
}

void IntegrityChecker::SendWorker()
{
    try {
        while (true) {
            std::string event;
            {
                std::unique_lock<std::mutex> lock(_mx_events_queue);
                       while (_events_to_send.empty() && !_stop_send) {
                    _cv_events_queue.wait(lock);
                }

                if (!_events_to_send.empty()) {
                    event = std::move(_events_to_send.front());
                    _events_to_send.pop_front();
                }
            }

            if (!event.empty()) {
                event.push_back('\n');  // EOL is messages' delimiter, expected by reciver
                if (!_tcp_client.Send(event)) {
                    std::cerr << "couldn't send event to monitoring service" << std::endl;
                }
            } else {
                if (_stop_send) {
                    break;
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in IntegrityChecker::SendWorker() - "
            << ex.what() << std::endl;
    }
    std::cout << "SendWorker finished " << std::endl;
}

void IntegrityChecker::ScheduleEventToSend(std::string&& event)
{
    std::lock_guard<std::mutex> lock(_mx_events_queue);
    _events_to_send.emplace_back(event);
    _cv_events_queue.notify_one();
}

void IntegrityChecker::CheckExpectedFiles()
{
    for (const auto& [key, value] : _files_hashes) {
        try {
            if (!std::filesystem::exists(key)) {
                const std::time_t ts = std::time(NULL);
                ScheduleEventToSend(CreateEvent("FileRemoved", key, ts));
            } else {
                std::ifstream ifs(key, std::ios::binary);
                if (!ifs.is_open()) {
                    std::cerr << "couldn't open file: " << key << std::endl;
                    continue;
                }
                const size_t fsize = std::filesystem::file_size(key);
                std::vector<unsigned char> filedata;
                filedata.reserve(fsize);
                std::copy(std::istream_iterator<unsigned char>(ifs),
                        std::istream_iterator<unsigned char>(),
                        std::back_inserter(filedata));

                SHA256 sha256;
                const std::string filehash(sha256(filedata.data(), fsize));
                if (filehash != value) {
                    const std::time_t ts = std::time(NULL);
                    ScheduleEventToSend(CreateEvent("FileModified", key, ts));
                }
            }
        } catch (const std::exception& ex) {
            std::cerr << "Exception when check file's hash: "
                << ex.what() << std::endl;
        }
    }
}

void IntegrityChecker::CheckUnexpectedFiles()
{
    /* the method scan the observed directory and 
      calculate difference of sets <found files> / <expected files>,
      for each item in result set (diff) generate event for sending.

      the percularity of current implementation is that for any file 
      added (unexpected file) a new event will be generated and sent
      every time when the method is called even if such event was 
      already sent (differed only by timestamp).

      considering that the method invoked continuosly by scan thread 
      such behaviour may be wrong, therefore might be needed to keep
      the info (path) about all files found during previous scans and 
      not generate events for them again.
    */

    std::set<std::filesystem::path> files;
    for (const auto& entry : 
            std::filesystem::recursive_directory_iterator(_path_to_watched_dir)) {
        if (entry.is_regular_file()) {
            files.insert(entry.path());
        }
    }

    std::set<std::filesystem::path> files_diff;
    std::set_difference(files.cbegin(), files.cend(), 
            _baseline_files.cbegin(), _baseline_files.cend(),
            std::inserter(files_diff, files_diff.begin()));
    
    for (const auto& entry : files_diff) {
        const std::time_t ts = std::time(NULL);
        ScheduleEventToSend(CreateEvent("FileAdded", entry, ts));
    }
}

