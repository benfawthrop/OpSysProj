#include "sjf.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <map>
#include <fstream>

// Helper function to calculate the new tau
double sjf::calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda) {
    return std::ceil(alpha * actual_burst + (1 - alpha) * old_tau);
}

// Function to compare tau values
bool sjf::CompareTau(const Process &a, const Process &b) {
    if (a.tau == b.tau) {
        return a.id < b.id;  // Tie-breaking by process ID
    }
    return a.tau < b.tau;
}

// sjf constructor definition
sjf::sjf(std::vector<Process> processes, int context_time, double alpha, double lambda)
        : processes(processes), context_time(context_time), alpha(alpha), lambda(lambda), elapsed_time(0),
          total_cpu_time(0), cpu_bound_wait_time(0), io_bound_wait_time(0),
          cpu_bound_turnaround_time(0), io_bound_turnaround_time(0),
          cpu_bound_context_switches(0), io_bound_context_switches(0),
          cpu_preempt(0), io_preempt(0), cpu_util(0.0), cpu_turn(0.0), io_turn(0.0), cpu_wait(0.0), io_wait(0.0),
          num_cpu_switches(0), num_io_switches(0) {
    // Initial tau setup for each process
    for (size_t i = 0; i < this->processes.size(); ++i) {
        this->processes[i].tau = std::ceil(1 / lambda);  // Initial tau value based on lambda
    }
}

// print_event function definition
void sjf::print_event(int time, const std::string &event, const std::vector<Process> &ready_queue) {
    std::cout << "time " << time << "ms: " << event << " [Q";
    if (ready_queue.empty()) {
        std::cout << " empty";
    } else {
        for (size_t i = 0; i < ready_queue.size(); ++i) {
            std::cout << " " << ready_queue[i].id;
        }
    }
    std::cout << "]" << std::endl;
}

// Function to check if a process has a specific ID
bool sjf::processHasId(const Process& p, const std::string& id) {
    return p.id == id;
}

// The simulate function implementation, including tau recalculation and event handling
void sjf::simulate() {
    print_event(0, "Simulator started for SJF", ready_queue);
    std::map<int, Process> io_bound_map;
    std::priority_queue<int, std::vector<int>, std::greater<int> > io_bound_map_keys;
    bool cpu_free = true;
    int i = 0;
    Process using_cpu;
    int time_cpu_frees = -1;
    int processes_killed = 0;

    while (processes_killed < processes.size()) {
        bool did_something = false;

        // Add any arriving processes to the ready queue
        if (i < processes.size()) {
            int curr_arrival = processes[i].arrival_time;
            if (curr_arrival >= elapsed_time &&
                (time_cpu_frees == -1 || curr_arrival <= time_cpu_frees) &&
                (io_bound_map_keys.empty() || curr_arrival <= io_bound_map_keys.top())) {

                processes[i].tau = std::ceil(1 / lambda);  // Initial tau value based on lambda
                ready_queue.push_back(processes[i]);
                elapsed_time = processes[i].arrival_time;

                std::sort(ready_queue.begin(), ready_queue.end(), CompareTau);

                if (elapsed_time <= 9999) {
                    print_event(elapsed_time, "Process " + processes[i].id + " (tau " + std::to_string(static_cast<int>(std::round(processes[i].tau))) + "ms) arrived; added to ready queue", ready_queue);
                }
                i++;
                did_something = true;
            }
        }

        // Check if we can send a process to the CPU
        if (cpu_free && !ready_queue.empty()) {
            cpu_free = false;
            elapsed_time += context_time / 2;
            using_cpu = ready_queue.front();
            int burst = using_cpu.bursts.front();  // Assign the first burst time
            time_cpu_frees = burst + elapsed_time;
            ready_queue.erase(ready_queue.begin());

            int wait_time = elapsed_time - using_cpu.arrival_time - (context_time / 2);  // Calculate the wait time
            if (using_cpu.is_cpu_bound) {
                cpu_bound_wait_time += wait_time;
            } else {
                io_bound_wait_time += wait_time;
            }

            total_cpu_time += burst;  // Track total CPU time

            if (elapsed_time <= 9999) {
                print_event(elapsed_time, "Process " + using_cpu.id + " (tau " + std::to_string(static_cast<int>(std::round(using_cpu.tau))) +
                                          "ms) started using the CPU for " + std::to_string(burst) + "ms burst", ready_queue);
            }
            using_cpu.bursts.erase(using_cpu.bursts.begin());
            did_something = true;
        }

        // Handle CPU or IO events
        if (!did_something || time_cpu_frees == elapsed_time ||
            (!io_bound_map_keys.empty() && io_bound_map_keys.top() == elapsed_time)) {
            if (time_cpu_frees > 0 && time_cpu_frees <= (!io_bound_map_keys.empty() ? io_bound_map_keys.top() : time_cpu_frees)) {
                elapsed_time = time_cpu_frees;
                time_cpu_frees = -1;
                cpu_free = true;

                // Process just completed a CPU burst
                if (elapsed_time <= 9999) {
                    print_event(elapsed_time, "Process " + using_cpu.id + " (tau " + std::to_string(static_cast<int>(std::round(using_cpu.tau))) +
                                              "ms) completed a CPU burst; " + std::to_string(using_cpu.bursts.size() / 2) + " bursts to go", ready_queue);
                }

                // Track turnaround time
                int turnaround_time = elapsed_time - using_cpu.arrival_time;
                if (using_cpu.is_cpu_bound) {
                    cpu_bound_turnaround_time += turnaround_time;
                    cpu_bound_context_switches++;
                } else {
                    io_bound_turnaround_time += turnaround_time;
                    io_bound_context_switches++;
                }

                // Check if the process has more bursts to execute
                if (!using_cpu.bursts.empty()) {
                    int actual_burst = using_cpu.bursts.front();
                    int old_tau = using_cpu.tau;
                    using_cpu.tau = calculate_new_tau(old_tau, actual_burst, alpha, lambda);

                    if (elapsed_time <= 9999) {
                        print_event(elapsed_time, "Recalculated tau for process " + using_cpu.id + ": old tau " +
                                                  std::to_string(static_cast<int>(std::round(old_tau))) + "ms ==> new tau " + std::to_string(static_cast<int>(std::round(using_cpu.tau))) + "ms", ready_queue);
                    }

                    // Now the process moves to I/O
                    using_cpu.bursts.erase(using_cpu.bursts.begin());
                    int io_completion_time = elapsed_time + context_time + actual_burst;
                    io_bound_map[io_completion_time] = using_cpu;
                    io_bound_map_keys.push(io_completion_time);

                    elapsed_time += context_time / 2;

                    if (elapsed_time <= 9999) {
                        print_event(elapsed_time, "Process " + using_cpu.id + " switching out of CPU; blocking on I/O until time " +
                                                  std::to_string(io_completion_time) + "ms", ready_queue);
                    }
                } else {
                    // If the process has no more bursts, it terminates
                    print_event(elapsed_time, "Process " + using_cpu.id + " terminated", ready_queue);
                    processes_killed++;
                    using_cpu = Process();  // Clear using_cpu to avoid further operations
                    time_cpu_frees = -1;    // Clear time_cpu_frees to avoid further processing
                }

            } else if (!io_bound_map_keys.empty() && (time_cpu_frees >= io_bound_map_keys.top() || time_cpu_frees == -1)) {
                elapsed_time = io_bound_map_keys.top();
                Process process_from_io = io_bound_map[elapsed_time];
                io_bound_map.erase(elapsed_time);
                io_bound_map_keys.pop();

                // Only re-add the process to the ready queue if it hasn't terminated
                bool found = false;
                for (std::vector<Process>::iterator it = ready_queue.begin(); it != ready_queue.end(); ++it) {
                    if (processHasId(*it, process_from_io.id)) {
                        found = true;
                        break;
                    }
                }

                if (!process_from_io.bursts.empty() && !found) {
                    ready_queue.push_back(process_from_io);
                    std::sort(ready_queue.begin(), ready_queue.end(), CompareTau);

                    if (elapsed_time <= 9999) {
                        print_event(elapsed_time, "Process " + process_from_io.id + " completed I/O; added to ready queue", ready_queue);
                    }
                }
            }
        }
    }

    print_event(elapsed_time, "Simulator ended for SJF", ready_queue);

    // Update statistics after simulation
    if (elapsed_time > 0) {
        cpu_util = (double)total_cpu_time / elapsed_time;
    }
    if (cpu_bound_context_switches > 0) {
        cpu_turn = (double)cpu_bound_turnaround_time / cpu_bound_context_switches;
        cpu_wait = (double)cpu_bound_wait_time / cpu_bound_context_switches;
    }
    if (io_bound_context_switches > 0) {
        io_turn = (double)io_bound_turnaround_time / io_bound_context_switches;
        io_wait = (double)io_bound_wait_time / io_bound_context_switches;
    }
    num_cpu_switches = cpu_bound_context_switches;
    num_io_switches = io_bound_context_switches;
}

