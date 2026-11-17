#include "member2_ayaan/RideMatcher.h"

#include "member2_ayaan/MinHeap.h"

namespace {
struct MatchLess {  // true when a should be shown before b
    bool operator()(const MatchResult& a, const MatchResult& b) const {
        if (a.walkKm != b.walkKm) return a.walkKm < b.walkKm;
        if (a.ride->departKey != b.ride->departKey) return a.ride->departKey < b.ride->departKey;
        return a.ride->id < b.ride->id;
    }
};
}  // namespace

namespace RideMatcher {

bool matchRide(const Graph& g, const Ride& ride, const std::string& pickup, const std::string& dropoff,
               int maxWalkKm, MatchResult& out) {
    int p = g.indexOf(pickup), d = g.indexOf(dropoff);
    if (p < 0 || d < 0 || p == d || ride.route.size() < 2) return false;

    // distance from the passenger's pickup to every zone, and from every route stop to the drop-off
    DynArray<int> fromPickup;
    g.dijkstra(p, fromPickup);
    DynArray<int> toDrop;
    for (int j = 0; j < ride.route.size(); j++) {
        int d2 = g.shortestPath(g.indexOf(ride.route[j]), d);
        toDrop.push(d2 < 0 ? Graph::INF : d2);
    }

    int bestCost = Graph::INF, bi = -1, bj = -1;
    for (int i = 0; i < ride.route.size(); i++) {
        int walkIn = fromPickup[g.indexOf(ride.route[i])];
        if (walkIn >= Graph::INF) continue;
        for (int j = i + 1; j < ride.route.size(); j++) {
            if (toDrop[j] >= Graph::INF) continue;
            int cost = walkIn + toDrop[j];
            if (cost < bestCost) {
                bestCost = cost;
                bi = i;
                bj = j;
            }
        }
    }
    if (bi < 0 || bestCost > maxWalkKm) return false;

    out.ride = const_cast<Ride*>(&ride);
    out.pickupIdx = bi;
    out.dropIdx = bj;
    out.walkKm = bestCost;
    int shared = g.pathDistance(ride.route, bi, bj);
    out.sharedKm = shared < 0 ? 0 : shared;
    return true;
}

void search(const RideManager& rm, const std::string& pickup, const std::string& dropoff, long long lo,
            long long hi, int maxWalkKm, const std::string& excludeDriver, DynArray<MatchResult>& out) {
    out.clear();
    MinHeap<MatchResult, MatchLess> heap;

    DynArray<Ride*> candidates;
    if (lo >= 0 && hi >= 0) rm.rangeByTime(lo, hi, candidates);   // BST range query
    else rm.forEachRide([&](Ride* r) { candidates.push(r); });     // whole feed

    for (int i = 0; i < candidates.size(); i++) {
        Ride* r = candidates[i];
        if (r->status != RideStatus::Open || r->seatsAvailable <= 0) continue;
        if (!excludeDriver.empty() && r->driver == excludeDriver) continue;
        MatchResult m;
        if (matchRide(rm.map(), *r, pickup, dropoff, maxWalkKm, m)) heap.push(m);
    }
    MatchResult m;
    while (heap.pop(m)) out.push(m);
}

}  // namespace RideMatcher
