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
};

#endif //OPSYSPROJ_PROCESS_H
