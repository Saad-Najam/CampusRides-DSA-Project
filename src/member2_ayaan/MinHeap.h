// Member 2 (Ayaan Kashif) - Binary Min-Heap / Priority Queue.
// Less(a, b) must return true when `a` has HIGHER priority than `b` (i.e. should come out first).
// Used for: ranking ride matches, listing upcoming rides by departure time, Dijkstra's frontier.
// O(log n) push / pop.
#pragma once
#include "common/DynArray.h"

template <typename T, typename Less>
class MinHeap {
    DynArray<T> a_;
    Less less_;

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (!less_(a_[i], a_[p])) break;
            T tmp = a_[i]; a_[i] = a_[p]; a_[p] = tmp;
            i = p;
        }
    }

    void siftDown(int i) {
        int n = a_.size();
        for (;;) {
            int l = 2 * i + 1, r = l + 1, best = i;
            if (l < n && less_(a_[l], a_[best])) best = l;
            if (r < n && less_(a_[r], a_[best])) best = r;
            if (best == i) break;
            T tmp = a_[i]; a_[i] = a_[best]; a_[best] = tmp;
            i = best;
        }
    }

public:
    MinHeap() {}
    explicit MinHeap(Less l) : less_(l) {}

    void push(const T& v) {
        a_.push(v);
        siftUp(a_.size() - 1);
    }

    bool pop(T& out) {
        if (a_.empty()) return false;
        out = a_[0];
        a_[0] = a_[a_.size() - 1];
        T dummy;
        a_.pop(dummy);
        if (!a_.empty()) siftDown(0);
        return true;
    }

    const T* top() const { return a_.empty() ? nullptr : &a_[0]; }
    bool empty() const { return a_.empty(); }
    int size() const { return a_.size(); }
};
