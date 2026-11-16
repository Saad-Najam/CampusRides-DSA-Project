// Member 3 (Shamaem Saqib) - administrator dashboard.
#include <iostream>

#include "common/DynArray.h"
#include "common/Utils.h"
#include "member1_saad/Ui.h"

namespace {

void printUser(const User* u) {
    std::cout << u->email << " | " << u->name << " | " << (u->studentId.empty() ? "-" : u->studentId) << " | rating ";
    if (u->ratingCount) std::cout << u->avgRating();
    else std::cout << "n/a";
    std::cout << " | " << (u->isAdmin ? "ADMIN" : u->suspended ? "SUSPENDED" : "active") << "\n";
}

void showStats(Context& ctx) {
    PlatformStats s = ctx.admin.stats();
    ui::title("Platform overview");
    std::cout << "Users:               " << s.users << " (" << s.suspended << " suspended)\n"
              << "Rides open:          " << s.ridesOpen << "\n"
              << "Rides completed:     " << s.ridesCompleted << "\n"
              << "Rides cancelled:     " << s.ridesCancelled << "\n"
              << "Confirmed seats:     " << s.seatsConfirmed << "\n"
              << "Pending requests:    " << s.pendingRequests << "\n"
              << "Ratings submitted:   " << ctx.ratings.count() << "\n";
    ui::pause();
}

void searchUsers(Context& ctx) {
    DynArray<User*> found;
    ctx.admin.searchUsers(ui::ask("Search users (blank = all): "), found);
    std::cout << found.size() << " user(s):\n";
    for (int i = 0; i < found.size(); i++) printUser(found[i]);
    ui::pause();
}

void searchRides(Context& ctx) {
    DynArray<Ride*> found;
    ctx.admin.searchRides(ui::ask("Search rides by driver, zone, status, date, vehicle (blank = all): "), found);
    std::cout << found.size() << " ride(s):\n";
    for (int i = 0; i < found.size(); i++) printRideLine(ctx, found[i]);
    ui::pause();
}

}  // namespace

void adminMenu(Context& ctx) {
    NavScope nav(ctx, "Admin");
    for (;;) {
        ctx.save();
        ui::title("Administrator panel");
        printBreadcrumb(ctx);
        std::cout << "1. Platform overview\n2. Search / list users\n3. Search / list rides\n4. Suspend a user\n"
                     "5. Reinstate a user\n6. Cancel a ride\n7. Undo last action"
                  << (ctx.admin.canUndo() ? "" : " (nothing to undo)") << "\n0. Logout\n";
        int c = ui::askInt("> ", 0, 7);
        std::string err, msg;
        switch (c) {
            case 0: return;
            case 1: showStats(ctx); break;
            case 2: searchUsers(ctx); break;
            case 3: searchRides(ctx); break;
            case 4: {
                std::string e = ui::ask("Email of user to suspend: ");
                if (ctx.admin.suspendUser(e, err)) std::cout << "User suspended. They can no longer log in.\n";
                else std::cout << err << "\n";
                break;
            }
            case 5: {
                std::string e = ui::ask("Email of user to reinstate: ");
                if (ctx.admin.unsuspendUser(e, err)) std::cout << "User reinstated.\n";
                else std::cout << err << "\n";
                break;
            }
            case 6: {
                int id = ui::askInt("Ride number to cancel: ", 1, 1000000);
                if (ctx.admin.cancelRide(id, err)) std::cout << "Ride cancelled and participants notified.\n";
                else std::cout << err << "\n";
                break;
            }
            case 7:
                ctx.admin.undoLast(msg);
                std::cout << msg << "\n";
                break;
        }
    }
}
