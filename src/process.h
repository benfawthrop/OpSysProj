//
// Created by Benjamin Fawthrop on 8/10/24.
//

#ifndef OPSYSPROJ_PROCESS_H
#define OPSYSPROJ_PROCESS_H

class Process {
public:
    std::string id; // PID
    int arrival_time;
    std::vector<int> bursts; // burst times
    bool is_cpu_bound;

//    Process& operator=(const Process &other);
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
