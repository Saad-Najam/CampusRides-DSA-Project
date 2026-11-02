#include "member3_shamaem/RatingSystem.h"

#include "common/Utils.h"
#include "member3_shamaem/NotificationService.h"

std::string RatingSystem::keyOf(int rideId, const std::string& rater, const std::string& ratee) {
    return util::toStr(rideId) + "|" + rater + "|" + ratee;
}

bool RatingSystem::rate(int rideId, const std::string& rater, const std::string& ratee, int stars,
                        const std::string& comment, std::string& err) {
    Ride* ride = rides_.findRide(rideId);
    if (!ride) { err = "Ride not found."; return false; }
    if (ride->status != RideStatus::Completed) { err = "You can only rate after the ride is completed."; return false; }
    if (rater == ratee) { err = "You cannot rate yourself."; return false; }
    if (!ride->isParticipant(rater) || !ride->isParticipant(ratee)) {
        err = "Both people must have taken part in this ride.";
        return false;
    }
    if (stars < 1 || stars > 5) { err = "Rating must be between 1 and 5 stars."; return false; }
    if (seen_.contains(keyOf(rideId, rater, ratee))) { err = "You have already rated this person for this ride."; return false; }

    RatingRecord rec;
    rec.rideId = rideId;
    rec.rater = rater;
    rec.ratee = ratee;
    rec.stars = stars;
    rec.comment = comment;
    rec.ts = util::nowKey();
    restore(rec);
    User* r = users_.findByEmail(rater);
    NotificationService::pushTo(users_, ratee,
                                (r ? r->name : rater) + " rated you " + util::toStr(stars) + "/5 for ride #" + util::toStr(rideId) + ".");
    return true;
}

void RatingSystem::restore(const RatingRecord& rec) {
    if (!seen_.insert(keyOf(rec.rideId, rec.rater, rec.ratee), true)) return;
    records_.pushBack(rec);
    User* u = users_.findByEmail(rec.ratee);
    if (u) {
        u->ratingSum += rec.stars;
        u->ratingCount++;
    }
}

void RatingSystem::pendingFor(const std::string& email, DynArray<PendingRating>& out) const {
    out.clear();
    rides_.forEachRide([&](Ride* r) {
        if (r->status != RideStatus::Completed || !r->isParticipant(email)) return;
        DynArray<std::string> others;
        if (r->driver != email) others.push(r->driver);
        for (int i = 0; i < r->bookings.size(); i++)
            if (r->bookings[i].passenger != email) others.push(r->bookings[i].passenger);
        for (int i = 0; i < others.size(); i++) {
            if (seen_.contains(keyOf(r->id, email, others[i]))) continue;
            PendingRating p;
            p.rideId = r->id;
            p.ratee = others[i];
            out.push(p);
        }
    });
}
