#include "fcfs.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>


void fcfs::sim_and_print() {
    print_line("Simulator started for FCFS");
    std::map<int, Process> io_bound_map; // stores all io bound processes keyed by their completion time
    std::priority_queue<int, std::vector<int>, std::greater<int> > io_bound_map_keys; // stores times for each io bound process completion
    bool cpu_free = true; // flag if the CPU is free to use
    int i = 0; // makes sure all processes are added
    Process using_cpu; // process that is currently holding the CPU
    int time_cpu_frees = -1; // -1 if cpu is free, int
    int processes_killed = 0; // counts how many processes we've killed

    // while there are processes alive
    while ( processes_killed < processes.size() ) {
        bool did_something = false; // sees if we can time skip
        // check if we still need to add any processes and if they are in time
        if ( i < processes.size() ) {
            // adds all processes 1 at a time
            int curr_arrival = processes[i].arrival_time; // arrival time of the soonest process
            // if adding the process corresponds to current time
            if (curr_arrival >= elapsed_time && (time_cpu_frees == -1 || curr_arrival <= time_cpu_frees) &&
                (io_bound_map_keys.empty() || curr_arrival <= io_bound_map_keys.top())) {

                // adds the process
                q.push(processes[i]);
                elapsed_time = processes[i].arrival_time;
                if (elapsed_time <= 9999) {
                    // prints if allowed
                    print_line("Process " + processes[i].id + " arrived; added to the ready queue");
                }
                i++;
                did_something = true;
            }
        }

        // checks to see if we can send a process to CPU
        if ( cpu_free && !q.empty()) {
            cpu_free = false;
            elapsed_time += context_switch_time / 2;
            using_cpu = q.front();
            time_cpu_frees = using_cpu.bursts.front() + elapsed_time;
            q.pop();
            if (elapsed_time <= 9999) {
                // prints if allowed
                print_line("Process " + using_cpu.id + " started using the CPU for " +
                           std::to_string(using_cpu.bursts.front()) + "ms burst");
            }
            using_cpu.bursts.erase(using_cpu.bursts.begin()); // remove burst just used
            did_something = true;
        }

        // if we need to do another operation this iteration
        if (!did_something || time_cpu_frees == elapsed_time ||
                (!io_bound_map_keys.empty() && io_bound_map_keys.top() == elapsed_time)) {
            if ( time_cpu_frees > 0 && time_cpu_frees <= (!io_bound_map_keys.empty() ? io_bound_map_keys.top() : time_cpu_frees) ) {
                // cpu event happens first
                elapsed_time = time_cpu_frees;
                time_cpu_frees = -1;
                cpu_free = true;
                int burst = using_cpu.bursts.front();
                // output if allowed
                if ( elapsed_time <= 9999 ) {
                    print_line("Process " + using_cpu.id + " completed a CPU burst; " +
                            std::to_string((using_cpu.bursts.size() / 2)) + " bursts to go");
                    if (using_cpu.bursts.size() > 0) {
                        print_line("Process " + using_cpu.id + " switching out of CPU; blocking on I/O until time " +
                                   std::to_string(elapsed_time + burst + (context_switch_time / 2)) + "ms");
                    }
                }
                // terminates if done with last CPU burst
                if ( using_cpu.bursts.size() == 0 ) {
                    print_line("Process " + using_cpu.id + " terminated" );
                    processes_killed++;
                } else {
                    // add our io bound process to the map for that ordered by time
                    using_cpu.bursts.erase(using_cpu.bursts.begin());

                    io_bound_map[elapsed_time + burst + (context_switch_time / 2)] = using_cpu;
                    io_bound_map_keys.push(elapsed_time + burst + (context_switch_time / 2));
                }
                elapsed_time += context_switch_time / 2;

            } else if ( io_bound_map_keys.size() > 0 && (time_cpu_frees >= io_bound_map_keys.top() || time_cpu_frees == -1) ) {
                // IO event happens first
                elapsed_time = io_bound_map_keys.top();
                q.push(io_bound_map[elapsed_time]);
                if ( elapsed_time <= 9999) {
                    print_line("Process " + io_bound_map[elapsed_time].id + " completed I/O; added to ready queue");
                }
                io_bound_map.erase(elapsed_time);
                io_bound_map_keys.pop();
            }
        }
    } // while
    print_line("Simulator ended for FCFS");
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