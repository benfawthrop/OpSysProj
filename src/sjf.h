#ifndef OPSYSPROJ_SJF_H
#define OPSYSPROJ_SJF_H

#include "process.h"
#include <vector>
#include <string>

class sjf {
public:
    sjf(std::vector<Process> processes, int context_time, double alpha);

    void simulate();

private:
    std::vector<Process> processes;
    std::vector<Process> ready_queue;  // Ready queue for SJF
    int context_time;
    double alpha;  // Alpha value for tau recalculation
    int elapsed_time;

    static bool CompareTau(const Process &a, const Process &b);

    void print_event(int time, const std::string &event, const std::vector<Process> &ready_queue);
};

#endif // OPSYSPROJ_SJF_H
