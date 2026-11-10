// Member 1 (Syed Saad Najam) - Application context shared by all UI flows.
// Owns every manager, the logged-in session, and the navigation Stack.
#pragma once
#include <string>

#include "member1_saad/Stack.h"
#include "member1_saad/UserManager.h"
#include "member2_ayaan/RideManager.h"
#include "member3_shamaem/AdminPanel.h"
#include "member3_shamaem/RatingSystem.h"
#include "member3_shamaem/RequestService.h"

enum class Role { Passenger, Driver };

struct Context {
    UserManager users;
    RideManager rides;
    RequestService requests;
    RatingSystem ratings;
    AdminPanel admin;

    std::string dataDir;
    User* current = nullptr;
    Role role = Role::Passenger;
    Stack<std::string> nav;  // breadcrumb trail of the menus the user is inside

    explicit Context(const std::string& dir)
        : requests(users, rides), ratings(users, rides), admin(users, rides, requests), dataDir(dir) {}

    void load();   // users -> rides -> social data (notifications, bookings, ratings, ...)
    void save();
};

// RAII helper: pushes a menu name on entry and pops it on exit (even when leaving via exception).
class NavScope {
    Context& ctx_;

public:
    NavScope(Context& c, const std::string& name) : ctx_(c) { ctx_.nav.push(name); }
    ~NavScope() {
        std::string tmp;
        ctx_.nav.pop(tmp);
    }
};

void printBreadcrumb(const Context& ctx);
