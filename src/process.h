//
// Created by Benjamin Fawthrop on 8/10/24.
//

#ifndef OPSYSPROJ_PROCESS_H
#define OPSYSPROJ_PROCESS_H

#include <string>
#include <vector>

class Process {
public:
    std::string id; // PID
    int arrival_time;
    std::vector<int> bursts; // burst times
    bool is_cpu_bound;
    double tau; // added this for sjf and srt
    int remaining_time;  // Remaining time for the current CPU burst

    Process() : id(""), arrival_time(0), tau(0), remaining_time(0) {}

    Process(const std::string& id, const std::vector<int>& bursts, int arrival_time, int tau)
            : id(id), arrival_time(arrival_time), bursts(bursts), tau(tau), remaining_time(0) {}

    bool operator==(const Process &other) const {
        return this->id == other.id;
    }
};

//Process& Process::operator=(const Process &other) {
//    // Check for self-assignment
//    if (this == &other)
//        return *this; // Return *this to deal with self-assignment
//
//    // Copy the data from 'other' to 'this'
//    id = other.id;
//    arrival_time = other.arrival_time;
//    bursts = other.bursts; // std::vector has its own copy assignment operator
//    is_cpu_bound = other.is_cpu_bound;
//
//    return *this; // Return *this to allow for chaining
//}

#endif //OPSYSPROJ_PROCESS_H
