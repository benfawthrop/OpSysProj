#include "fcfs.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>


void fcfs::sim_and_print() {
    print_line("Simulator started for FCFS");

    std::map<int, Process> io_bound_map;
    std::priority_queue<int, std::vector<int>, std::greater<int>> io_bound_map_keys;
    std::queue<int> times_entered_q;
    bool cpu_free = true;
    int i = 0;
    Process using_cpu;
    int time_cpu_frees = -1;
    int processes_killed = 0;

    int total_cpu_time = 0;
//    int total_cpu_bursts = 0, total_io_bursts = 0;

    // Separate tracking for CPU-bound and I/O-bound processes
    int cpu_bound_wait_time = 0, io_bound_wait_time = 0;
    int cpu_bound_turnaround_time = 0, io_bound_turnaround_time = 0;
    int cpu_bound_context_switches = 0, io_bound_context_switches = 0;

    // while there are processes alive
    while (processes_killed < processes.size()) {
        bool did_something = false;

        if (i < processes.size()) {
            int curr_arrival = processes[i].arrival_time;
            if (curr_arrival >= elapsed_time && (time_cpu_frees == -1 || curr_arrival <= time_cpu_frees) &&
                (io_bound_map_keys.empty() || curr_arrival <= io_bound_map_keys.top())) {

                q.push(processes[i]);
                times_entered_q.push(elapsed_time);
                elapsed_time = processes[i].arrival_time;
                if (elapsed_time <= 9999) {
                    print_line("Process " + processes[i].id + " arrived; added to the ready queue");
                }
                i++;
                did_something = true;
            }
        }

        if (cpu_free && !q.empty()) {
            cpu_free = false;
            elapsed_time += context_switch_time / 2;
            using_cpu = q.front();
            q.pop();
            int wait_time = elapsed_time - times_entered_q.front() - (context_switch_time / 2);
            times_entered_q.pop();

            if (using_cpu.is_cpu_bound) {
                cpu_bound_wait_time += wait_time;
            } else {
                io_bound_wait_time += wait_time;
            }

            time_cpu_frees = using_cpu.bursts.front() + elapsed_time;
            if (elapsed_time <= 9999) {
                print_line("Process " + using_cpu.id + " started using the CPU for " +
                           std::to_string(using_cpu.bursts.front()) + "ms burst");
            }
            int turn_around = using_cpu.bursts.front() + context_switch_time;
            if (using_cpu.is_cpu_bound) {
                cpu_bound_turnaround_time += turn_around;
            } else {
                io_bound_turnaround_time += turn_around;
            }

            total_cpu_time += using_cpu.bursts.front();
            using_cpu.bursts.erase(using_cpu.bursts.begin());

            if (using_cpu.is_cpu_bound) {
                cpu_bound_context_switches++;
            } else {
                io_bound_context_switches++;
            }

            did_something = true;
        }

        if (!did_something || time_cpu_frees == elapsed_time ||
            (!io_bound_map_keys.empty() && io_bound_map_keys.top() == elapsed_time)) {
            if (time_cpu_frees > 0 && time_cpu_frees <= (!io_bound_map_keys.empty() ? io_bound_map_keys.top() : time_cpu_frees)) {
                elapsed_time = time_cpu_frees;
                time_cpu_frees = -1;
                cpu_free = true;

                int burst = using_cpu.bursts.front();
                if (elapsed_time <= 9999) {
                    print_line("Process " + using_cpu.id + " completed a CPU burst; " +
                               std::to_string((using_cpu.bursts.size() / 2)) + " bursts to go");

                    if (using_cpu.bursts.size() > 0) {
                        print_line("Process " + using_cpu.id + " switching out of CPU; blocking on I/O until time " +
                                   std::to_string(elapsed_time + burst + (context_switch_time / 2)) + "ms");
                    }
                }

                if (using_cpu.bursts.size() == 0) {
                    print_line("Process " + using_cpu.id + " terminated");
                    processes_killed++;
//                    int turnaround_time = elapsed_time - using_cpu.arrival_time;
//                    if (using_cpu.is_cpu_bound) {
//                        cpu_bound_turnaround_time += turnaround_time;
//                    } else {
//                        io_bound_turnaround_time += turnaround_time;
//                    }
                } else {
                    using_cpu.bursts.erase(using_cpu.bursts.begin());
                    io_bound_map[elapsed_time + burst + (context_switch_time / 2)] = using_cpu;
                    io_bound_map_keys.push(elapsed_time + burst + (context_switch_time / 2));
                }
                elapsed_time += context_switch_time / 2;
            } else if (io_bound_map_keys.size() > 0 && (time_cpu_frees >= io_bound_map_keys.top() || time_cpu_frees == -1)) {
                elapsed_time = io_bound_map_keys.top();
                q.push(io_bound_map[elapsed_time]);
                times_entered_q.push(elapsed_time);
                if (elapsed_time <= 9999) {
                    print_line("Process " + io_bound_map[elapsed_time].id + " completed I/O; added to ready queue");
                }
                io_bound_map.erase(elapsed_time);
                io_bound_map_keys.pop();
            }
        }
    }

    print_line("Simulator ended for FCFS");

    cpu_util = (double)total_cpu_time / elapsed_time;
    cpu_turn = (double)cpu_bound_turnaround_time / cpu_bound_context_switches;
    io_turn = (double)io_bound_turnaround_time / io_bound_context_switches;

    cpu_wait = (double)cpu_bound_wait_time / cpu_bound_context_switches;
    io_wait = (double)io_bound_wait_time / (io_bound_context_switches / 2);
    num_cpu_switches = cpu_bound_context_switches;
    num_io_switches = io_bound_context_switches;
}

