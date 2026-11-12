// Member 3 (Shamaem Saqib) - Administrative oversight with undo.
// Every reversible action (suspend / unsuspend / cancel ride) is pushed on a Stack so the admin
// can undo the most recent one (LIFO).
#pragma once
#include <string>

#include "common/DynArray.h"
#include "member1_saad/Stack.h"
#include "member1_saad/UserManager.h"
#include "member2_ayaan/RideManager.h"
#include "member3_shamaem/RequestService.h"

enum class AdminActionType { Suspend, Unsuspend, CancelRide };

struct AdminAction {
    AdminActionType type = AdminActionType::Suspend;
    std::string email;  // user actions
    int rideId = 0;     // ride actions
};

struct PlatformStats {
    int users = 0, suspended = 0;
    int ridesOpen = 0, ridesCompleted = 0, ridesCancelled = 0;
    int seatsConfirmed = 0, pendingRequests = 0;
};

class AdminPanel {
    UserManager& users_;
    RideManager& rides_;
    RequestService& requests_;
    Stack<AdminAction> undo_;

public:
    AdminPanel(UserManager& u, RideManager& r, RequestService& q) : users_(u), rides_(r), requests_(q) {}

    bool suspendUser(const std::string& email, std::string& err);
    bool unsuspendUser(const std::string& email, std::string& err);
    bool cancelRide(int rideId, std::string& err);
    // Reverts the most recent action; `msg` describes what was undone (or why it failed).
    bool undoLast(std::string& msg);
    bool canUndo() const { return !undo_.empty(); }

    // Case-insensitive substring search over every field.
    void searchUsers(const std::string& term, DynArray<User*>& out) const;
    void searchRides(const std::string& term, DynArray<Ride*>& out) const;

    PlatformStats stats() const;
};
