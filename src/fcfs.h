#ifndef OPSYSPROJ_FCFS_H
#define OPSYSPROJ_FCFS_H

#include <vector>
#include <iostream>
#include <string>
#include "rng.h"
#include "process.h"
#include <queue>
#include <cmath>




class fcfs {
public:
    //constructor
    fcfs(const std::vector<Process>& processes, int context_switch_time)
        : processes(processes), context_switch_time(context_switch_time), elapsed_time(0), cpu_util(0.0),
        cpu_wait(0.0), io_wait(0.0), cpu_turn(0.0), io_turn(0.0), num_cpu_switches(0), num_io_switches(0),
        cpu_preempt(0), io_preempt(0) {
        sim_and_print();
    }

    // getters
    void sim_and_print();
    void write_statistics(const std::string& filename) const;
//    void write_statistics(const std::string& filename) const;

private:
    std::vector<Process> processes;
    std::queue<Process> q; /* this would be a p queue for other algos */
    int context_switch_time, elapsed_time;
    double cpu_util, cpu_wait, io_wait, cpu_turn, io_turn;
    int num_cpu_switches, num_io_switches, cpu_preempt, io_preempt;
    // helper function to get the end-of-line queue status updates
    std::string get_queue_status();
    // helper function to make our outputting to cout easier
    void print_line(const std::string& message) {
        std::cout << "time " << elapsed_time << "ms: " << message << " " << get_queue_status() << std::endl;
    }
    // Define the comparison function
    static bool compare_by_arrival_time(const Process& a, const Process& b) {
        return a.arrival_time < b.arrival_time;
    }

};

#endif // OPSYSPROJ_FCFS_H
