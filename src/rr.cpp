// Created by Benjamin Fawthrop, Jimmy Wang on 8/10/24.

#include "rr.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <map>
#include <cmath>



std::string rr::get_queue_status() {
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

void rr::simulate() {
    std::sort(processes.begin(), processes.end(), compare_by_arrival_time);
    print_line("Simulator started for RR");

    std::map<int, Process> io_bound_map;
    std::priority_queue<int, std::vector<int>, std::greater<int> > io_bound_map_keys;
    std::queue<int> times_entered_q;
    std::map<std::string, int> cpu_burst_active; // map for seeing if current cpu burst has been done before
    bool cpu_free = true;
    int i = 0;
    Process using_cpu;
    int time_cpu_frees = -1;
    int processes_killed = 0;


    int total_cpu_time = 0;

    // Separate tracking for CPU-bound and I/O-bound processes
    int cpu_bound_wait_time = 0, io_bound_wait_time = 0;
    int cpu_bound_turnaround_time = 0, io_bound_turnaround_time = 0;
    int cpu_bursts_count = 0, io_bursts_count = 0;
    int cpu_one_slice = 0, io_one_slice = 0;

    // while there are processes alive
    while (processes_killed < (int) processes.size()) {
        bool did_something = false;

        if (i < (int) processes.size()) {
            int curr_arrival = processes[i].arrival_time;
            if (curr_arrival >= elapsed_time && (time_cpu_frees == -1 || curr_arrival <= time_cpu_frees) &&
                (io_bound_map_keys.empty() || curr_arrival <= io_bound_map_keys.top())) {

                q.push(processes[i]);
                times_entered_q.push(elapsed_time);
                elapsed_time = processes[i].arrival_time;
                if (elapsed_time <= 9999) {
                    print_line("Process " + processes[i].id + " arrived; added to ready queue");
                }
                i++;
                cpu_burst_active[processes[i].id] = 0;
                did_something = true;
            }
        }

        if (cpu_free && !q.empty() /* &&
            (io_bound_map_keys.empty() || elapsed_time + (t_cs / 2) < io_bound_map_keys.top()) */) {
            cpu_free = false;
            elapsed_time += t_cs / 2;
            using_cpu = q.front();
            q.pop();
            int wait_time = elapsed_time - times_entered_q.front() - (t_cs / 2);
            times_entered_q.pop();

            if (using_cpu.is_cpu_bound) {
                cpu_bound_wait_time += wait_time;
            } else {
                io_bound_wait_time += wait_time;
            }

            if (cpu_burst_active[using_cpu.id] == 0) {
                cpu_burst_active[using_cpu.id] = using_cpu.bursts.front();
                if (elapsed_time <= 9999) {
                    print_line("Process " + using_cpu.id + " started using the CPU for " +
                               std::to_string(using_cpu.bursts.front()) + "ms burst");
                }
                
                if (using_cpu.is_cpu_bound) {
                    cpu_bursts_count++;
                    if (t_slc > using_cpu.bursts.front()) {
                        cpu_one_slice++;
                    }
                } else {
                    io_bursts_count++;
                    if (t_slc > using_cpu.bursts.front()) {
                        io_one_slice++;
                    }
                }

            } else if (elapsed_time <= 9999) {
                print_line("Process " + using_cpu.id + " started using the CPU for remaining " +
                           std::to_string(using_cpu.bursts.front()) + "ms of " +
                           std::to_string(cpu_burst_active[using_cpu.id]) + "ms burst");
            }

            int turn_around = using_cpu.bursts.front() + t_cs;
            if (using_cpu.is_cpu_bound) {
                cpu_bound_turnaround_time += turn_around;
            } else {
                io_bound_turnaround_time += turn_around;
            }

//            total_cpu_time += using_cpu.bursts.front();

            if (using_cpu.is_cpu_bound) {
                num_cpu_switches++;
            } else {
                num_io_switches++;
            }

            time_cpu_frees = t_slc < using_cpu.bursts.front() ? t_slc + elapsed_time : using_cpu.bursts.front() + elapsed_time;
            total_cpu_time += time_cpu_frees - elapsed_time;
            using_cpu.bursts.front() -= t_slc;

            did_something = true;
        }

        if (!did_something || time_cpu_frees == elapsed_time ||
            (!io_bound_map_keys.empty() && io_bound_map_keys.top() == elapsed_time)) {
            if (time_cpu_frees > 0 && time_cpu_frees <= (!io_bound_map_keys.empty() ? io_bound_map_keys.top() : time_cpu_frees)) {
                elapsed_time = time_cpu_frees;
                if (using_cpu.bursts.front() <= 0) {
                    // IF PROCESS COMPLETES AND DOESN'T GET CUT OFF
                    time_cpu_frees = -1;
                    cpu_free = true;

                    
                    // pops CPU burst
                    using_cpu.bursts.erase(using_cpu.bursts.begin());
                    

                    int burst = using_cpu.bursts.front();
                    cpu_burst_active[using_cpu.id] = 0;
                    if (elapsed_time <= 9999 && using_cpu.bursts.size() > 0) {
                        print_line("Process " + using_cpu.id + " completed a CPU burst; " +
                            std::to_string((using_cpu.bursts.size() / 2)) + " burst" +
                            ((using_cpu.bursts.size() / 2) > 1 ? "s" : "") + " to go");
                        print_line("Process " + using_cpu.id + " switching out of CPU; blocking on I/O until time " +
                            std::to_string(elapsed_time + burst + (t_cs / 2)) + "ms");
                    }

                    if (using_cpu.bursts.empty()) {
                        print_line("Process " + using_cpu.id + " terminated");
                        processes_killed++;
                    } else {
                        using_cpu.bursts.erase(using_cpu.bursts.begin());
                        io_bound_map[elapsed_time + burst + (t_cs / 2)] = using_cpu;
                        io_bound_map_keys.push(elapsed_time + burst + (t_cs / 2));
                    }
                    elapsed_time += t_cs / 2;
                } else if (q.empty()) {
                    // IF SLICE ENDS AND THERE IS NOTHING IN THE QUEUE
                    time_cpu_frees = t_slc < using_cpu.bursts.front() ? t_slc + elapsed_time : using_cpu.bursts.front() + elapsed_time;
                    total_cpu_time += time_cpu_frees - elapsed_time;
                    using_cpu.bursts.front() -= t_slc;
                    if (elapsed_time <= 9999) {
                        print_line("Time slice expired; no preemption because ready queue is empty");
                    }
                } else {
                    if (elapsed_time <= 9999) {
                        // if allowed
                        print_line("Time slice expired; preempting process " + using_cpu.id + " with " +
                                std::to_string(using_cpu.bursts.front()) + "ms remaining");
                    }
                    if (using_cpu.is_cpu_bound) {
                        cpu_preempt++;
                    } else {
                        io_preempt++;
                    }
                    q.push(using_cpu);
                    times_entered_q.push(elapsed_time);
                    using_cpu = q.front();
                    q.pop();
                    elapsed_time += t_cs;

                    if (cpu_burst_active[using_cpu.id] == 0) {
                        cpu_burst_active[using_cpu.id] = using_cpu.bursts.front();
                        if (elapsed_time <= 9999) {
                            print_line("Process " + using_cpu.id + " started using the CPU for " +
                                       std::to_string(using_cpu.bursts.front()) + "ms burst");
                        }
                        if (using_cpu.is_cpu_bound) {
                            cpu_bursts_count++;
                            if (t_slc > using_cpu.bursts.front()) {
                                cpu_one_slice++;
                            }
                        } else {
                            io_bursts_count++;
                            if (t_slc > using_cpu.bursts.front()) {
                                io_one_slice++;
                            }
                        }
                    } else if (elapsed_time <= 9999) {
                        print_line("Process " + using_cpu.id + " started using the CPU for remaining " +
                                   std::to_string(using_cpu.bursts.front()) + "ms of " +
                                   std::to_string(cpu_burst_active[using_cpu.id]) + "ms burst");
                    }

                    int wait_time = elapsed_time - times_entered_q.front() - (t_cs / 2);
                    times_entered_q.pop();
                    if (using_cpu.is_cpu_bound) {
                        cpu_bound_wait_time += wait_time;
                    } else {
                        io_bound_wait_time += wait_time;
                    }
                    if (using_cpu.is_cpu_bound) {
                        num_cpu_switches++;
                    } else {
                        num_io_switches++;
                    }
                    time_cpu_frees = t_slc < using_cpu.bursts.front() ? t_slc + elapsed_time : using_cpu.bursts.front() + elapsed_time;
                    total_cpu_time += time_cpu_frees - elapsed_time;
                    using_cpu.bursts.front() -= t_slc;
                }

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

    print_line("Simulator ended for RR");

    cpu_util = (double)total_cpu_time / elapsed_time;
    cpu_turn = (double)cpu_bound_turnaround_time / num_cpu_switches;
    io_turn = (double)io_bound_turnaround_time / num_io_switches;

    cpu_wait = (double)cpu_bound_wait_time / num_cpu_switches;
    io_wait = (double)io_bound_wait_time / ((double) num_io_switches / 2);

    cpu_bursts_in_slice = (double) cpu_one_slice / (double) cpu_bursts_count;
    io_bursts_in_slice = (double) io_one_slice / (double) io_bursts_count;
    total_in_slice = ((double) cpu_one_slice + io_one_slice) / ((double) cpu_bursts_count + io_bursts_count);
}


void rr::write_statistics(const std::string& filename) {
    // TODO: Fix bugs with data collection for this
    std::ofstream outfile(filename, std::ios::app);

    outfile << "Algorithm RR" << std::endl;
    outfile << "-- CPU utilization: " << std::fixed << std::setprecision(3) << (std::ceil(cpu_util * 100000) / 1000) << "%" << std::endl;
    outfile << "-- CPU-bound average wait time: " << std::fixed << std::setprecision(3) << cpu_wait << " ms" << std::endl;
    outfile << "-- I/O-bound average wait time: " << std::fixed << std::setprecision(3) << io_wait << " ms" << std::endl;
    outfile << "-- overall average wait time: " << std::fixed << std::setprecision(3) << ((cpu_wait + io_wait) / 2) << " ms" << std::endl;
    outfile << "-- CPU-bound average turnaround time: " << std::fixed << std::setprecision(3) << cpu_turn << " ms" << std::endl;
    outfile << "-- I/O-bound average turnaround time: " << std::fixed << std::setprecision(3) << io_turn << " ms" << std::endl;
    outfile << "-- overall average turnaround time: " << std::fixed << std::setprecision(3) << ((cpu_turn + io_turn) / 2) << " ms" << std::endl;
    outfile << "-- CPU-bound number of context switches: " << num_cpu_switches << std::endl;
    outfile << "-- I/O-bound number of context switches: " << num_io_switches << std::endl;
    outfile << "-- overall number of context switches: " << num_cpu_switches + num_io_switches << std::endl;
    outfile << "-- CPU-bound number of preemptions: " << cpu_preempt << std::endl;
    outfile << "-- I/O-bound  number of preemptions: " << io_preempt << std::endl;
    outfile << "-- overall number of preemptions: " << cpu_preempt + io_preempt << std::endl;
    outfile << "-- CPU-bound percentage of CPU bursts completed within one time slice: " << std::setprecision(3) << cpu_bursts_in_slice * 100 << "%\n";
    outfile << "-- I/O-bound percentage of CPU bursts completed within one time slice: " << std::setprecision(3) << io_bursts_in_slice * 100 << "%\n";
    outfile << "-- overall percentage of CPU bursts completed within one time slice: " << std::setprecision(3) << total_in_slice * 100 << "%\n";

//    outfile.flush();
    outfile.close();
}
