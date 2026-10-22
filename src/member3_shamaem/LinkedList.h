// Member 3 (Shamaem Saqib) - Singly Linked List.
// Used for: per-user notification queues, ride history, ride chat, rating records.
// O(1) pushBack / pushFront / popFront (tail pointer is kept).
#pragma once

template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& d) : data(d), next(nullptr) {}
    };

    LinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    ~LinkedList() { clear(); }
    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    void pushBack(const T& v) {
        Node* n = new Node(v);
        if (tail_) tail_->next = n;
        else head_ = n;
        tail_ = n;
        size_++;
    }

    void pushFront(const T& v) {
        Node* n = new Node(v);
        n->next = head_;
        head_ = n;
        if (!tail_) tail_ = n;
        size_++;
    }

    bool popFront(T& out) {
        if (!head_) return false;
        Node* n = head_;
        out = n->data;
        head_ = n->next;
        if (!head_) tail_ = nullptr;
        delete n;
        size_--;
        return true;
    }

    T* front() { return head_ ? &head_->data : nullptr; }
    const T* front() const { return head_ ? &head_->data : nullptr; }

    // Removes the first element satisfying pred. Returns true if one was removed.
    template <typename Pred>
    bool removeFirst(Pred pred) {
        Node* prev = nullptr;
        for (Node* cur = head_; cur; prev = cur, cur = cur->next) {
            if (pred(cur->data)) {
                if (prev) prev->next = cur->next;
                else head_ = cur->next;
                if (cur == tail_) tail_ = prev;
                delete cur;
                size_--;
                return true;
            }
        }
        return false;
    }

    void clear() {
        while (head_) {
            Node* n = head_;
            head_ = head_->next;
            delete n;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    template <typename F>
    void forEach(F f) {
        for (Node* n = head_; n; n = n->next) f(n->data);
    }
    template <typename F>
    void forEach(F f) const {
        for (const Node* n = head_; n; n = n->next) f(n->data);
    }

    int size() const { return size_; }
    bool empty() const { return size_ == 0; }
    Node* head() const { return head_; }

private:
    Node* head_;
    Node* tail_;
    int size_;
};
