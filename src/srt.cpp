#include "srt.h"
#include <cmath>
#include <iomanip>
#include <queue>

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
                    double new_tau = calculate_new_tau(current_process->tau, current_process->bursts.front(), alpha, lambda);                    print_event(elapsed_time, "Recalculated tau for process " + current_process->id + ": old tau " + std::to_string(int(current_process->tau)) + "ms ==> new tau " + std::to_string(int(new_tau)) + "ms", ready_queue);
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
}
