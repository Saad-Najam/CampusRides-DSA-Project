// Member 2 (Ayaan Kashif) - Route-aware matching between a passenger trip and driver routes.
//
// A ride matches when the driver's route visits a stop near the passenger's pickup, followed
// later by a stop near the passenger's drop-off. "Near" is measured with graph shortest paths:
// an exact stop costs 0 km of walking; otherwise the passenger walks/auto-rides to the closest
// stop within `maxWalkKm`. Matches are ranked by a MinHeap: least walking first, then earliest
// departure.
#pragma once
#include <string>

#include "common/DynArray.h"
#include "member2_ayaan/RideManager.h"

struct MatchResult {
    Ride* ride = nullptr;
    int pickupIdx = -1;   // index into ride->route where the passenger boards
    int dropIdx = -1;     // index into ride->route where the passenger alights
    int walkKm = 0;       // extra distance the passenger covers to reach those stops
    int sharedKm = 0;     // distance travelled together on the driver's route
};

namespace RideMatcher {

// Fills `out` and returns true if `ride` serves pickup -> dropoff.
bool matchRide(const Graph& g, const Ride& ride, const std::string& pickup, const std::string& dropoff,
               int maxWalkKm, MatchResult& out);

// Searches rides departing in [lo, hi] (pass -1/-1 for any time). Only open rides with a free seat
// that do not belong to `excludeDriver` are considered. Results are best-first.
void search(const RideManager& rm, const std::string& pickup, const std::string& dropoff, long long lo,
            long long hi, int maxWalkKm, const std::string& excludeDriver, DynArray<MatchResult>& out);

}  // namespace RideMatcher
