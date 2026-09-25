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
    std::lock_guard lock(_mx);
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
    /* use a kind of double check locking pattern:
     1. Acquire a "lightweight" shared lock, check that object 
     which tracks activity of process with given PID exist 
     and test suspicious activity (with call SlidingWindow::InsertItem ).
     2. In case when there is no object yet, acquire unique lock
     and check the presense of object again (because it might be
     inserted by other thread after shared lock is released,
     but unique lock is not yet acquired): 
     - if object not exist - create a new one
     - otherwise - check suspicious activity with method SlidingWindow::InsertItem
    */
    {
        std::shared_lock lock(_mx);
        auto it = _spawn_stats.find(pid);
        if (it != _spawn_stats.cend()) {
            return it->second->InsertItem(child_pid, time_start);
        }
    }

    std::unique_lock lock(_mx);
    auto it = _spawn_stats.find(pid);
    if (it == _spawn_stats.cend()) {
        auto sliding_window = std::make_unique<SlidingWindow>(child_pid, time_start);
        _spawn_stats[pid] = std::move(sliding_window);
        return {};
    }

    return it->second->InsertItem(child_pid, time_start);
}
