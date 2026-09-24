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
    };

    const size_t CHECKED_TIMEFRAME_SEC = 10;
    const size_t MAX_ALLOWED_PER_FRAME = 5;

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
    std::unordered_map<unsigned, std::vector<ChildInfo>> _spawn_stats;
};