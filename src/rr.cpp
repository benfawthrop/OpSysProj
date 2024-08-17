// Created by Benjamin Fawthrop, Jimmy Wang on 8/10/24.

#include "rr.h"
#include <iostream>
#include <queue>
#include <iomanip>
#include <algorithm>

rr::rr(std::vector<Process>& processes, int t_cs, int t_slice)
        : processes(processes), t_cs(t_cs), t_slice(t_slice) {
    simulate();
}

void rr::printQueue(std::queue<Process*> readyQueue) {
    if (readyQueue.empty()) {
        std::cout << "[Q empty]";
    } else {
        std::cout << "[Q";
        std::queue<Process*> tempQueue = readyQueue;
        while (!tempQueue.empty()) {
            std::cout << " " << tempQueue.front()->id;
            tempQueue.pop();
        }
        std::cout << "]";
    }
}

void rr::simulate() {
    std::queue<Process*> readyQueue;
    int currentTime = 0;
    int contextSwitches = 0;
    int preemptions = 0;

    std::cout << "time 0ms: Simulator started for RR [Q empty]\n";

    size_t processIndex = 0;
    Process* currentProcess = nullptr;
    int timeRemaining = 0;

    while (!readyQueue.empty() || processIndex < processes.size() || currentProcess != nullptr) {
        // Check if a new process arrives
        while (processIndex < processes.size() && processes[processIndex].arrival_time <= currentTime) {
            Process* newProcess = &processes[processIndex++];
            readyQueue.push(newProcess);
            std::cout << "time " << currentTime << "ms: Process " << newProcess->id << " arrived; added to ready queue ";
            printQueue(readyQueue);
            std::cout << "\n";
        }

        // If no process is currently using the CPU, take the next one from the queue
        if (currentProcess == nullptr && !readyQueue.empty()) {
            currentProcess = readyQueue.front();
            readyQueue.pop();

            timeRemaining = std::min(t_slice, currentProcess->bursts[0]);
            std::cout << "time " << currentTime << "ms: Process " << currentProcess->id
                      << " started using the CPU for " << timeRemaining << "ms burst ";
            printQueue(readyQueue);
            std::cout << "\n";
        }

        if (currentProcess != nullptr) {
            // Process the time slice or the remaining burst time
            currentTime += timeRemaining;
            currentProcess->bursts[0] -= timeRemaining;

            if (currentProcess->bursts[0] > 0) {
                // Time slice expired but burst not completed, preempt process
                std::cout << "time " << currentTime << "ms: Time slice expired; preempting process "
                          << currentProcess->id << " with " << currentProcess->bursts[0] << "ms remaining ";
                printQueue(readyQueue);
                std::cout << "\n";
                readyQueue.push(currentProcess);
                preemptions++;
            } else {
                // Burst completed
                currentProcess->bursts.erase(currentProcess->bursts.begin());
                if (!currentProcess->bursts.empty()) {
                    // More bursts remaining, process goes to I/O
                    std::cout << "time " << currentTime << "ms: Process " << currentProcess->id
                              << " completed a CPU burst; " << currentProcess->bursts.size() / 2 << " bursts to go ";
                    printQueue(readyQueue);
                    std::cout << "\n";
                    std::cout << "time " << currentTime << "ms: Process " << currentProcess->id
                              << " switching out of CPU; blocking on I/O until time "
                              << (currentTime + currentProcess->bursts[0]) << "ms ";
                    printQueue(readyQueue);
                    std::cout << "\n";

                    // Update process arrival time after I/O
                    currentProcess->arrival_time = currentTime + currentProcess->bursts[0];
                    readyQueue.push(currentProcess);
                } else {
                    // No bursts remaining, process terminates
                    std::cout << "time " << currentTime << "ms: Process " << currentProcess->id << " terminated ";
                    printQueue(readyQueue);
                    std::cout << "\n";
                }
            }

            currentProcess = nullptr;  // CPU becomes idle

            if (!readyQueue.empty() || processIndex < processes.size()) {
                currentTime += t_cs / 2;  // Context switch out time
                contextSwitches++;
            }
        } else {
            // CPU idle, find the next event (next process arrival or next process returning from I/O)
            int nextEventTime = (processIndex < processes.size()) ? processes[processIndex].arrival_time : INT_MAX;
            if (currentProcess == nullptr && !readyQueue.empty()) {
                nextEventTime = std::min(nextEventTime, readyQueue.front()->arrival_time);
            }

            // If there is a next event, move time to it
            if (nextEventTime > currentTime) {
                currentTime = nextEventTime;
            } else {
                currentTime++;
            }
        }
    }

    std::cout << "time " << currentTime << "ms: Simulator ended for RR [Q empty]\n";
}
