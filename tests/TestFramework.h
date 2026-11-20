// Tiny assertion framework shared by the three test files.
#pragma once
#include <iostream>

struct TestStats {
    int passed = 0;
    int failed = 0;
};
extern TestStats g_stats;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (cond) g_stats.passed++;                                                   \
        else {                                                                        \
            g_stats.failed++;                                                         \
            std::cout << "    FAIL " << __FILE__ << ":" << __LINE__ << "  " << #cond << "\n"; \
        }                                                                             \
    } while (0)

#define SECTION(name) std::cout << "  - " << name << "\n"

void runSaadTests();
void runAyaanTests();
void runShamaemTests();
