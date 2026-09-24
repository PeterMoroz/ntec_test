#include <gtest/gtest.h>

#include "check_too_frequent_spawn.h"

#include <array>
#include <random>
#include <set>
#include <vector>
#include <algorithm>

TEST(TestRule, ProcessSpawnNoMoreThan5ChildForAllTime)
{
    /*
      we have 100 processes, 
      each spawns up to 5 children with time interval in range 11 - 300 seconds
    */ 

    static const size_t NPARENTS = 100;

    std::array<unsigned, NPARENTS> parent_pids;
    std::set<unsigned> used_pids;
    std::array<std::vector<std::pair<unsigned, unsigned>>, NPARENTS> children;

    std::random_device rd;
    std::mt19937 rng(rd());
    using uniform_int_distribution = std::uniform_int_distribution<std::mt19937::result_type>;
    uniform_int_distribution dist_pids(1000, 10000);
    uniform_int_distribution dist_start_time(1790125200, 1790168400);

    uniform_int_distribution dist_nchilds(1, 5);
    // each process can start in the interval [30, 3000] seconds after start time
    uniform_int_distribution dist_proc_create_interval(30, 3000);
    // the child can be spawned in the interval [11, 300] seconds after parent started
    uniform_int_distribution dist_child_spawn_interval(11, 300);

    auto generate_pid = [&used_pids, &dist_pids, &rng]() -> unsigned
    {
        while (1) {
            unsigned pid = dist_pids(rng);
            if (used_pids.count(pid) == 0) {
                used_pids.insert(pid);
                return pid;
            }
        }
    };

    CheckTooFrequentSpawn rule;

    unsigned start_time_global = dist_start_time(rng);
    for (size_t i = 0; i < NPARENTS; i++) {
        parent_pids[i] = generate_pid();
        unsigned start_time = start_time_global + dist_proc_create_interval(rng);
        unsigned nchilds = dist_nchilds(rng);
        
        for (size_t j = 0; j < nchilds; j++) {
            unsigned pid = generate_pid();            
            unsigned ts = start_time + dist_child_spawn_interval(rng);
            children[i].push_back(std::make_pair(pid, ts));
        }
    }

    for (size_t i = 0; i < NPARENTS; i++) {
        const unsigned ppid = parent_pids[i];
        const auto& child_entries = children[i];
        for (size_t j = 0; j < child_entries.size(); j++) {
            const auto [pid, ts] = child_entries[j];
            const auto child_pids{rule.CheckProcess(ppid, pid, ts)};
            ASSERT_TRUE(child_pids.empty());
        }
    }
}

TEST(TestRule, ProcessSpawnMoreThan5ChildButNotIn10secondsFrame)
{
    /*
      we have 100 processes, 
      each spawns up to 15 children with time interval in range 11 - 300 seconds
    */ 

    static const size_t NPARENTS = 100;

    std::array<unsigned, NPARENTS> parent_pids;
    std::set<unsigned> used_pids;
    std::array<std::vector<std::pair<unsigned, unsigned>>, NPARENTS> children;

    std::random_device rd;
    std::mt19937 rng(rd());
    using uniform_int_distribution = std::uniform_int_distribution<std::mt19937::result_type>;
    uniform_int_distribution dist_pids(1000, 10000);
    uniform_int_distribution dist_start_time(1790125200, 1790168400);

    uniform_int_distribution dist_nchilds(1, 15);
    // each process can start in the interval [30, 3000] seconds after start time
    uniform_int_distribution dist_proc_create_interval(30, 3000);
    // the child can be spawned in the interval [11, 300] seconds after parent started
    uniform_int_distribution dist_child_spawn_interval(11, 300);

    auto generate_pid = [&used_pids, &dist_pids, &rng]() -> unsigned
    {
        while (1) {
            unsigned pid = dist_pids(rng);
            if (used_pids.count(pid) == 0) {
                used_pids.insert(pid);
                return pid;
            }
        }
    };

    CheckTooFrequentSpawn rule;

    unsigned start_time_global = dist_start_time(rng);
    for (size_t i = 0; i < NPARENTS; i++) {
        parent_pids[i] = generate_pid();
        unsigned start_time = start_time_global + dist_proc_create_interval(rng);
        unsigned nchilds = dist_nchilds(rng);
        
        for (size_t j = 0; j < nchilds; j++) {
            unsigned pid = generate_pid();            
            unsigned ts = start_time + dist_child_spawn_interval(rng);
            children[i].push_back(std::make_pair(pid, ts));
        }
    }

    for (size_t i = 0; i < NPARENTS; i++) {
        const unsigned ppid = parent_pids[i];
        const auto& child_entries = children[i];
        for (size_t j = 0; j < child_entries.size(); j++) {
            const auto [pid, ts] = child_entries[j];
            const auto child_pids{rule.CheckProcess(ppid, pid, ts)};
            ASSERT_TRUE(child_pids.empty());
        }
    }
}

