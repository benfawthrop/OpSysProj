#include "fcfs.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <fstream>

// All lowercase class name and function definitions
fcfs::fcfs(const std::vector<Process>& processes, int context_switch_time)
        : processes(processes), context_switch_time(context_switch_time),
          cpu_utilization(0.0), average_turnaround_time(0.0),
          average_wait_time(0.0), total_context_switches(0) {}

void fcfs::simulate() {
    run_simulation();
}

// Function to print the current state of the ready queue
void print_queue(const std::queue<Process>& ready_queue) {
    std::queue<Process> copy = ready_queue;
    if (copy.empty()) {
        std::cout << "empty";
    } else {
        while (!copy.empty()) {
            std::cout << copy.front().id;
            copy.pop();
            if (!copy.empty()) {
                std::cout << " ";
            }
        }
    }
}

void fcfs::run_simulation() {
    int time = 0;
    std::queue<Process> ready_queue;

    std::cout << "time " << time << "ms: Simulator started for FCFS [Q empty]" << std::endl;

    Process* current_process = nullptr;
    int burst_completion_time = 0;
    int total_processes = processes.size();
    int terminated_processes = 0;

    // Simulation loop
    while (terminated_processes < total_processes || !ready_queue.empty() || current_process != nullptr) {
        // Handle new arrivals at the current time
        for (size_t i = 0; i < processes.size(); ++i) {
            if (processes[i].arrival_time == time) {
                ready_queue.push(processes[i]);
                std::cout << "time " << time << "ms: Process " << processes[i].id << " arrived; added to ready queue [Q ";
                print_queue(ready_queue);
                std::cout << "]" << std::endl;
            }
        }

        // If there's no current process, take the next one from the queue
        if (current_process == nullptr && !ready_queue.empty()) {
            current_process = &ready_queue.front();
            ready_queue.pop();

            int burst_time = current_process->bursts.front();
            burst_completion_time = time + burst_time;

            std::cout << "time " << time << "ms: Process " << current_process->id
                      << " started using the CPU for " << burst_time << "ms burst [Q ";
            print_queue(ready_queue);
            std::cout << "]" << std::endl;
        }

        // Check if the current process has completed its CPU burst
        if (current_process != nullptr && time == burst_completion_time) {
            current_process->bursts.erase(current_process->bursts.begin());
            std::cout << "time " << time << "ms: Process " << current_process->id
                      << " completed a CPU burst; " << current_process->bursts.size() / 2 << " bursts to go [Q ";
            print_queue(ready_queue);
            std::cout << "]" << std::endl;

            if (!current_process->bursts.empty()) {
                int io_completion_time = time + context_switch_time + current_process->bursts.front();
                current_process->arrival_time = io_completion_time;
                std::cout << "time " << time << "ms: Process " << current_process->id
                          << " switching out of CPU; blocking on I/O until time " << io_completion_time << "ms [Q ";
                print_queue(ready_queue);
                std::cout << "]" << std::endl;
            } else {
                std::cout << "time " << time << "ms: Process " << current_process->id << " terminated [Q ";
                print_queue(ready_queue);
                std::cout << "]" << std::endl;
                terminated_processes++;
            }

            // The CPU becomes available again
            current_process = nullptr;
            time += context_switch_time / 2;  // Simulate half of the context switch time
        }

        // Increment time
        time++;
    }

    std::cout << "time " << time << "ms: Simulator ended for FCFS [Q empty]" << std::endl;
}





void fcfs::print_results() const {
    // This can be used to output final summary, but now all events are printed during the simulation.
}

void fcfs::write_statistics(const std::string& filename) const {
    std::ofstream out(filename.c_str(), std::ios::app);
    out << "Algorithm FCFS" << std::endl;
    out << "-- CPU utilization: " << std::fixed << std::setprecision(3) << cpu_utilization << "%" << std::endl;
    out << "-- Average turnaround time: " << std::fixed << std::setprecision(3) << average_turnaround_time << " ms" << std::endl;
    out << "-- Average wait time: " << std::fixed << std::setprecision(3) << average_wait_time << " ms" << std::endl;
    out << "-- Total context switches: " << total_context_switches << std::endl;
    out.close();
}
