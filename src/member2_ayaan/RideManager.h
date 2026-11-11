// Member 2 (Ayaan Kashif) - Owns all rides and their indexes.
//   HashTable        id -> Ride*                      O(1) lookup
//   AVLTree (BST)    (departure, id) -> Ride*         date/time search + range queries
//   DoublyLinkedList rides ordered by departure       feed browsing (next / previous)
//   MinHeap          built on demand                  "upcoming rides" earliest-first
//   Graph            campus road map                  routing and matching
#pragma once
#include <string>

#include "common/DynArray.h"
#include "member1_saad/HashTable.h"
#include "member2_ayaan/AVLTree.h"
#include "member2_ayaan/DoublyLinkedList.h"
#include "member2_ayaan/Graph.h"
#include "member2_ayaan/Ride.h"

struct RideKey {
    long long departKey = 0;
    int id = 0;
    Ride* ride = nullptr;
};

struct RideKeyLess {
    bool operator()(const RideKey& a, const RideKey& b) const {
        if (a.departKey != b.departKey) return a.departKey < b.departKey;
        return a.id < b.id;
    }
};

class RideManager {
public:
    RideManager();
    ~RideManager();
    RideManager(const RideManager&) = delete;
    RideManager& operator=(const RideManager&) = delete;

    Graph& map() { return map_; }
    const Graph& map() const { return map_; }

    // Validates and creates a ride. Returns nullptr and sets err on failure.
    Ride* createRide(const std::string& driverEmail, const DynArray<std::string>& route, long long departKey,
                     int seats, const std::string& vehicle, const std::string& notes, std::string& err);
    // Takes ownership of a fully built ride (file loader). Returns false on duplicate id.
    bool adoptRide(Ride* r);
    Ride* findRide(int id) const;
    bool deleteRide(int id);
    int count() const { return byId_.size(); }

    const DoublyLinkedList<Ride*>& feed() const { return feed_; }           // ascending by departure
    const AVLTree<RideKey, RideKeyLess>& index() const { return index_; }

    // Rides departing in [lo, hi] (inclusive keys) via the BST.
    void rangeByTime(long long lo, long long hi, DynArray<Ride*>& out) const;
    // Earliest-departing open rides, popped from a MinHeap. limit <= 0 means all.
    void upcomingOpen(int limit, DynArray<Ride*>& out) const;
    void ridesOfDriver(const std::string& email, DynArray<Ride*>& out) const;
    void ridesOfParticipant(const std::string& email, DynArray<Ride*>& out) const;

    template <typename F>
    void forEachRide(F f) const {  // f(Ride*), ascending by departure time
        feed_.forEach(f);
    }

private:
    HashTable<Ride*> byId_;
    AVLTree<RideKey, RideKeyLess> index_;
    DoublyLinkedList<Ride*> feed_;
    Graph map_;
    int nextId_;
};
