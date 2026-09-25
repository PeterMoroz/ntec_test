#include "check_too_frequent_spawn.h"

#include <algorithm>


namespace
{

const size_t CHECKED_TIMEFRAME_SEC = 10;
const size_t MAX_ALLOWED_PER_FRAME = 5;

}

std::vector<unsigned> 
CheckTooFrequentSpawn::SlidingWindow::InsertItem(unsigned pid, unsigned ts)
{
    _wnd.push_back({pid, ts});
    // sort by timestamp
    std::sort(_wnd.begin(), _wnd.end(), 
        [](const ChildInfo& lhs, const ChildInfo& rhs)
        { return lhs.ts < rhs.ts; });

    std::vector<unsigned> pids;
    if (_wnd.size() > MAX_ALLOWED_PER_FRAME) {
        const auto last_ts = _wnd.back().ts;
        const auto first_ts = _wnd.front().ts;
        if ((last_ts - first_ts) <= CHECKED_TIMEFRAME_SEC) {
            // create list of children pids, including new comer
            pids.resize(_wnd.size());
            for (size_t i = 0; i < _wnd.size(); i++) {
                pids[i] = _wnd[i].pid;
            }
        }

        // drop the earliest element(s)
        while (_wnd.size() > MAX_ALLOWED_PER_FRAME) {
            _wnd.erase(_wnd.begin(), _wnd.begin() + 1);
        }
    }

    return pids;
}

std::vector<unsigned> CheckTooFrequentSpawn::CheckProcess(
        unsigned pid, unsigned child_pid, unsigned time_start)
{
    auto it = _spawn_stats.find(pid);
    if (it == _spawn_stats.cend()) {
        _spawn_stats.emplace(pid, SlidingWindow{child_pid, time_start});
        return {};
    }
    return it->second.InsertItem(child_pid, time_start);
}
