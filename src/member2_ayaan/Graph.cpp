#include "member2_ayaan/Graph.h"

#include "common/Utils.h"
#include "member2_ayaan/MinHeap.h"
#include "member3_shamaem/Queue.h"

namespace {
struct QItem {
    int dist = 0;
    int node = 0;
};
struct QLess {
    bool operator()(const QItem& a, const QItem& b) const { return a.dist < b.dist; }
};
}  // namespace

Graph::~Graph() {
    for (int i = 0; i < adj_.size(); i++) {
        Edge* e = adj_[i];
        while (e) {
            Edge* next = e->next;
            delete e;
            e = next;
        }
    }
}

int Graph::addZone(const std::string& name) {
    std::string key = util::toLower(util::trim(name));
    int* existing = index_.find(key);
    if (existing) return *existing;
    int v = names_.size();
    names_.push(util::trim(name));
    adj_.push(nullptr);
    index_.insert(key, v);
    return v;
}

int Graph::indexOf(const std::string& name) const {
    int* p = index_.find(util::toLower(util::trim(name)));
    return p ? *p : -1;
}

bool Graph::addEdge(int from, int to, int km) {
    if (from < 0 || to < 0 || from >= names_.size() || to >= names_.size() || km <= 0 || from == to) return false;
    for (Edge* e = adj_[from]; e; e = e->next) {
        if (e->to == to) {  // keep the shorter of parallel roads
            if (km < e->weight) e->weight = km;
            return true;
        }
    }
    Edge* e = new Edge{to, km, adj_[from]};
    adj_[from] = e;
    return true;
}

bool Graph::addRoad(const std::string& a, const std::string& b, int km) {
    int x = addZone(a), y = addZone(b);
    return addEdge(x, y, km) && addEdge(y, x, km);
}

bool Graph::hasEdge(int from, int to) const {
    if (from < 0 || from >= names_.size()) return false;
    for (Edge* e = adj_[from]; e; e = e->next)
        if (e->to == to) return true;
    return false;
}

void Graph::dijkstra(int src, DynArray<int>& dist, DynArray<int>* prev) const {
    int n = names_.size();
    dist.clear();
    for (int i = 0; i < n; i++) dist.push(INF);
    if (prev) {
        prev->clear();
        for (int i = 0; i < n; i++) prev->push(-1);
    }
    if (src < 0 || src >= n) return;
    MinHeap<QItem, QLess> pq;
    dist[src] = 0;
    QItem s;
    s.dist = 0;
    s.node = src;
    pq.push(s);
    QItem cur;
    while (pq.pop(cur)) {
        if (cur.dist > dist[cur.node]) continue;  // stale entry
        for (Edge* e = adj_[cur.node]; e; e = e->next) {
            int nd = cur.dist + e->weight;
            if (nd < dist[e->to]) {
                dist[e->to] = nd;
                if (prev) (*prev)[e->to] = cur.node;
                QItem q;
                q.dist = nd;
                q.node = e->to;
                pq.push(q);
            }
        }
    }
}

int Graph::shortestPath(int src, int dst, DynArray<int>* path) const {
    if (src < 0 || dst < 0 || src >= names_.size() || dst >= names_.size()) return -1;
    DynArray<int> dist, prev;
    dijkstra(src, dist, &prev);
    if (dist[dst] >= INF) return -1;
    if (path) {
        path->clear();
        for (int v = dst; v != -1; v = prev[v]) path->push(v);
        // reverse in place
        for (int i = 0, j = path->size() - 1; i < j; i++, j--) {
            int t = (*path)[i];
            (*path)[i] = (*path)[j];
            (*path)[j] = t;
        }
    }
    return dist[dst];
}

bool Graph::reachable(int src, int dst) const {
    int n = names_.size();
    if (src < 0 || dst < 0 || src >= n || dst >= n) return false;
    DynArray<bool> seen;
    for (int i = 0; i < n; i++) seen.push(false);
    Queue<int> q;
    q.enqueue(src);
    seen[src] = true;
    int v;
    while (q.dequeue(v)) {
        if (v == dst) return true;
        for (Edge* e = adj_[v]; e; e = e->next) {
            if (!seen[e->to]) {
                seen[e->to] = true;
                q.enqueue(e->to);
            }
        }
    }
    return false;
}

int Graph::pathDistance(const DynArray<std::string>& route, int i, int j) const {
    int total = 0;
    for (int k = i; k < j; k++) {
        int d = shortestPath(indexOf(route[k]), indexOf(route[k + 1]));
        if (d < 0) return -1;
        total += d;
    }
    return total;
}