TEST(TestRule, ProcessSpawnNoMoreThan5ChildIn10secondsFrame)
{
    /*
      we have 100 processes, 
      each spawns up to 5 children with time interval in range 1 - 10 seconds
    */ 

    static const size_t NPARENTS = 100;

    std::array<unsigned, NPARENTS> parent_pids;
    std::set<unsigned> used_pids;
    std::array<std::vector<std::pair<unsigned, unsigned>>, NPARENTS> children;

    std::random_device rd;
    std::mt19937 rng(rd());
    using uniform_int_distribution = std::uniform_int_distribution<std::mt19937::result_type>;
    uniform_int_distribution dist_pids(1000, 10000);
    uniform_int_distribution dist_start_time(1790125200, 1790168400);

    uniform_int_distribution dist_nchilds(1, 5);
    // each process can start in the interval [30, 3000] seconds after start time
    uniform_int_distribution dist_proc_create_interval(30, 3000);
    // the child can be spawned in the interval [1, 10] seconds after parent started
    uniform_int_distribution dist_child_spawn_interval(1, 10);    

    auto generate_pid = [&used_pids, &dist_pids, &rng]() -> unsigned
    {
        while (1) {
            unsigned pid = dist_pids(rng);
            if (used_pids.count(pid) == 0) {
                used_pids.insert(pid);
                return pid;
            }
        }
    };

    CheckTooFrequentSpawn rule;

    unsigned start_time_global = dist_start_time(rng);
    for (size_t i = 0; i < NPARENTS; i++) {
        parent_pids[i] = generate_pid();
        unsigned start_time = start_time_global + dist_proc_create_interval(rng);
        unsigned nchilds = dist_nchilds(rng);
        
        for (size_t j = 0; j < nchilds; j++) {
            unsigned pid = generate_pid();            
            unsigned ts = start_time + dist_child_spawn_interval(rng);
            children[i].push_back(std::make_pair(pid, ts));
        }
    }

    for (size_t i = 0; i < NPARENTS; i++) {
        const unsigned ppid = parent_pids[i];
        const auto& child_entries = children[i];
        for (size_t j = 0; j < child_entries.size(); j++) {
            const auto [pid, ts] = child_entries[j];
            const auto child_pids{rule.CheckProcess(ppid, pid, ts)};
            ASSERT_TRUE(child_pids.empty());
        }
    }
}

TEST(TestRule, ProcessSpawnMoreThan5ChildIn10secondsFrame)
{
    /*
      we have 100 processes, 
      each spawns up to 15 children with time interval in range 1 - 10 seconds
    */ 

    static const size_t NPARENTS = 100;

    std::array<unsigned, NPARENTS> parent_pids;
    std::set<unsigned> used_pids;
    std::array<std::vector<std::pair<unsigned, unsigned>>, NPARENTS> children;

    std::random_device rd;
    std::mt19937 rng(rd());
    using uniform_int_distribution = std::uniform_int_distribution<std::mt19937::result_type>;
    uniform_int_distribution dist_pids(1000, 10000);
    uniform_int_distribution dist_start_time(1790125200, 1790168400);

    uniform_int_distribution dist_nchilds(1, 15);
    // each process can start in the interval [30, 3000] seconds after start time
    uniform_int_distribution dist_proc_create_interval(30, 3000);
    // the child can be spawned in the interval [1, 10] seconds after parent started
    uniform_int_distribution dist_child_spawn_interval(1, 10);    

    auto generate_pid = [&used_pids, &dist_pids, &rng]() -> unsigned
    {
        while (1) {
            unsigned pid = dist_pids(rng);
            if (used_pids.count(pid) == 0) {
                used_pids.insert(pid);
                return pid;
            }
        }
    };

    CheckTooFrequentSpawn rule;

    unsigned start_time_global = dist_start_time(rng);
    for (size_t i = 0; i < NPARENTS; i++) {
        parent_pids[i] = generate_pid();
        unsigned start_time = start_time_global + dist_proc_create_interval(rng);
        unsigned nchilds = dist_nchilds(rng);
        
        for (size_t j = 0; j < nchilds; j++) {
            unsigned pid = generate_pid();            
            unsigned ts = start_time + dist_child_spawn_interval(rng);
            children[i].push_back(std::make_pair(pid, ts));
        }
    }

    for (size_t i = 0; i < NPARENTS; i++) {
        const unsigned ppid = parent_pids[i];
        const auto& child_entries = children[i];
        if (child_entries.size() <= 5) {
            for (size_t j = 0; j < child_entries.size(); j++) {
                const auto [pid, ts] = child_entries[j];
                const auto child_pids{rule.CheckProcess(ppid, pid, ts)};
                ASSERT_TRUE(child_pids.empty());
            }
        } else {
            for (size_t j = 0; j < child_entries.size(); j++) {
                const auto [pid, ts] = child_entries[j];
                const auto child_pids{rule.CheckProcess(ppid, pid, ts)};
                if (!child_pids.empty()) {
                    std::vector<std::pair<unsigned, unsigned>> child_entries_copy;
                    for (const auto pid : child_pids) {
                        auto it = std::find_if(child_entries.cbegin(), child_entries.cend(),
                            [pid](const auto& item){ return item.first == pid; });
                        if (it != child_entries.cend()) {
                            child_entries_copy.emplace_back((*it).first, (*it).second);
                        }
                    }

                    ASSERT_TRUE(child_entries_copy.size() == 6);
                    std::sort(child_entries_copy.begin(), child_entries_copy.end(),
                        [](const auto& lhs, const auto& rhs){ return lhs.second < rhs.second; });
                    const auto time_diff = child_entries_copy.back().second 
                                        - child_entries_copy.front().second;
                    ASSERT_TRUE(time_diff <= 10);
                }
            }
        }
    }    
}