// Implementing the write_statistics function for SJF
void sjf::write_statistics(const std::string& filename) const {
    std::fstream outfile(filename, std::ios::app);

    outfile << "Algorithm SJF" << std::endl;
    outfile << "-- CPU utilization: " << std::fixed << std::setprecision(3) << (cpu_util * 100) << "%" << std::endl;
    outfile << "-- CPU-bound average wait time: " << std::fixed << std::setprecision(3) << (cpu_bound_context_switches > 0 ? cpu_wait : 0) << " ms" << std::endl;
    outfile << "-- I/O-bound average wait time: " << std::fixed << std::setprecision(3) << (io_bound_context_switches > 0 ? io_wait : 0) << " ms" << std::endl;
    outfile << "-- Overall average wait time: " << std::fixed << std::setprecision(3) << ((cpu_bound_context_switches > 0 && io_bound_context_switches > 0) ? ((cpu_wait + io_wait) / 2) : 0) << " ms" << std::endl;
    outfile << "-- CPU-bound average turnaround time: " << std::fixed << std::setprecision(3) << (cpu_bound_context_switches > 0 ? cpu_turn : 0) << " ms" << std::endl;
    outfile << "-- I/O-bound average turnaround time: " << std::fixed << std::setprecision(3) << (io_bound_context_switches > 0 ? io_turn : 0) << " ms" << std::endl;
    outfile << "-- Overall average turnaround time: " << std::fixed << std::setprecision(3) << ((cpu_bound_context_switches > 0 && io_bound_context_switches > 0) ? ((cpu_turn + io_turn) / 2) : 0) << " ms" << std::endl;
    outfile << "-- CPU-bound number of context switches: " << num_cpu_switches << std::endl;
    outfile << "-- I/O-bound number of context switches: " << num_io_switches << std::endl;
    outfile << "-- Overall number of context switches: " << num_cpu_switches + num_io_switches << std::endl;
    outfile << "-- CPU-bound number of preemptions: " << cpu_preempt << std::endl;
    outfile << "-- I/O-bound number of preemptions: " << io_preempt << std::endl;
    outfile << "-- Overall number of preemptions: " << cpu_preempt + io_preempt << std::endl << std::endl;

    outfile.close();
}
