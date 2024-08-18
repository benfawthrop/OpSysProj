#ifndef OPSYSPROJ_SRT_H
#define OPSYSPROJ_SRT_H

#include <vector>
#include <string>
#include <iostream>
#include "process.h"
#include <queue>
#include <algorithm>
#include <map>

class srt {
public:
    srt(const std::vector<Process>& processes, int context_time, double alpha, double lambda)
            : processes(processes), context_time(context_time), elapsed_time(0), alpha(alpha), lambda(lambda) {
        for (size_t i = 0; i < this->processes.size(); ++i) {
            this->processes[i].tau = std::ceil(1 / lambda);  // Initial tau value based on lambda
        }
    }

    void simulate();

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
    std::map<int, Process> io_bound_map;
    std::priority_queue<int, std::vector<int>, std::greater<int> > io_bound_map_keys;

    void print_event(int time, const std::string &event, const std::priority_queue<Process, std::vector<Process>, CompareRemainingTime> &ready_queue);
    double calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda);
    bool processHasId(const Process &p, const std::string &id);
};

#endif //OPSYSPROJ_SRT_H
