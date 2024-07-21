//
// Created by Benjamin Fawthrop on 7/21/24.
// g++ -Wall -Werror -std=c++11 -o main main.cpp
//./main 3 1 32 0.001 1024
//
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iomanip>
#include <fstream>

// process object to make referring to specific processes simpler
struct Process {
    std::string id; // PID
    int arrival_time;
    std::vector<int> bursts; // burst times
};

/*
 * args:
 *      argc -> # of args
 *      argv -> arguments
 *      The rest of the arguments are passed by reference so that main gets the arguments aswell
 *
 * Output:
 *      none, just changes the args given by reference
 */
void parse_arguments(int argc, char* argv[], int &n, int &ncpu, int &seed, double &lambda, int &bound) {
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

double drand48() {
    // generates a random double precision floating-point number between 0 and 1.
    return static_cast<double>(std::rand()) / RAND_MAX;
}

void srand48(long seed) {
    // initializes seed for predictable output
    std::srand(seed);
}

double next_exp(double lambda) {
    // generates a random number from an exponential distribution with rate parameter lambda
    return -std::log(drand48()) / lambda;
}

/*
 *
 */
std::vector<Process> generate_processes(int n, int ncpu, double lambda, int bound) {
    std::vector<Process> processes;
    char process_id[3] = "A0";

    for (int i = 0; i < n; ++i) {
        Process p;
        p.id = process_id;
        p.arrival_time = std::floor(next_exp(lambda));

        int cpu_bursts_count = std::ceil(drand48() * 32);
        for (int j = 0; j < cpu_bursts_count; ++j) {
            int cpu_burst = std::ceil(next_exp(lambda));
            if (i < ncpu) {
                cpu_burst *= 4;
            }
            p.bursts.push_back(cpu_burst);

            if (j < cpu_bursts_count - 1) {
                int io_burst = std::ceil(next_exp(lambda)) * 8;
                if (i < ncpu) {
                    io_burst /= 8;
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
 *
 */
void print_processes(const std::vector<Process>& processes, int n, int ncpu, int seed, double lambda, int bound) {
    std::cout << "<<< PROJECT PART I" << std::endl;
    std::cout << "<<< -- process set (n=" << n << ") with " << ncpu << " CPU-bound process" << (ncpu > 1 ? "es" : "") << std::endl;
    std::cout << "<<< -- seed=" << seed << "; lambda=" << std::fixed << std::setprecision(6) << lambda << "; bound=" << bound << std::endl;

    for (const auto& p : processes) {
        bool is_cpu_bound = (p.bursts[0] % 4 == 0);  // crude way to identify CPU-bound processes
        std::cout << (is_cpu_bound ? "CPU-bound" : "I/O-bound") << " process " << p.id << ": arrival time " << p.arrival_time << "ms; " << (p.bursts.size() / 2) << " CPU bursts:" << std::endl;

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
 *
 */
void write_statistics(const std::vector<Process>& processes, const std::string& filename) {
    int num_processes = processes.size();
    int num_cpu_bound = std::count_if(processes.begin(), processes.end(), [](const Process& p) { return p.bursts[0] % 4 == 0; });
    int num_io_bound = num_processes - num_cpu_bound;

    double total_cpu_burst_time = 0;
    double total_io_burst_time = 0;

    for (const auto& p : processes) {
        for (size_t i = 0; i < p.bursts.size(); i += 2) {
            total_cpu_burst_time += p.bursts[i];
            if (i + 1 < p.bursts.size()) {
                total_io_burst_time += p.bursts[i + 1];
            }
        }
    }

    double avg_cpu_burst_time = total_cpu_burst_time / num_processes;
    double avg_io_burst_time = total_io_burst_time / num_processes;

    std::ofstream out(filename);
    out << "-- number of processes: " << num_processes << std::endl;
    out << "-- number of CPU-bound processes: " << num_cpu_bound << std::endl;
    out << "-- number of I/O-bound processes: " << num_io_bound << std::endl;
    out << "-- CPU-bound average CPU burst time: " << std::fixed << std::setprecision(3) << avg_cpu_burst_time << " ms" << std::endl;
    out << "-- I/O-bound average CPU burst time: " << std::fixed << std::setprecision(3) << avg_io_burst_time << " ms" << std::endl;
    out << "-- overall average CPU burst time: " << std::fixed << std::setprecision(3) << avg_cpu_burst_time << " ms" << std::endl;
    out << "-- CPU-bound average I/O burst time: " << std::fixed << std::setprecision(3) << avg_io_burst_time << " ms" << std::endl;
    out << "-- I/O-bound average I/O burst time: " << std::fixed << std::setprecision(3) << avg_io_burst_time << " ms" << std::endl;
    out << "-- overall average I/O burst time: " << std::fixed << std::setprecision(3) << avg_io_burst_time << " ms" << std::endl;
    out.close();
}

int main(int argc, char* argv[]) {
    int n, ncpu, seed, bound;
    double lambda;

    parse_arguments(argc, argv, n, ncpu, seed, lambda, bound);
    srand48(seed);

    std::vector<Process> processes = generate_processes(n, ncpu, lambda, bound);

    print_processes(processes, n, ncpu, seed, lambda, bound);
    write_statistics(processes, "simout.txt");

    return 0;
}

