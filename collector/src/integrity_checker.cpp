#include "integrity_checker.h"

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>

#include <nlohmann/json.hpp>

IntegrityChecker::IntegrityChecker(
        const std::filesystem::path& path_to_watched_dir,
        const std::filesystem::path& path_to_baseline)
    : _path_to_watched_dir(_path_to_watched_dir)
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
            _files_hashes.emplace(watched_path, value);
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

    _scan_thread = std::move(std::thread(&IntegrityChecker::scan_worker, this));
}

IntegrityChecker::~IntegrityChecker()
{
    Stop();
    try {
        _scan_thread.join();
    } catch (const std::exception& ex) {
        std::cerr << "Exception in IntegrityChecker::~IntegrityChecker() - "
            << ex.what() << std::endl;
    }
}

void IntegrityChecker::Stop() noexcept
{
    _need_stop = true;
}

void IntegrityChecker::scan_worker()
{
    try {
        while (true) {
            // TO DO: read directory content
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception in IntegrityChecker::scan_worker() - "
            << ex.what() << std::endl;
    }
}
