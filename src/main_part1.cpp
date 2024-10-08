//
// Created by Benjamin Fawthrop, Ricky Wang, Jimmy Wang on 7/21/24.
// g++ -Wall -Werror -o main *.cpp -lm
// ./main 3 1 32 0.001 1024
// ./main 8 6 512 0.001 1024
// ./main 16 2 256 0.001 2048
// ./main 20 12 128 0.01 4096
//

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "rng.h"

// process object to make referring to specific processes simpler
struct Process {
    std::string id; // PID
    int arrival_time;
    std::vector<int> bursts; // burst times
    bool is_cpu_bound;
};

// Functor to check if a process is CPU-bound
struct IsCpuBound {
    bool operator()(const Process& p) const {
        return p.is_cpu_bound;
    }
};

/*
 * In charge of parsing arguments via reference arguments
 *
 * ARGUMENTS:
 *  argc -> number of command line args
 *  argv -> command line args
 *
 *  *(argv + 1) -> n, the # of processes to simulate
 *                PID are given as A0, A1, A2, . . ., A9, B0, B1, . . ., Z9
 *  *(argv + 2) -> ncpu, # of CPU bound processes
 *
 *  *(argv + 3) -> seed, seed for random number gen
 *
 *  *(argv + 4) -> lambda, (1/lambda) represents the average number for
 *                the random number generator
 *
 *  *(argv + 5) -> bound, upper bound for random numbers
 */
void parse_arguments(int argc, char** argv, int &n, int &ncpu, int &seed, double &lambda, int &bound) {
    // error checking
    if (argc != 6) {
        std::cerr << "ERROR: Incorrect number of arguments" << std::endl;
        std::exit(1);
    }

    // error checking arg types
    try {
        n = std::stoi(argv[1]);
        ncpu = std::stoi(argv[2]);
        seed = std::stoi(argv[3]);
        lambda = std::stod(argv[4]);
        bound = std::stoi(argv[5]);
    } catch (std::exception &e) {
        std::cerr << "ERROR: Invalid argument type" << std::endl;
        std::exit(1);
    }
}

//functions for pseudo random number generator
//
//double drand48() {
//    // generates a random double precision floating-point number between 0 and 1.
//    return static_cast<double>(std::rand()) / RAND_MAX;
//}
//
//void srand48(long seed) {
//    // initializes seed for predictable output
//    std::srand(seed);
//}
//
double next_exp(RandomGenerator& rng, double lambda, int bound) {
    // generates a random number from an exponential distribution with rate parameter lambda
    // also uses the bound to make sure the returned value is valid
    double value;
    do {
        value = -std::log(rng.drand48()) / lambda;
    } while (value > bound);
    return value;
}

/*
 * generates a vector of processes based off of the parameters given in the command line args
 */
std::vector<Process> generate_processes(RandomGenerator& rng, int n, int ncpu, double lambda, int bound) {
    std::vector<Process> processes;
    char process_id[3] = "A0"; // init pid

    // iterates through the # of processes
    for (int i = 0; i < n; ++i) {
        Process p; // initialize process
        p.id = process_id;
        p.arrival_time = std::floor(next_exp(rng, lambda, bound));
        p.is_cpu_bound = false;

        int cpu_bursts_count = std::ceil(rng.drand48() * 32);
        // iterates by # of cpu bursts
        for (int j = 0; j < cpu_bursts_count; ++j) {
            int cpu_burst = std::ceil(next_exp(rng, lambda, bound));

            // if cpu bound
            if ( i < ncpu ) {
                cpu_burst *= 4; // as per doc
                p.is_cpu_bound = true;
            }

            p.bursts.push_back(cpu_burst);

            // adds an io burst for the cpu burst if it's not the last
            if (j < cpu_bursts_count - 1) {
                int io_burst = std::ceil(next_exp(rng, lambda, bound));

                if (!p.is_cpu_bound) {
                    // multiplies by 8 if it's an io burst bc those take longer
                    io_burst *= 8;
                }
                p.bursts.push_back(io_burst);
            }
        }

        processes.push_back(p);

        // Increment process ID
        if (process_id[1] == '9') {
            process_id[1] = '0';
            process_id[0]++;
        } else {
            process_id[1]++;
        }
    }

    return processes;
}

/*
 * manages the output to stdout of our random processes
 *
 * ARGUMENTS:
 *      processes -> vector of our generated processes
 *      n -> # of processes
 *      ncpu -> # of cpu bound processes
 *      seed -> seed for rnum generator
 *      lambda -> (1/lambda) represents the average number for
 *                the random number generator
 *      bound -> upper bound for rnums
 */
void print_processes(const std::vector<Process>& processes, int n, int ncpu, int seed, double lambda, int bound) {
    std::cout << "<<< PROJECT PART I" << std::endl;
    std::cout << "<<< -- process set (n=" << n << ") with " << ncpu << " CPU-bound process" <<
            (ncpu > 1 ? "es" : "") << std::endl;
    std::cout << "<<< -- seed=" << seed << "; lambda=" << std::fixed << std::setprecision(6) <<
            lambda << "; bound=" << bound << std::endl;

    // loops through our processes
    for (int j = 0 ; j < n ; j++) {
        const Process & p = processes[j];
        bool is_cpu_bound = p.is_cpu_bound;
        std::cout << (is_cpu_bound ? "CPU-bound" : "I/O-bound") << " process " << p.id << ": arrival time " <<
                  p.arrival_time << "ms; " << (p.bursts.size() / 2) + 1 << " CPU burst" <<
                  ((p.bursts.size() / 2) + 1 == 1 ? "" : "s") << ":" << std::endl;


        // loops through each process's bursts
        for (size_t i = 0; i < p.bursts.size(); i += 2) {
            std::cout << "==> CPU burst " << p.bursts[i] << "ms";
            if (i + 1 < p.bursts.size()) {
                std::cout << " ==> I/O burst " << p.bursts[i + 1] << "ms";
            }
            std::cout << std::endl;
        }
    }
}



