#pragma once
#include <iostream>

using namespace std;

bool profilerPrint = true;
int profilerThreshold = 0;

class Profiler {
public:
    string name;
    clock_t start_time;

    Profiler(string name_) {
        name = name_;
        start_time = clock();
    }

    ~Profiler() {
        clock_t now = clock();
        if (profilerPrint && (now - start_time > profilerThreshold)) {
            printf("<%s - %lums>\n", name.c_str(), now - start_time);
        }
    }
};