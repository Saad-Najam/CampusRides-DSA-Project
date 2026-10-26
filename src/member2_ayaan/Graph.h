// Member 2 (Ayaan Kashif) - Directed weighted Graph (adjacency list).
// Vertices are Karachi city zones; edge weights are road distances in km.
// Provides Dijkstra shortest paths (MinHeap) and BFS reachability (Queue), used for
// route suggestion and route-overlap detection between drivers and passengers.
#pragma once
#include <string>

#include "common/DynArray.h"
#include "member1_saad/HashTable.h"

class Graph {
public:
    struct Edge {
        int to;
        int weight;
        Edge* next;
    };

    enum { INF = 1000000000 };

    Graph() {}
    ~Graph();
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;

    int addZone(const std::string& name);                 // returns the vertex index (existing or new)
    int indexOf(const std::string& name) const;           // case-insensitive, -1 if unknown
    const std::string& nameOf(int v) const { return names_[v]; }
    int zoneCount() const { return names_.size(); }

    bool addEdge(int from, int to, int km);               // directed
    bool addRoad(const std::string& a, const std::string& b, int km);  // adds both directions
    bool hasEdge(int from, int to) const;
    const Edge* edgesOf(int v) const { return adj_[v]; }

    // Dijkstra from src; dist[v] == INF if unreachable. prev[v] = predecessor or -1.
    void dijkstra(int src, DynArray<int>& dist, DynArray<int>* prev = nullptr) const;
    // Returns the shortest distance or -1 if unreachable. Optionally fills the vertex path src..dst.
    int shortestPath(int src, int dst, DynArray<int>* path = nullptr) const;
    bool reachable(int src, int dst) const;               // BFS

    // Sum of shortest distances along route[i] -> route[i+1] -> ... -> route[j] (route holds zone names).
    int pathDistance(const DynArray<std::string>& route, int i, int j) const;

private:
    DynArray<std::string> names_;
    DynArray<Edge*> adj_;
    HashTable<int> index_;  // lower-case name -> vertex
};
