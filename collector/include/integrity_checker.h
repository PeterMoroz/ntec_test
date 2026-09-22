#pragma once

#include <atomic>
#include <filesystem>
#include <thread>
#include <string>
#include <unordered_map>


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
    void scan_worker();

private:
    const std::filesystem::path _path_to_watched_dir;
    std::thread _scan_thread;
    std::atomic<bool> _need_stop{false};
    std::unordered_map<std::filesystem::path, std::string> _files_hashes;
};
