#ifndef OPSYSPROJ_SRT_H
#define OPSYSPROJ_SRT_H

#include <vector>
#include <string>
#include <iostream>
#include "process.h"
#include <queue>
#include <algorithm>
#include <map>
#include <cmath>

class srt {
public:
    srt(const std::vector<Process>& processes, int context_time, double alpha, double lambda)
            : processes(processes), context_time(context_time), elapsed_time(0), alpha(alpha), lambda(lambda),
              cpu_util(0), cpu_wait(0), io_wait(0), cpu_turn(0), io_turn(0),
              num_cpu_switches(0), num_io_switches(0), cpu_preempt(0), io_preempt(0),
              cpu_bound_context_switches(0), io_bound_context_switches(0),
              cpu_bound_preemptions(0), io_bound_preemptions(0) {
        for (size_t i = 0; i < this->processes.size(); ++i) {
            this->processes[i].tau = std::ceil(1 / lambda);  // Initial tau value based on lambda
        }
    }

    void simulate();

    void write_statistics(const std::string& filename) const;

private:
    struct CompareRemainingTime {
        bool operator()(const Process &a, const Process &b) const {
            if (a.remaining_time == b.remaining_time) {
                return a.id > b.id;  // Tie-breaking by process ID
            }
            return a.remaining_time > b.remaining_time;
        }
    };

    std::vector<Process> processes;
    std::priority_queue<Process, std::vector<Process>, CompareRemainingTime> ready_queue;
    int context_time, elapsed_time;
    double alpha, lambda;

    // Statistics variables
    double cpu_util;
    double cpu_wait;
    double io_wait;
    double cpu_turn;
    double io_turn;
    int num_cpu_switches;
    int num_io_switches;
    int cpu_preempt;
    int io_preempt;

    // Context switch and preemption tracking
    int cpu_bound_context_switches;
    int io_bound_context_switches;
    int cpu_bound_preemptions;
    int io_bound_preemptions;
    static bool compare_by_arrival_time(const Process& a, const Process& b);

    std::map<int, Process> io_bound_map;
    std::priority_queue<int, std::vector<int>, std::greater<int> > io_bound_map_keys;

    void print_event(int time, const std::string &event, const std::priority_queue<Process, std::vector<Process>, CompareRemainingTime> &ready_queue);
    double calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda);
    bool processHasId(const Process &p, const std::string &id);
};

#endif //OPSYSPROJ_SRT_H
