#ifndef OPSYSPROJ_SJF_H
#define OPSYSPROJ_SJF_H

#include "process.h"
#include <vector>
#include <string>

class sjf {
public:
    sjf(std::vector<Process> processes, int context_time, double alpha, double lambda);

    void simulate();

private:
    std::vector<Process> processes;
    std::vector<Process> ready_queue;  // Ready queue for SJF
    int context_time;
    double alpha;  // Alpha value for tau recalculation
    double lambda; // Lambda value for tau recalculation
    int elapsed_time;

    static bool CompareTau(const Process &a, const Process &b);
    double calculate_new_tau(double old_tau, int actual_burst, double alpha, double lambda);

    void print_event(int time, const std::string &event, const std::vector<Process> &ready_queue);

    // Declare processHasId as a private member function
    bool processHasId(const Process& p, const std::string& id);
};

#endif // OPSYSPROJ_SJF_H
