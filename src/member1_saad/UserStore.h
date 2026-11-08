// Member 1 (Syed Saad Najam) - CSV persistence for users (users.csv).
// Format: email,studentId,name,passHash,phone,vehicle,isAdmin,suspended
// Rating totals are NOT stored here; RatingSystem rebuilds them from ratings.csv on load.
#pragma once
#include <string>

#include "member1_saad/UserManager.h"

namespace UserStore {
bool save(const UserManager& users, const std::string& path);
bool load(UserManager& users, const std::string& path);  // false if file missing
}  // namespace UserStore
