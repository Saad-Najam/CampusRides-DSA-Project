#include "member3_shamaem/AdminPanel.h"

#include "common/Utils.h"
#include "member3_shamaem/NotificationService.h"

bool AdminPanel::suspendUser(const std::string& email, std::string& err) {
    User* u = users_.findByEmail(email);
    if (u && u->suspended) { err = "User is already suspended."; return false; }
    if (!users_.setSuspended(email, true, err)) return false;
    AdminAction a;
    a.type = AdminActionType::Suspend;
    a.email = users_.findByEmail(email)->email;
    undo_.push(a);
    return true;
}

bool AdminPanel::unsuspendUser(const std::string& email, std::string& err) {
    User* u = users_.findByEmail(email);
    if (u && !u->suspended) { err = "User is not suspended."; return false; }
    if (!users_.setSuspended(email, false, err)) return false;
    AdminAction a;
    a.type = AdminActionType::Unsuspend;
    a.email = users_.findByEmail(email)->email;
    undo_.push(a);
    return true;
}

bool AdminPanel::cancelRide(int rideId, std::string& err) {
    Ride* r = rides_.findRide(rideId);
    if (!r) { err = "Ride not found."; return false; }
    if (!requests_.cancelRide(r, "", true, err)) return false;
    AdminAction a;
    a.type = AdminActionType::CancelRide;
    a.rideId = rideId;
    undo_.push(a);
    return true;
}

bool AdminPanel::undoLast(std::string& msg) {
    AdminAction a;
    if (!undo_.pop(a)) { msg = "Nothing to undo."; return false; }
    std::string err;
    switch (a.type) {
        case AdminActionType::Suspend:
            if (!users_.setSuspended(a.email, false, err)) { msg = err; return false; }
            msg = "Undid suspension of " + a.email + ".";
            return true;
        case AdminActionType::Unsuspend:
            if (!users_.setSuspended(a.email, true, err)) { msg = err; return false; }
            msg = "Undid un-suspension of " + a.email + " (suspended again).";
            return true;
        case AdminActionType::CancelRide:
            if (!requests_.reopenRide(rides_.findRide(a.rideId), err)) { msg = err; return false; }
            msg = "Undid cancellation of ride #" + util::toStr(a.rideId) + ".";
            return true;
    }
    return false;
}

void AdminPanel::searchUsers(const std::string& term, DynArray<User*>& out) const {
    out.clear();
    users_.forEach([&](User* u) {
        if (term.empty() || util::containsIgnoreCase(u->email, term) || util::containsIgnoreCase(u->name, term) ||
            util::containsIgnoreCase(u->studentId, term) || util::containsIgnoreCase(u->phone, term) ||
            util::containsIgnoreCase(u->vehicle, term) ||
            (u->suspended && util::containsIgnoreCase("suspended", term)) ||
            (u->isAdmin && util::containsIgnoreCase("admin", term)))
            out.push(u);
    });
    out.sort([](User* const& a, User* const& b) { return a->email < b->email; });
}

void AdminPanel::searchRides(const std::string& term, DynArray<Ride*>& out) const {
    out.clear();
    rides_.forEachRide([&](Ride* r) {
        if (term.empty() || util::containsIgnoreCase(r->driver, term) || util::containsIgnoreCase(r->routeString(), term) ||
            util::containsIgnoreCase(statusName(r->status), term) || util::containsIgnoreCase(r->vehicle, term) ||
            util::containsIgnoreCase(r->notes, term) || util::containsIgnoreCase(util::formatDateTime(r->departKey), term) ||
            util::containsIgnoreCase("#" + util::toStr(r->id), term))
            out.push(r);
    });
}

PlatformStats AdminPanel::stats() const {
    PlatformStats s;
    users_.forEach([&](User* u) {
        s.users++;
        if (u->suspended) s.suspended++;
    });
    rides_.forEachRide([&](Ride* r) {
        if (r->status == RideStatus::Open) s.ridesOpen++;
        else if (r->status == RideStatus::Completed) s.ridesCompleted++;
        else s.ridesCancelled++;
        s.seatsConfirmed += r->bookings.size();
        s.pendingRequests += r->requests.size();
    });
    return s;
}
