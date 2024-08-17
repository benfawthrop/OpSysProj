//
// Created by Benjamin Fawthrop on 8/10/24.
//

#include "srt.h"

std::string srt::get_queue_status() {
    std::string result = "[Q";
    if (q.empty()) {
        result.append(" empty]");
    } else {
        std::priority_queue<Process, std::vector<Process>, CompareTau> temp = q;
        while(!temp.empty()) {
            result.append(" " + temp.top().id);
            temp.pop();
        }
        result.append("]");
    }

    return result;
}

void srt::sim_and_print() {
    print_line("Simulator started for SRT");
}

void srt::write_statistics(const std::string &filename) {
    // TODO: Write this
}