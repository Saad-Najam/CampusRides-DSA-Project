// Member 2 (Ayaan Kashif) - Doubly Linked List.
// Used for: the ordered ride feed (sorted by departure time) with next/previous browsing.
#pragma once

template <typename T>
class DoublyLinkedList {
public:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    DoublyLinkedList() : head_(nullptr), tail_(nullptr), size_(0) {}
    ~DoublyLinkedList() { clear(); }
    DoublyLinkedList(const DoublyLinkedList&) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList&) = delete;

    Node* pushBack(const T& v) {
        Node* n = new Node(v);
        n->prev = tail_;
        if (tail_) tail_->next = n;
        else head_ = n;
        tail_ = n;
        size_++;
        return n;
    }

    Node* pushFront(const T& v) {
        Node* n = new Node(v);
        n->next = head_;
        if (head_) head_->prev = n;
        else tail_ = n;
        head_ = n;
        size_++;
        return n;
    }

    // Inserts before the first node that `less(v, node->data)` says is strictly later, keeping the list
    // sorted and stable. Scans from the tail because new rides are usually posted for later dates.
    template <typename Less>
    Node* insertSorted(const T& v, Less less) {
        Node* cur = tail_;
        while (cur && less(v, cur->data)) cur = cur->prev;
        // insert after `cur` (or at the front if cur is null)
        if (!cur) return pushFront(v);
        if (cur == tail_) return pushBack(v);
        Node* n = new Node(v);
        n->prev = cur;
        n->next = cur->next;
        cur->next->prev = n;
        cur->next = n;
        size_++;
        return n;
    }

    void remove(Node* n) {
        if (!n) return;
        if (n->prev) n->prev->next = n->next;
        else head_ = n->next;
        if (n->next) n->next->prev = n->prev;
        else tail_ = n->prev;
        delete n;
        size_--;
    }

    template <typename Pred>
    Node* findIf(Pred pred) const {
        for (Node* n = head_; n; n = n->next)
            if (pred(n->data)) return n;
        return nullptr;
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
    void forEach(F f) const {
        for (Node* n = head_; n; n = n->next) f(n->data);
    }
    template <typename F>
    void forEachReverse(F f) const {
        for (Node* n = tail_; n; n = n->prev) f(n->data);
    }

    Node* head() const { return head_; }
    Node* tail() const { return tail_; }
    int size() const { return size_; }
    bool empty() const { return size_ == 0; }

private:
    Node* head_;
    Node* tail_;
    int size_;
};
