// Member 1 (Syed Saad Najam) - Hash Table with separate chaining.
// String keys, auto-resize at load factor 0.75, O(1) average insert / find / remove.
// Used for: user lookup by email and student ID, ride lookup by id, zone-name lookup in the graph.
#pragma once
#include <string>

#include "common/Utils.h"

template <typename V>
class HashTable {
    struct Node {
        std::string key;
        V value;
        Node* next;
        Node(const std::string& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    Node** buckets_;
    int cap_;
    int size_;

    int indexFor(const std::string& key, int cap) const { return (int)(util::hash64(key) % (unsigned long long)cap); }

    void rehash() {
        int newCap = cap_ * 2;
        Node** nb = new Node*[newCap]();
        for (int i = 0; i < cap_; i++) {
            Node* n = buckets_[i];
            while (n) {
                Node* next = n->next;
                int idx = indexFor(n->key, newCap);
                n->next = nb[idx];
                nb[idx] = n;
                n = next;
            }
        }
        delete[] buckets_;
        buckets_ = nb;
        cap_ = newCap;
    }

public:
    explicit HashTable(int initialCap = 16) : buckets_(new Node*[initialCap]()), cap_(initialCap), size_(0) {}
    ~HashTable() {
        clear();
        delete[] buckets_;
    }
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    // Returns false (and leaves the table untouched) if the key already exists.
    bool insert(const std::string& key, const V& value) {
        if (find(key)) return false;
        if (size_ + 1 > cap_ * 3 / 4) rehash();
        int idx = indexFor(key, cap_);
        Node* n = new Node(key, value);
        n->next = buckets_[idx];
        buckets_[idx] = n;
        size_++;
        return true;
    }

    V* find(const std::string& key) const {
        for (Node* n = buckets_[indexFor(key, cap_)]; n; n = n->next)
            if (n->key == key) return &n->value;
        return nullptr;
    }

    bool contains(const std::string& key) const { return find(key) != nullptr; }

    bool remove(const std::string& key) {
        int idx = indexFor(key, cap_);
        Node* prev = nullptr;
        for (Node* n = buckets_[idx]; n; prev = n, n = n->next) {
            if (n->key == key) {
                if (prev) prev->next = n->next;
                else buckets_[idx] = n->next;
                delete n;
                size_--;
                return true;
            }
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < cap_; i++) {
            Node* n = buckets_[i];
            while (n) {
                Node* next = n->next;
                delete n;
                n = next;
            }
            buckets_[i] = nullptr;
        }
        size_ = 0;
    }

    int size() const { return size_; }
    int bucketCount() const { return cap_; }

    // f(const std::string& key, V& value)
    template <typename F>
    void forEach(F f) const {
        for (int i = 0; i < cap_; i++)
            for (Node* n = buckets_[i]; n; n = n->next) f(n->key, n->value);
    }
};
