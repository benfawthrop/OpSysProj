#ifndef OPSYSPROJ_RR_H
#define OPSYSPROJ_RR_H

#include <vector>
#include <queue>
#include "process.h"

class rr {
private:
    std::vector<Process> processes;
    int t_cs; // context switch time
    int t_slice; // time slice for RR

    void printQueue(std::queue<Process*> readyQueue);

    // Function to simulate the RR scheduling
    void simulate();

public:
    rr(std::vector<Process>& processes, int t_cs, int t_slice);
};

#endif //OPSYSPROJ_RR_H