std::string fcfs::get_queue_status() {
    std::string result = "[Q";
    if (q.empty()) {
        result.append(" empty]");
    } else {
        std::queue<Process> temp = q;
        while(!temp.empty()) {
            result.append(" " + temp.front().id);
            temp.pop();
        }
        result.append("]");
    }

    return result;
}

void fcfs::write_statistics(const std::string& filename) {
    // TODO: Fix bugs with data collection for this
    std::fstream outfile(filename, std::ios::out);

    outfile << "Algorithm FCFS" << std::endl;
    outfile << "-- CPU utilization: " << std::fixed << std::setprecision(3) << (std::ceil(cpu_util * 100000) / 1000) << "%" << std::endl;
    outfile << "-- CPU-bound average wait time: " << std::fixed << std::setprecision(3) << cpu_wait << " ms" << std::endl;
    outfile << "-- I/O-bound average wait time: " << std::fixed << std::setprecision(3) << io_wait << " ms" << std::endl;
    outfile << "-- Overall average wait time: " << std::fixed << std::setprecision(3) << ((cpu_wait + io_wait) / 2) << " ms" << std::endl;
    outfile << "-- CPU-bound average turnaround time: " << std::fixed << std::setprecision(3) << cpu_turn << " ms" << std::endl;
    outfile << "-- I/O-bound average turnaround time: " << std::fixed << std::setprecision(3) << io_turn << " ms" << std::endl;
    outfile << "-- Overall average turnaround time: " << std::fixed << std::setprecision(3) << ((cpu_turn + io_turn) / 2) << " ms" << std::endl;
    outfile << "-- CPU-bound context switches: " << num_cpu_switches << std::endl;
    outfile << "-- I/O-bound context switches: " << num_io_switches << std::endl;
    outfile << "-- Overall context switches: " << num_cpu_switches + num_io_switches << std::endl;
    outfile << "-- CPU-bound preemptions: " << cpu_preempt << std::endl;
    outfile << "-- I/O-bound preemptions: " << io_preempt << std::endl;
    outfile << "-- Overall preemptions: " << cpu_preempt + io_preempt << std::endl;

//    outfile.flush();
    outfile.close();
}

