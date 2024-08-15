#ifndef OPSYSPROJ_FCFS_H
#define OPSYSPROJ_FCFS_H

#include <vector>
#include <iostream>
#include <string>
#include "rng.h"
#include "process.h"
#include <queue>
#include <string>



class fcfs {
public:
    //constructor
    fcfs(const std::vector<Process>& processes, int context_switch_time)
        : processes(processes), context_switch_time(context_switch_time), cpu_util(0.0), elapsed_time(0) {
        sim_and_print();
        cpu_util = 1;
    }

    // getters
    void sim_and_print();
//    void write_statistics(const std::string& filename) const;

private:
    std::vector<Process> processes;
    std::queue<Process> q; /* this would be a p queue for other algos */
    int context_switch_time;
    double cpu_util;
    int elapsed_time;
    // helper function to get the end-of-line queue status updates
    std::string get_queue_status();
    // helper function to make our outputting to cout easier
    void print_line(const std::string& message) {
        std::cout << "time " << elapsed_time << "ms: " << message << " " << get_queue_status() << std::endl;
    }
};

#endif // OPSYSPROJ_FCFS_H
