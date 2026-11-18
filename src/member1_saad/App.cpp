// Member 1 (Syed Saad Najam) - top-level menus: home screen and the logged-in dashboard.
#include <iostream>

#include "common/DynArray.h"
#include "common/Utils.h"
#include "member1_saad/Ui.h"

namespace {

enum Action {
    A_SWITCH_ROLE, A_SEARCH, A_BROWSE, A_UPCOMING, A_MY_BOOKINGS, A_POST, A_MANAGE, A_CHAT,
    A_INBOX, A_RATINGS, A_HISTORY, A_PROFILE, A_LOGOUT
};

void dashboard(Context& ctx) {
    NavScope nav(ctx, ctx.current->name);
    for (;;) {
        ctx.save();  // persist after every action
        bool driver = ctx.role == Role::Driver;
        ui::title(std::string("Dashboard - ") + (driver ? "DRIVER" : "PASSENGER") + " mode");
        printBreadcrumb(ctx);
        int unread = ctx.current->unreadCount();

        DynArray<std::string> labels;
        DynArray<int> actions;
        labels.push(driver ? "Switch to Passenger mode" : "Switch to Driver mode"); actions.push(A_SWITCH_ROLE);
        if (driver) {
            labels.push("Post a ride"); actions.push(A_POST);
            labels.push("Manage my rides (requests, complete, cancel)"); actions.push(A_MANAGE);
        } else {
            labels.push("Search rides for my route"); actions.push(A_SEARCH);
            labels.push("Browse ride feed (next / previous)"); actions.push(A_BROWSE);
            labels.push("Upcoming rides / find by date"); actions.push(A_UPCOMING);
            labels.push("My requests & bookings"); actions.push(A_MY_BOOKINGS);
        }
        labels.push("Ride chat"); actions.push(A_CHAT);
        labels.push("Notifications" + (unread ? " (" + util::toStr(unread) + " new)" : std::string())); actions.push(A_INBOX);
        labels.push("Ratings & reviews"); actions.push(A_RATINGS);
        labels.push("Ride history"); actions.push(A_HISTORY);
        labels.push("My profile"); actions.push(A_PROFILE);
        labels.push("Logout"); actions.push(A_LOGOUT);

        for (int i = 0; i < labels.size(); i++) std::cout << (i + 1) << ". " << labels[i] << "\n";
        int c = ui::askInt("> ", 1, labels.size());
        switch (actions[c - 1]) {
            case A_SWITCH_ROLE: ctx.role = driver ? Role::Passenger : Role::Driver; break;
            case A_SEARCH: searchRidesFlow(ctx); break;
            case A_BROWSE: browseFeedFlow(ctx); break;
            case A_UPCOMING: upcomingRidesFlow(ctx); break;
            case A_MY_BOOKINGS: passengerBookingsFlow(ctx); break;
            case A_POST: postRideFlow(ctx); break;
            case A_MANAGE: driverRidesFlow(ctx); break;
            case A_CHAT: chatFlow(ctx); break;
            case A_INBOX: inboxFlow(ctx); break;
            case A_RATINGS: ratingsFlow(ctx); break;
            case A_HISTORY: historyFlow(ctx); break;
            case A_PROFILE: profileMenu(ctx); break;
            case A_LOGOUT:
                ctx.current = nullptr;
                return;
        }
    }
}

}  // namespace

void runApp(Context& ctx) {
    NavScope home(ctx, "Home");
    for (;;) {
        ctx.save();
        ui::title("CAMPUS RIDES - NUCES Karachi carpooling");
        std::cout << "1. Login\n2. Register\n0. Exit\n";
        int c = ui::askInt("> ", 0, 2);
        if (c == 0) return;
        if (c == 2) {
            registerFlow(ctx);
        } else if (loginFlow(ctx)) {
            if (ctx.current->isAdmin) adminMenu(ctx);
            else dashboard(ctx);
            ctx.current = nullptr;
        }
    }
}
