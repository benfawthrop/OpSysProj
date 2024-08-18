#ifndef OPSYSPROJ_SJF_H
#define OPSYSPROJ_SJF_H

#include "process.h"
#include <vector>
#include <string>
#include <algorithm>


class sjf {
public:
    sjf(std::vector<Process> processes, int context_time, double alpha, double lambda);

    void simulate();

    void write_statistics(const std::string& filename) const;

private:
    std::vector<Process> processes;
    std::vector<Process> ready_queue;  // Ready queue for SJF
    int context_time;
    double alpha;  // Alpha value for tau recalculation
    double lambda; // Lambda value for tau recalculation
    int elapsed_time;

    int total_cpu_time;
    int cpu_bound_wait_time, io_bound_wait_time;
    int cpu_bound_turnaround_time, io_bound_turnaround_time;
    int cpu_bound_context_switches, io_bound_context_switches;
    int cpu_preempt, io_preempt;

    double cpu_util, cpu_turn, io_turn, cpu_wait, io_wait;
    int num_cpu_switches, num_io_switches;

    static bool CompareTau(const Process &a, const Process &b);
    static bool compare_by_arrival_time(const Process& a, const Process& b);

    double calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda);

    void print_event(int time, const std::string &event, const std::vector<Process> &ready_queue);

    bool processHasId(const Process& p, const std::string& id);
};

#endif // OPSYSPROJ_SJF_H
