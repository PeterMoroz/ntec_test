#include "check_too_frequent_spawn.h"

#include <algorithm>

std::vector<unsigned> CheckTooFrequentSpawn::CheckProcess(
        unsigned pid, unsigned child_pid, unsigned time_start)
{
    auto it = _spawn_stats.find(pid);
    if (it == _spawn_stats.cend()) {
        _spawn_stats[pid].push_back({child_pid, time_start});
        return {};
    }

    auto& infos = it->second;
    infos.push_back({child_pid, time_start});
    // sort by timestamp
    std::sort(infos.begin(), infos.end(), 
        [](const ChildInfo& lhs, const ChildInfo& rhs)
        { return lhs.ts < rhs.ts; });

    std::vector<unsigned> pids;
    if (infos.size() > MAX_ALLOWED_PER_FRAME) {
        const auto last_ts = infos[infos.size() - 1].ts;
        const auto first_ts = infos[0].ts;
        if ((last_ts - first_ts) <= CHECKED_TIMEFRAME_SEC) {
            // create list of children pids, including new comer
            pids.resize(infos.size());
            for (size_t i = 0; i < infos.size(); i++) {
                pids[i] = infos[i].pid;
            }
        }

        // drop the earliest element(s)
        while (infos.size() > MAX_ALLOWED_PER_FRAME) {
            infos.erase(infos.begin(), infos.begin() + 1);
        }
    }

    return pids;
}
