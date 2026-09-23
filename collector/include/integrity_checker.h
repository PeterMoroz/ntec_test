#pragma once

#include <atomic>
#include <filesystem>
#include <thread>
#include <string>
#include <unordered_map>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <set>

#include "tcp_client.h"

class IntegrityChecker final
{
    IntegrityChecker(const IntegrityChecker&) = delete;
    IntegrityChecker& operator=(const IntegrityChecker&) = delete;

public:
    IntegrityChecker(
        const std::filesystem::path& path_to_watched_dir,
        const std::filesystem::path& path_to_baseline);
    ~IntegrityChecker();

    void Stop() noexcept;

private:
    void ScanWorker();
    void SendWorker();

    void ScheduleEventToSend(std::string&& event);

    void CheckExpectedFiles();
    void CheckUnexpectedFiles();

private:
    const std::filesystem::path _path_to_watched_dir;
    std::thread _scan_thread;
    std::thread _send_thread;
    std::atomic<bool> _stop_scan{false};
    std::atomic<bool> _stop_send{false};
    std::unordered_map<std::filesystem::path, std::string> _files_hashes;
    std::set<std::filesystem::path> _baseline_files;
    std::deque<std::string> _events_to_send;
    std::mutex _mx_events_queue;
    std::condition_variable _cv_events_queue;

    TCPClient _tcp_client;
};
