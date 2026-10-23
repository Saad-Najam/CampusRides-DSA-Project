// Minimal growable array (our own replacement for std::vector). Shared by all modules.
#pragma once

template <typename T>
class DynArray {
    T* data_;
    int size_;
    int cap_;

    void grow(int minCap) {
        int nc = cap_ == 0 ? 8 : cap_ * 2;
        while (nc < minCap) nc *= 2;
        T* nd = new T[nc];
        for (int i = 0; i < size_; i++) nd[i] = data_[i];
        delete[] data_;
        data_ = nd;
        cap_ = nc;
    }

public:
    DynArray() : data_(nullptr), size_(0), cap_(0) {}
    DynArray(const DynArray& o) : data_(nullptr), size_(0), cap_(0) {
        if (o.size_ > 0) grow(o.size_);
        for (int i = 0; i < o.size_; i++) data_[i] = o.data_[i];
        size_ = o.size_;
    }
    DynArray& operator=(const DynArray& o) {
        if (this == &o) return *this;
        size_ = 0;
        if (o.size_ > cap_) grow(o.size_);
        for (int i = 0; i < o.size_; i++) data_[i] = o.data_[i];
        size_ = o.size_;
        return *this;
    }
    ~DynArray() { delete[] data_; }

    void push(const T& v) {
        if (size_ == cap_) {
            T copy = v;  // v may alias an element
            grow(size_ + 1);
            data_[size_++] = copy;
        } else {
            data_[size_++] = v;
        }
    }
    bool pop(T& out) {
        if (size_ == 0) return false;
        out = data_[--size_];
        return true;
    }
    void removeAt(int i) {
        for (int k = i; k + 1 < size_; k++) data_[k] = data_[k + 1];
        size_--;
    }
    void clear() { size_ = 0; }
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    T& operator[](int i) { return data_[i]; }
    const T& operator[](int i) const { return data_[i]; }
    T& back() { return data_[size_ - 1]; }

    // Linear search with operator==, -1 if absent.
    int indexOf(const T& v) const {
        for (int i = 0; i < size_; i++)
            if (data_[i] == v) return i;
        return -1;
    }

    // Stable insertion sort; fine for the list sizes shown in menus.
    template <typename Less>
    void sort(Less less) {
        for (int i = 1; i < size_; i++) {
            T key = data_[i];
            int j = i - 1;
            while (j >= 0 && less(key, data_[j])) {
                data_[j + 1] = data_[j];
                j--;
            }
            data_[j + 1] = key;
        }
    }
};