/*
 * handles writing our outputs to filename based on described in the assignment
 *
 * ARGUMENTS:
 *      processes -> our generated processes
 *      filename -> name of the file to print to
 */
void write_statistics(const std::vector<Process>& processes, const std::string& filename) {
    int num_processes = processes.size();
    int num_cpu_bound = std::count_if(processes.begin(), processes.end(), IsCpuBound());
    int num_io_bound = num_processes - num_cpu_bound;

    double total_cpu_bound_cpu_burst_time = 0;
    double total_cpu_bound_io_burst_time = 0;
    int total_cpu_bound_cpu_bursts = 0;
    int total_cpu_bound_io_bursts = 0;

    double total_io_bound_cpu_burst_time = 0;
    double total_io_bound_io_burst_time = 0;
    int total_io_bound_cpu_bursts = 0;
    int total_io_bound_io_bursts = 0;

    // loops through all processes
    for (int j = 0 ; j < num_processes ; j++) {
        const Process & p = processes[j];
        bool is_cpu_bound = p.is_cpu_bound;
        // loops through all bursts
        for (size_t i = 0; i < p.bursts.size(); i += 2) {
            if (is_cpu_bound) {
                total_cpu_bound_cpu_burst_time += p.bursts[i];
                total_cpu_bound_cpu_bursts++;
                if (i + 1 < p.bursts.size()) {
                    total_cpu_bound_io_burst_time += p.bursts[i + 1];
                    total_cpu_bound_io_bursts++;
                }
            } else {
                total_io_bound_cpu_burst_time += p.bursts[i];
                total_io_bound_cpu_bursts++;
                if (i + 1 < p.bursts.size()) {
                    total_io_bound_io_burst_time += p.bursts[i + 1];
                    total_io_bound_io_bursts++;
                }
            }
        }
    }

    double avg_cpu_bound_cpu_burst_time = total_cpu_bound_cpu_bursts > 0 ?
            std::ceil((total_cpu_bound_cpu_burst_time / total_cpu_bound_cpu_bursts) * 1000.0) / 1000.0 : 0;
    double avg_io_bound_cpu_burst_time = total_io_bound_cpu_bursts > 0 ?
            std::ceil((total_io_bound_cpu_burst_time / total_io_bound_cpu_bursts) * 1000.0) / 1000.0 : 0;
    double overall_avg_cpu_burst_time = (total_cpu_bound_cpu_bursts + total_io_bound_cpu_bursts) > 0 ?
            std::ceil(((total_cpu_bound_cpu_burst_time + total_io_bound_cpu_burst_time) /
            (total_cpu_bound_cpu_bursts + total_io_bound_cpu_bursts)) * 1000.0) / 1000.0 : 0;

    double avg_cpu_bound_io_burst_time = total_cpu_bound_io_bursts > 0 ?
            std::ceil((total_cpu_bound_io_burst_time / total_cpu_bound_io_bursts) * 1000.0) / 1000.0 : 0;
    double avg_io_bound_io_burst_time = total_io_bound_io_bursts > 0 ?
            std::ceil((total_io_bound_io_burst_time / total_io_bound_io_bursts) * 1000.0) / 1000.0 : 0;
    double overall_avg_io_burst_time = (total_cpu_bound_io_bursts + total_io_bound_io_bursts) > 0 ?
            std::ceil(((total_cpu_bound_io_burst_time + total_io_bound_io_burst_time) /
            (total_cpu_bound_io_bursts + total_io_bound_io_bursts)) * 1000.0) / 1000.0 : 0;


    std::ofstream out(filename);
    out << "-- number of processes: " << num_processes << std::endl;
    out << "-- number of CPU-bound processes: " << num_cpu_bound << std::endl;
    out << "-- number of I/O-bound processes: " << num_io_bound << std::endl;
    out << "-- CPU-bound average CPU burst time: " << std::fixed << std::setprecision(3) <<
        avg_cpu_bound_cpu_burst_time << " ms" << std::endl;
    out << "-- I/O-bound average CPU burst time: " << std::fixed << std::setprecision(3) <<
        avg_io_bound_cpu_burst_time << " ms" << std::endl;
    out << "-- overall average CPU burst time: " << std::fixed << std::setprecision(3) <<
        overall_avg_cpu_burst_time << " ms" << std::endl;
    out << "-- CPU-bound average I/O burst time: " << std::fixed << std::setprecision(3) <<
        avg_cpu_bound_io_burst_time << " ms" << std::endl;
    out << "-- I/O-bound average I/O burst time: " << std::fixed << std::setprecision(3) <<
        avg_io_bound_io_burst_time << " ms" << std::endl;
    out << "-- overall average I/O burst time: " << std::fixed << std::setprecision(3) <<
        overall_avg_io_burst_time << " ms" << std::endl;
    out.close();
}

int main(int argc, char** argv) {
    int n, ncpu, seed, bound;
    double lambda;

    parse_arguments(argc, argv, n, ncpu, seed, lambda, bound);
    RandomGenerator rng(seed);

    std::vector<Process> processes = generate_processes(rng, n, ncpu, lambda, bound);

    print_processes(processes, n, ncpu, seed, lambda, bound);
    write_statistics(processes, "simout.txt");

    return 0;
}

