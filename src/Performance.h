#pragma once

extern class Performance perf;

class Performance {
    uint32_t loops = 0;
    uint32_t msTimeStart;   // ms

public:
    void init();
    void loop() { loops++; }
    void print();
    void memory_logStats();

};

