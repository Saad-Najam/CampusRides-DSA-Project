// Member 3 (Shamaem Saqib) - FIFO Queue built on our LinkedList.
// Used for: the per-ride seat request queue (requests are served in arrival order).
#pragma once
#include "member3_shamaem/LinkedList.h"

template <typename T>
class Queue {
    LinkedList<T> list_;

public:
    void enqueue(const T& v) { list_.pushBack(v); }
    bool dequeue(T& out) { return list_.popFront(out); }
    T* peek() { return list_.front(); }
    int size() const { return list_.size(); }
    bool empty() const { return list_.empty(); }
    void clear() { list_.clear(); }

    // Remove the first element matching pred (e.g. a passenger cancelling a pending request).
    template <typename Pred>
    bool removeIf(Pred pred) {
        return list_.removeFirst(pred);
    }

    template <typename F>
    void forEach(F f) const {
        list_.forEach(f);
    }
};
