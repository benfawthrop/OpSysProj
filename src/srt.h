//
// Created by Benjamin Fawthrop on 8/10/24.
//

#ifndef OPSYSPROJ_SRT_H
#define OPSYSPROJ_SRT_H
#include <vector>
#include <string>
#include <iostream>
#include "process.h"
#include <queue>


class srt {
public:
    //constructor
    srt(const std::vector<Process>& processes, int context_switch_time, double alpha)
            : processes(processes), context_switch_time(context_switch_time), elapsed_time(0), cpu_util(0.0),
              cpu_wait(0.0), io_wait(0.0), cpu_turn(0.0), io_turn(0.0), alpha(alpha), num_cpu_switches(0), num_io_switches(0),
              cpu_preempt(0), io_preempt(0) {
        sim_and_print();
    }

    // getters
    void sim_and_print();
    void write_statistics(const std::string& filename);
private:
    struct CompareTau {
        bool operator()(const Process &a, const Process &b) const {
            if (a.tau == b.tau) {
                return a.id > b.id;  // Reverse logic for max-heap behavior
            }
            return a.tau > b.tau;
        }
    };

    std::vector<Process> processes;
    std::priority_queue<Process, std::vector<Process>, CompareTau> q; /* this would be a p queue for other algos */
    int context_switch_time, elapsed_time;
    double cpu_util, cpu_wait, io_wait, cpu_turn, io_turn, alpha;
    int num_cpu_switches, num_io_switches, cpu_preempt, io_preempt;
    // helper function to get the end-of-line queue status updates
    std::string get_queue_status();
    // helper function to make our outputting to cout easier
    void print_line(const std::string& message) {
        std::cout << "time " << elapsed_time << "ms: " << message << " " << get_queue_status() << std::endl;
    }
};




#endif //OPSYSPROJ_SRT_H
