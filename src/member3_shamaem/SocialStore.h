// Member 3 (Shamaem Saqib) - CSV persistence for everything attached to users and rides:
//   notifications.csv  email,ts,read,text
//   history.csv        email,ts,rideId,role,coTravelers,route           (stored newest first)
//   ratings.csv        rideId,rater,ratee,stars,ts,comment
//   bookings.csv       rideId,passenger,pickup,dropoff
//   requests.csv       rideId,passenger,pickup,dropoff,ts               (queue order = file order)
//   messages.csv       rideId,ts,sender,text
// Load users and rides first, then call SocialStore::load.
#pragma once
#include <string>

#include "member1_saad/UserManager.h"
#include "member2_ayaan/RideManager.h"
#include "member3_shamaem/RatingSystem.h"

namespace SocialStore {
bool save(const UserManager& users, const RideManager& rides, const RatingSystem& ratings, const std::string& dir);
void load(UserManager& users, RideManager& rides, RatingSystem& ratings, const std::string& dir);
}  // namespace SocialStore
