#include "sjf.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>

// Constructor
sjf::sjf(std::vector<Process> processes, int context_time, double alpha)
        : processes(processes), context_time(context_time), alpha(alpha) {}

// Comparator function to sort processes by their current tau value
bool CompareTau(const Process &a, const Process &b) {
    if (a.tau == b.tau) {
        return a.id < b.id;  // Tie-breaking by process ID
    }
    return a.tau < b.tau;
}

// Function to print events with proper formatting
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

// Function to simulate the SJF scheduling
void sjf::simulate() {
    int time = 0;
    std::vector<Process> ready_queue;

    print_event(time, "Simulator started for SJF", ready_queue);

    while (!processes.empty() || !ready_queue.empty()) {
        // Add new arrivals to the ready queue
        for (size_t i = 0; i < processes.size();) {
            if (processes[i].arrival_time <= time) {
                processes[i].tau = 1000;  // Initial tau value for all processes
                ready_queue.push_back(processes[i]);
                print_event(time, "Process " + processes[i].id + " (tau 1000ms) arrived; added to ready queue", ready_queue);
                processes.erase(processes.begin() + i);
            } else {
                ++i;
            }
        }

        // Sort the ready queue by tau (Shortest Job First)
        std::sort(ready_queue.begin(), ready_queue.end(), CompareTau);

        if (!ready_queue.empty()) {
            Process &current_process = ready_queue[0];

            // Start process on the CPU
            print_event(time, "Process " + current_process.id + " (tau " + std::to_string(static_cast<int>(std::round(current_process.tau))) +
                              "ms) started using the CPU for " + std::to_string(current_process.bursts[0]) + "ms burst", ready_queue);
            int burst_time = current_process.bursts[0];
            time += burst_time; // Process runs

            // Process completed its burst
            print_event(time, "Process " + current_process.id + " (tau " + std::to_string(static_cast<int>(std::round(current_process.tau))) +
                              "ms) completed a CPU burst; " + std::to_string(current_process.bursts.size() / 2) + " bursts to go", ready_queue);

            // Recalculate tau after the burst
            double old_tau = current_process.tau;
            current_process.tau = std::ceil(alpha * burst_time + (1 - alpha) * current_process.tau);
            print_event(time, "Recalculated tau for process " + current_process.id + ": old tau " +
                              std::to_string(static_cast<int>(std::round(old_tau))) + "ms ==> new tau " + std::to_string(static_cast<int>(std::round(current_process.tau))) + "ms", ready_queue);

            ready_queue.erase(ready_queue.begin());

            if (!current_process.bursts.empty()) {
                current_process.bursts.erase(current_process.bursts.begin()); // Remove the completed burst

                if (!current_process.bursts.empty()) {
                    // Process moves to I/O and then returns to ready queue
                    int io_time = current_process.bursts[0];  // Assuming the next burst is the I/O time
                    current_process.arrival_time = time + io_time + context_time;
                    print_event(time, "Process " + current_process.id + " switching out of CPU; blocking on I/O until time " +
                                      std::to_string(current_process.arrival_time) + "ms", ready_queue);
                    processes.push_back(current_process); // Reinsert the process for the next I/O completion
                } else {
                    print_event(time, "Process " + current_process.id + " terminated", ready_queue);
                }
            }
        } else {
            time++;
        }
    }

    print_event(time, "Simulator ended for SJF", ready_queue);
}
 