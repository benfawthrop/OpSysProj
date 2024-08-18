#ifndef OPSYSPROJ_RR_H
#define OPSYSPROJ_RR_H

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include "rng.h"
#include "process.h"

class rr {
public:
    rr(const std::vector<Process> &processes, int context_time, int t_slc)
    : processes(processes), t_cs(context_time), elapsed_time(0), t_slc(t_slc), cpu_util(0.0),
    cpu_wait(0.0), io_wait(0.0), cpu_turn(0.0), io_turn(0.0), num_cpu_switches(0), num_io_switches(0),
    cpu_preempt(0), io_preempt(0){
        simulate();
    }

    void simulate();
    void write_statistics(const std::string& filename);


private:
    std::vector<Process> processes;
    std::queue<Process> q; /* this would be a p queue for other algos */
    int t_cs, elapsed_time, t_slc;
    double cpu_util, cpu_wait, io_wait, cpu_turn, io_turn;
    int num_cpu_switches, num_io_switches, cpu_preempt, io_preempt;
    // helper function to get the end-of-line queue status updates
    std::string get_queue_status();
    // helper function to make our outputting to cout easier
    void print_line(const std::string& message) {
        std::cout << "time " << elapsed_time << "ms: " << message << " " << get_queue_status() << std::endl;
    }
};

#endif //OPSYSPROJ_RR_H
