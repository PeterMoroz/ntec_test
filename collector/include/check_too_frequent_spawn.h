#pragma once

#include <vector>
#include <unordered_map>

class CheckTooFrequentSpawn final
{
    CheckTooFrequentSpawn(const CheckTooFrequentSpawn&) = delete;
    CheckTooFrequentSpawn& operator=(const CheckTooFrequentSpawn&) = delete;

    struct ChildInfo
    {
        unsigned pid{0};
        unsigned ts{0};

        ChildInfo() = default;
        ChildInfo(unsigned p, unsigned t) : pid(p), ts(t) {}
    };

    struct SlidingWindow
    {
        SlidingWindow() = default;
        SlidingWindow(unsigned pid, unsigned ts) {
            _wnd.emplace_back(pid, ts);
        }

        std::vector<unsigned> InsertItem(unsigned pid, unsigned ts);

        std::vector<ChildInfo> _wnd;
    };

public:
    CheckTooFrequentSpawn() = default;
    ~CheckTooFrequentSpawn() = default;

    /* The method checks if behavior of process with given PID is suspicious 
      by comparing info of new spawned child against parent's statistics
      If so, the list of children PID's will be returned or empty list otherwise.
     */
    std::vector<unsigned> CheckProcess(unsigned pid, 
            unsigned child_pid, unsigned time_start);

private:
    // spawn statistics: map parent pid to set children descriptions
    std::unordered_map<unsigned, SlidingWindow> _spawn_stats;
};