// Test driver: runs every member's tests. Exit code is non-zero if any check failed.
#include "TestFramework.h"

TestStats g_stats;

int main() {
    std::cout << "== Member 1 (Saad): HashTable, Stack, Users, Auth, UserStore ==\n";
    runSaadTests();
    std::cout << "== Member 2 (Ayaan): Graph, MinHeap, AVL/BST, DoublyLinkedList, Rides, Matching ==\n";
    runAyaanTests();
    std::cout << "== Member 3 (Shamaem): Queue, LinkedList, Requests, Ratings, Admin, Persistence ==\n";
    runShamaemTests();
    std::cout << "\nPassed: " << g_stats.passed << "   Failed: " << g_stats.failed << "\n";
    return g_stats.failed == 0 ? 0 : 1;
}
