// Member 1 (Syed Saad Najam) - LIFO Stack on top of DynArray.
// Used for: menu navigation breadcrumbs (session history) and undo of admin actions.
#pragma once
#include "common/DynArray.h"

template <typename T>
class Stack {
    DynArray<T> items_;

public:
    void push(const T& v) { items_.push(v); }
    bool pop(T& out) { return items_.pop(out); }
    T* top() { return items_.empty() ? nullptr : &items_.back(); }
    bool empty() const { return items_.empty(); }
    int size() const { return items_.size(); }
    void clear() { items_.clear(); }
    const T& at(int i) const { return items_[i]; }  // 0 = bottom of the stack
};
