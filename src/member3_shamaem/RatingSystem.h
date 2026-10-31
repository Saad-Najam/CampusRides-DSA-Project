// Member 3 (Shamaem Saqib) - Mutual 1-5 star ratings after a completed ride.
// Every participant (driver + confirmed passengers) may rate every other participant once per ride.
// User rating totals are always rebuilt from the records, so ratings.csv is the single source of truth.
#pragma once
#include <string>

#include "common/DynArray.h"
#include "member1_saad/HashTable.h"
#include "member1_saad/UserManager.h"
#include "member2_ayaan/RideManager.h"
#include "member3_shamaem/LinkedList.h"

struct RatingRecord {
    int rideId = 0;
    std::string rater;   // emails
    std::string ratee;
    int stars = 0;
    std::string comment;
    long long ts = 0;
};

struct PendingRating {
    int rideId = 0;
    std::string ratee;
};

class RatingSystem {
    UserManager& users_;
    RideManager& rides_;
    LinkedList<RatingRecord> records_;
    HashTable<bool> seen_;  // "rideId|rater|ratee" -> already rated

    static std::string keyOf(int rideId, const std::string& rater, const std::string& ratee);

public:
    RatingSystem(UserManager& u, RideManager& r) : users_(u), rides_(r) {}

    bool rate(int rideId, const std::string& rater, const std::string& ratee, int stars, const std::string& comment,
              std::string& err);
    // Used by the file loader: records the rating and updates the ratee's totals.
    void restore(const RatingRecord& rec);

    // People `email` still has to rate across all completed rides.
    void pendingFor(const std::string& email, DynArray<PendingRating>& out) const;

    int count() const { return records_.size(); }
    template <typename F>
    void forEach(F f) const {  // f(const RatingRecord&)
        records_.forEach(f);
    }
};
