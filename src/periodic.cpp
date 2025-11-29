#include "periodic.h"

#include <cstdio>

void periodic() {
    static uint32_t loops = 0;
    if (++loops % 100 == 0) { // print about once per second
        std::printf("Periodic function called (%lu)\n", static_cast<unsigned long>(loops));
    }
}
