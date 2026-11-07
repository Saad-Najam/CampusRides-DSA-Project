// Member 2 (Ayaan Kashif) - CSV persistence for rides (rides.csv).
// Format: id,driver,route(zones joined by '>'),departKey,totalSeats,seatsAvailable,vehicle,notes,status
// Bookings, pending requests and chat are stored by SocialStore (Member 3).
#pragma once
#include <string>

#include "member2_ayaan/RideManager.h"

namespace RideStore {
bool save(const RideManager& rm, const std::string& path);
bool load(RideManager& rm, const std::string& path);  // false if file missing
}  // namespace RideStore
