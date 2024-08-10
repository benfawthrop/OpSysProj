#ifndef OPSYSPROJ_FCFS_H
#define OPSYSPROJ_FCFS_H

#include <vector>
#include <string>
#include "rng.h"
#include "process.h"




class fcfs {  // Lowercase class name
public:
    fcfs(const std::vector<Process>& processes, int context_switch_time);

    void simulate();
    void print_results() const;
    void write_statistics(const std::string& filename) const;

private:
    std::vector<Process> processes;
    int context_switch_time;

    double cpu_utilization;
    double average_turnaround_time;
    double average_wait_time;
    int total_context_switches;

    void run_simulation();
};

#endif // OPSYSPROJ_FCFS_H
