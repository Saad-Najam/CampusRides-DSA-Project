// Member 2 (Ayaan Kashif) - Binary Search Tree (self-balancing AVL variant).
// Used for: the ride index sorted by (departure time, ride id) -> date/time search and range queries.
// Plain BSTs degrade to O(n) when rides are inserted in chronological order; AVL rotations keep
// insert / remove / search at O(log n) and range queries at O(log n + k).
// Less(a, b) is a strict weak ordering; two values are "equal" if neither is less than the other.
#pragma once

template <typename T, typename Less>
class AVLTree {
    struct Node {
        T val;
        Node* l;
        Node* r;
        int h;
        explicit Node(const T& v) : val(v), l(nullptr), r(nullptr), h(1) {}
    };

    Node* root_;
    int size_;
    Less less_;

    static int height(Node* n) { return n ? n->h : 0; }
    static void update(Node* n) {
        int a = height(n->l), b = height(n->r);
        n->h = (a > b ? a : b) + 1;
    }
    static int balanceOf(Node* n) { return n ? height(n->l) - height(n->r) : 0; }

    static Node* rotateRight(Node* y) {
        Node* x = y->l;
        y->l = x->r;
        x->r = y;
        update(y);
        update(x);
        return x;
    }
    static Node* rotateLeft(Node* x) {
        Node* y = x->r;
        x->r = y->l;
        y->l = x;
        update(x);
        update(y);
        return y;
    }

    static Node* rebalance(Node* n) {
        update(n);
        int b = balanceOf(n);
        if (b > 1) {
            if (balanceOf(n->l) < 0) n->l = rotateLeft(n->l);
            return rotateRight(n);
        }
        if (b < -1) {
            if (balanceOf(n->r) > 0) n->r = rotateRight(n->r);
            return rotateLeft(n);
        }
        return n;
    }

    Node* insert(Node* n, const T& v, bool& added) {
        if (!n) {
            added = true;
            return new Node(v);
        }
        if (less_(v, n->val)) n->l = insert(n->l, v, added);
        else if (less_(n->val, v)) n->r = insert(n->r, v, added);
        else return n;  // duplicate
        return rebalance(n);
    }

    Node* remove(Node* n, const T& v, bool& removed) {
        if (!n) return nullptr;
        if (less_(v, n->val)) n->l = remove(n->l, v, removed);
        else if (less_(n->val, v)) n->r = remove(n->r, v, removed);
        else {
            removed = true;
            if (!n->l || !n->r) {
                Node* child = n->l ? n->l : n->r;
                delete n;
                return child;
            }
            Node* m = n->r;
            while (m->l) m = m->l;
            n->val = m->val;
            bool dummy = false;
            n->r = remove(n->r, m->val, dummy);
        }
        return rebalance(n);
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->l);
        destroy(n->r);
        delete n;
    }

    template <typename F>
    void inorder(Node* n, F& f) const {
        if (!n) return;
        inorder(n->l, f);
        f(n->val);
        inorder(n->r, f);
    }

    template <typename F>
    void range(Node* n, const T& lo, const T& hi, F& f) const {
        if (!n) return;
        bool geLo = !less_(n->val, lo);
        bool leHi = !less_(hi, n->val);
        if (geLo) range(n->l, lo, hi, f);
        if (geLo && leHi) f(n->val);
        if (leHi) range(n->r, lo, hi, f);
    }

public:
    AVLTree() : root_(nullptr), size_(0) {}
    ~AVLTree() { destroy(root_); }
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(const T& v) {
        bool added = false;
        root_ = insert(root_, v, added);
        if (added) size_++;
        return added;
    }

    bool remove(const T& v) {
        bool removed = false;
        root_ = remove(root_, v, removed);
        if (removed) size_--;
        return removed;
    }

    bool contains(const T& v) const {
        Node* n = root_;
        while (n) {
            if (less_(v, n->val)) n = n->l;
            else if (less_(n->val, v)) n = n->r;
            else return true;
        }
        return false;
    }

    // In-order (ascending) traversal.
    template <typename F>
    void forEach(F f) const { inorder(root_, f); }

    // Visits every value v with lo <= v <= hi in ascending order.
    template <typename F>
    void rangeQuery(const T& lo, const T& hi, F f) const { range(root_, lo, hi, f); }

    int size() const { return size_; }
    int height() const { return height(root_); }
    bool empty() const { return size_ == 0; }
};
