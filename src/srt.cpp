#include "srt.h"
#include <cmath>
#include <iomanip>
#include <queue>
#include <fstream>


// Helper function to print events and queue status
void srt::print_event(int time, const std::string &event, const std::priority_queue<Process, std::vector<Process>, CompareRemainingTime> &ready_queue) {
    std::cout << "time " << time << "ms: " << event << " [Q";

    if (ready_queue.empty()) {
        std::cout << " empty";
    } else {
        std::priority_queue<Process, std::vector<Process>, CompareRemainingTime> temp_queue = ready_queue;
        while (!temp_queue.empty()) {
            std::cout << " " << temp_queue.top().id;
            temp_queue.pop();
        }
    }

    std::cout << "]" << std::endl;
}

// Helper function to calculate new tau using exponential averaging
double srt::calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda) {
    return alpha * actual_burst + (1 - alpha) * old_tau;
}

void srt::simulate() {
    print_event(elapsed_time, "Simulator started for SRT", ready_queue);

    Process* current_process = nullptr;
    int time_cpu_frees = -1;  // Time when the CPU becomes free
    int context_switch_time_remaining = 0;

    int total_cpu_time = 0;
    int cpu_bound_wait_time = 0, io_bound_wait_time = 0;
    int cpu_bound_turnaround_time = 0, io_bound_turnaround_time = 0;

    while (!processes.empty() || !ready_queue.empty() || current_process != nullptr || !io_bound_map_keys.empty()) {
        bool did_something = false;

        // Handle arriving processes
        while (!processes.empty() && processes.front().arrival_time <= elapsed_time) {
            Process new_process = processes.front();
            processes.erase(processes.begin());

            new_process.remaining_time = new_process.bursts.front();  // Set initial remaining time to the first burst length
            ready_queue.push(new_process);
            print_event(elapsed_time, "Process " + new_process.id + " (tau " + std::to_string(int(new_process.tau)) + "ms) arrived; added to ready queue", ready_queue);

            // Check for preemption
            if (current_process && new_process.remaining_time < current_process->remaining_time) {
                print_event(elapsed_time, "Process " + new_process.id + " (tau " + std::to_string(int(new_process.tau)) + "ms) preempting " + current_process->id, ready_queue);
                ready_queue.push(*current_process);
                current_process = nullptr;
                time_cpu_frees = elapsed_time + context_time / 2; // Time when CPU will be free after context switch
                context_switch_time_remaining = context_time / 2;

                if (new_process.is_cpu_bound) {
                    cpu_bound_preemptions++;
                } else {
                    io_bound_preemptions++;
                }
            }
            did_something = true;
        }

        // Handle I/O completion
        if (!io_bound_map_keys.empty() && io_bound_map_keys.top() == elapsed_time) {
            Process io_completed = io_bound_map[elapsed_time];
            io_bound_map.erase(elapsed_time);
            io_bound_map_keys.pop();

            io_completed.remaining_time = io_completed.tau;
            ready_queue.push(io_completed);
            print_event(elapsed_time, "Process " + io_completed.id + " (tau " + std::to_string(int(io_completed.tau)) + "ms) completed I/O; added to ready queue", ready_queue);

            // Check for preemption
            if (current_process && io_completed.remaining_time < current_process->remaining_time) {
                print_event(elapsed_time, "Process " + io_completed.id + " (tau " + std::to_string(int(io_completed.tau)) + "ms) preempting " + current_process->id, ready_queue);
                ready_queue.push(*current_process);
                current_process = nullptr;
                time_cpu_frees = elapsed_time + context_time / 2;
                context_switch_time_remaining = context_time / 2;

                if (io_completed.is_cpu_bound) {
                    cpu_bound_preemptions++;
                } else {
                    io_bound_preemptions++;
                }
            }
            did_something = true;
        }

        // Handle context switch or start new process
        if (context_switch_time_remaining > 0) {
            context_switch_time_remaining--;
            did_something = true;
        } else if (current_process == nullptr && !ready_queue.empty() && elapsed_time >= time_cpu_frees) {
            current_process = new Process(ready_queue.top());
            ready_queue.pop();

            time_cpu_frees = elapsed_time + current_process->remaining_time;
            print_event(elapsed_time, "Process " + current_process->id + " (tau " + std::to_string(int(current_process->tau)) + "ms) started using the CPU for " + std::to_string(current_process->remaining_time) + "ms burst", ready_queue);

            if (current_process->is_cpu_bound) {
                cpu_bound_context_switches++;
            } else {
                io_bound_context_switches++;
            }
            total_cpu_time += current_process->remaining_time;  // Track total CPU time
            did_something = true;
        }

        // Handle CPU burst completion
        if (current_process && elapsed_time == time_cpu_frees) {
            current_process->bursts.front() -= current_process->remaining_time;
            if (current_process->bursts.front() <= 0) {
                current_process->bursts.erase(current_process->bursts.begin());

                if (current_process->bursts.empty()) {
                    print_event(elapsed_time, "Process " + current_process->id + " terminated", ready_queue);
                    delete current_process;
                    current_process = nullptr;
                } else {
                    double new_tau = calculate_new_tau(current_process->tau, current_process->bursts.front(), alpha, lambda);
                    print_event(elapsed_time, "Recalculated tau for process " + current_process->id + ": old tau " + std::to_string(int(current_process->tau)) + "ms ==> new tau " + std::to_string(int(new_tau)) + "ms", ready_queue);
                    current_process->tau = new_tau;
                    current_process->remaining_time = new_tau;
                    io_bound_map[elapsed_time + new_tau + context_time / 2] = *current_process;
                    io_bound_map_keys.push(elapsed_time + new_tau + context_time / 2);
                    delete current_process;
                    current_process = nullptr;
                }
            } else {
                current_process->remaining_time = 0; // Finished burst
                delete current_process;
                current_process = nullptr;
            }
            did_something = true;
        }

        // Prevent infinite loop by incrementing time when nothing is done
        if (!did_something) {
            elapsed_time++;
        }
    }

    print_event(elapsed_time, "Simulator ended for SRT", ready_queue);

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
    cpu_preempt = cpu_bound_preemptions;
    io_preempt = io_bound_preemptions;
}

// Implementing the write_statistics function for SRT
void srt::write_statistics(const std::string& filename) const {
    std::fstream outfile(filename, std::ios::app);

    outfile << "Algorithm SRT" << std::endl;
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
