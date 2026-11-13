#include "member2_ayaan/RideManager.h"

#include "member2_ayaan/CampusMap.h"
#include "member2_ayaan/MinHeap.h"

namespace {
struct RidePtrLess {  // earlier departure first, ties by id
    bool operator()(Ride* const& a, Ride* const& b) const {
        if (a->departKey != b->departKey) return a->departKey < b->departKey;
        return a->id < b->id;
    }
};
}  // namespace

RideManager::RideManager() : nextId_(1) { buildCampusMap(map_); }

RideManager::~RideManager() {
    byId_.forEach([](const std::string&, Ride*& r) { delete r; });
}

Ride* RideManager::createRide(const std::string& driverEmail, const DynArray<std::string>& route, long long departKey,
                              int seats, const std::string& vehicle, const std::string& notes, std::string& err) {
    if (route.size() < 2) { err = "A route needs at least two stops."; return nullptr; }
    if (route.size() > 10) { err = "A route can have at most 10 stops."; return nullptr; }
    DynArray<std::string> canonical;
    for (int i = 0; i < route.size(); i++) {
        int v = map_.indexOf(route[i]);
        if (v < 0) { err = "Unknown zone: " + route[i]; return nullptr; }
        for (int j = 0; j < canonical.size(); j++)
            if (util::equalsIgnoreCase(canonical[j], map_.nameOf(v))) { err = "Zone repeated in route: " + route[i]; return nullptr; }
        canonical.push(map_.nameOf(v));
    }
    for (int i = 0; i + 1 < canonical.size(); i++) {
        if (!map_.reachable(map_.indexOf(canonical[i]), map_.indexOf(canonical[i + 1]))) {
            err = "No road connection between " + canonical[i] + " and " + canonical[i + 1] + ".";
            return nullptr;
        }
    }
    if (departKey < 0) { err = "Invalid departure time."; return nullptr; }
    if (seats < 1 || seats > 8) { err = "Seats must be between 1 and 8."; return nullptr; }

    Ride* r = new Ride();
    r->id = nextId_;
    r->driver = driverEmail;
    r->route = canonical;
    r->departKey = departKey;
    r->totalSeats = r->seatsAvailable = seats;
    r->vehicle = vehicle;
    r->notes = notes;
    adoptRide(r);
    return r;
}

bool RideManager::adoptRide(Ride* r) {
    if (!byId_.insert(util::toStr(r->id), r)) {
        delete r;
        return false;
    }
    RideKey k;
    k.departKey = r->departKey;
    k.id = r->id;
    k.ride = r;
    index_.insert(k);
    feed_.insertSorted(r, RidePtrLess());
    if (r->id >= nextId_) nextId_ = r->id + 1;
    return true;
}

Ride* RideManager::findRide(int id) const {
    Ride** p = byId_.find(util::toStr(id));
    return p ? *p : nullptr;
}

bool RideManager::deleteRide(int id) {
    Ride* r = findRide(id);
    if (!r) return false;
    RideKey k;
    k.departKey = r->departKey;
    k.id = r->id;
    index_.remove(k);
    DoublyLinkedList<Ride*>::Node* n = feed_.findIf([&](Ride* x) { return x == r; });
    feed_.remove(n);
    byId_.remove(util::toStr(id));
    delete r;
    return true;
}

void RideManager::rangeByTime(long long lo, long long hi, DynArray<Ride*>& out) const {
    out.clear();
    RideKey a, b;
    a.departKey = lo;
    a.id = 0;
    b.departKey = hi;
    b.id = 2147483647;
    index_.rangeQuery(a, b, [&](const RideKey& k) { out.push(k.ride); });
}

void RideManager::upcomingOpen(int limit, DynArray<Ride*>& out) const {
    out.clear();
    MinHeap<Ride*, RidePtrLess> heap;
    feed_.forEach([&](Ride* r) {
        if (r->status == RideStatus::Open) heap.push(r);
    });
    Ride* r;
    while ((limit <= 0 || out.size() < limit) && heap.pop(r)) out.push(r);
}

void RideManager::ridesOfDriver(const std::string& email, DynArray<Ride*>& out) const {
    out.clear();
    feed_.forEach([&](Ride* r) {
        if (r->driver == email) out.push(r);
    });
}

void RideManager::ridesOfParticipant(const std::string& email, DynArray<Ride*>& out) const {
    out.clear();
    feed_.forEach([&](Ride* r) {
        if (r->isParticipant(email)) out.push(r);
    });
}
