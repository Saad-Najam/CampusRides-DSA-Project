// Member 3 (Shamaem Saqib) - requests, ride management, chat, notifications, ratings, history.
#include <iostream>

#include "common/DynArray.h"
#include "common/Utils.h"
#include "member1_saad/Ui.h"
#include "member3_shamaem/NotificationService.h"

namespace {

std::string nameOf(const Context& ctx, const std::string& email) {
    User* u = ctx.users.findByEmail(email);
    return u ? u->name : email;
}

void showRideManagement(Context& ctx, Ride* r) {
    printRideDetails(ctx, r);
    std::cout << "\nConfirmed passengers (" << r->bookings.size() << "):\n";
    if (r->bookings.empty()) std::cout << "  none\n";
    for (int i = 0; i < r->bookings.size(); i++)
        std::cout << "  - " << nameOf(ctx, r->bookings[i].passenger) << ": " << r->bookings[i].pickup << " -> "
                  << r->bookings[i].dropoff << "\n";
    std::cout << "Pending requests, oldest first (" << r->requests.size() << "):\n";
    if (r->requests.empty()) std::cout << "  none\n";
    int pos = 1;
    r->requests.forEach([&](const SeatRequest& q) {
        std::cout << "  " << pos++ << ". " << nameOf(ctx, q.passenger) << ": " << q.pickup << " -> " << q.dropoff
                  << " (requested " << util::formatDateTime(q.ts) << ")\n";
    });
}

void manageRide(Context& ctx, Ride* r) {
    NavScope nav(ctx, "Ride #" + util::toStr(r->id));
    for (;;) {
        ui::title("Manage ride #" + util::toStr(r->id));
        printBreadcrumb(ctx);
        showRideManagement(ctx, r);
        std::cout << "\n1. Accept oldest request\n2. Decline oldest request\n3. Mark ride completed\n"
                     "4. Cancel ride\n5. Open ride chat\n0. Back\n";
        int c = ui::askInt("> ", 0, 5);
        std::string err;
        bool ok = true;
        if (c == 0) return;
        if (c == 1 || c == 2) {
            ok = ctx.requests.processNext(r, ctx.current, c == 1, err);
            if (ok) std::cout << (c == 1 ? "Request accepted." : "Request declined.") << "\n";
        } else if (c == 3) {
            if (ui::askYesNo("Mark this ride as completed?")) {
                ok = ctx.requests.completeRide(r, ctx.current, err);
                if (ok) std::cout << "Ride completed. Passengers can now rate you.\n";
            }
        } else if (c == 4) {
            if (ui::askYesNo("Cancel this ride and notify everyone?")) {
                ok = ctx.requests.cancelRide(r, ctx.current->email, false, err);
                if (ok) std::cout << "Ride cancelled.\n";
            }
        } else {
            chatFlow(ctx);
        }
        if (!ok) std::cout << err << "\n";
        ctx.save();
    }
}

void chatRoom(Context& ctx, Ride* r) {
    NavScope nav(ctx, "Chat #" + util::toStr(r->id));
    for (;;) {
        ui::title("Chat - ride #" + util::toStr(r->id) + " (" + r->routeString() + ")");
        if (r->chat.empty()) std::cout << "No messages yet.\n";
        r->chat.forEach([&](const ChatMessage& m) {
            std::cout << "[" << util::formatDateTime(m.ts) << "] " << nameOf(ctx, m.sender) << ": " << m.text << "\n";
        });
        std::string t = ui::ask("\nMessage (Enter to leave): ");
        if (t.empty()) return;
        std::string err;
        if (!ctx.requests.sendMessage(r, ctx.current, t, err)) std::cout << err << "\n";
        else ctx.save();
    }
}

}  // namespace

void passengerBookingsFlow(Context& ctx) {
    NavScope nav(ctx, "My bookings");
    for (;;) {
        ui::title("My requests & bookings");
        printBreadcrumb(ctx);
        DynArray<Ride*> mine;
        const std::string& me = ctx.current->email;
        ctx.rides.forEachRide([&](Ride* r) {
            if (r->bookingIndex(me) >= 0 || r->hasPendingRequest(me)) mine.push(r);
        });
        if (mine.empty()) {
            std::cout << "You have no requests or bookings yet.\n";
            return;
        }
        for (int i = 0; i < mine.size(); i++) {
            Ride* r = mine[i];
            std::string state;
            if (r->bookingIndex(me) >= 0) state = "CONFIRMED";
            else {
                int pos = 0, found = 0;
                r->requests.forEach([&](const SeatRequest& q) {
                    pos++;
                    if (q.passenger == me) found = pos;
                });
                state = "PENDING (position " + util::toStr(found) + " in queue)";
            }
            std::cout << (i + 1) << ") " << state << " - ";
            printRideLine(ctx, r);
        }
        int c = ui::askInt("\nCancel/withdraw ride number (0 = back): ", 0, mine.size());
        if (c == 0) return;
        std::string err;
        if (!ui::askYesNo("Are you sure?")) continue;
        if (ctx.requests.cancelMyBooking(mine[c - 1], ctx.current, err)) std::cout << "Done. The driver has been notified.\n";
        else std::cout << err << "\n";
        ctx.save();
    }
}

void driverRidesFlow(Context& ctx) {
    NavScope nav(ctx, "My rides");
    for (;;) {
        ui::title("My posted rides");
        printBreadcrumb(ctx);
        DynArray<Ride*> mine;
        ctx.rides.ridesOfDriver(ctx.current->email, mine);
        if (mine.empty()) {
            std::cout << "You have not posted any rides yet.\n";
            return;
        }
        for (int i = 0; i < mine.size(); i++) {
            std::cout << (i + 1) << ") ";
            printRideLine(ctx, mine[i]);
            if (mine[i]->requests.size())
                std::cout << "     -> " << mine[i]->requests.size() << " pending request(s)\n";
        }
        int c = ui::askInt("\nOpen ride number (0 = back): ", 0, mine.size());
        if (c == 0) return;
        manageRide(ctx, mine[c - 1]);
    }
}

void chatFlow(Context& ctx) {
    NavScope nav(ctx, "Chat");
    DynArray<Ride*> mine;
    ctx.rides.ridesOfParticipant(ctx.current->email, mine);
    DynArray<Ride*> usable;
    for (int i = 0; i < mine.size(); i++)
        if (mine[i]->status != RideStatus::Cancelled) usable.push(mine[i]);
    if (usable.empty()) {
        std::cout << "\nChat is available once you are the driver of a ride or have a confirmed seat.\n";
        return;
    }
    ui::title("Choose a ride chat");
    for (int i = 0; i < usable.size(); i++) {
        std::cout << (i + 1) << ") ";
        printRideLine(ctx, usable[i]);
    }
    int c = ui::askInt("Ride number (0 = back): ", 0, usable.size());
    if (c > 0) chatRoom(ctx, usable[c - 1]);
}

void inboxFlow(Context& ctx) {
    NavScope nav(ctx, "Notifications");
    User* u = ctx.current;
    for (;;) {
        ui::title("Notifications");
        printBreadcrumb(ctx);
        if (u->notifications.empty()) std::cout << "Your inbox is empty.\n";
        u->notifications.forEach([](const Notification& n) {
            std::cout << (n.read ? "   " : "NEW") << " [" << util::formatDateTime(n.ts) << "] " << n.text << "\n";
        });
        NotificationService::markAllRead(u);
        std::cout << "\n1. Dismiss oldest notification\n2. Clear all\n0. Back\n";
        int c = ui::askInt("> ", 0, 2);
        if (c == 0) return;
        if (c == 1) {
            Notification n;
            if (!NotificationService::popOldest(u, n)) std::cout << "Nothing to dismiss.\n";
        } else {
            u->notifications.clear();
        }
        ctx.save();
    }
}

void ratingsFlow(Context& ctx) {
    NavScope nav(ctx, "Ratings");
    User* u = ctx.current;
    for (;;) {
        ui::title("Ratings & reviews");
        printBreadcrumb(ctx);
        if (u->ratingCount) std::cout << "Your rating: " << u->avgRating() << " / 5 from " << u->ratingCount << " review(s)\n";
        else std::cout << "You have not been rated yet.\n";
        ctx.ratings.forEach([&](const RatingRecord& r) {
            if (r.ratee == u->email)
                std::cout << "  " << r.stars << "/5 from " << nameOf(ctx, r.rater) << " (ride #" << r.rideId << ")"
                          << (r.comment.empty() ? "" : ": " + r.comment) << "\n";
        });

        DynArray<PendingRating> pending;
        ctx.ratings.pendingFor(u->email, pending);
        std::cout << "\nPeople you can rate now:\n";
        if (pending.empty()) std::cout << "  nobody - you're all caught up\n";
        for (int i = 0; i < pending.size(); i++)
            std::cout << "  " << (i + 1) << ") " << nameOf(ctx, pending[i].ratee) << " (ride #" << pending[i].rideId << ")\n";
        if (pending.empty()) return;

        int c = ui::askInt("\nRate person number (0 = back): ", 0, pending.size());
        if (c == 0) return;
        int stars = ui::askInt("Stars (1-5): ", 1, 5);
        std::string comment = ui::ask("Comment (optional): ");
        std::string err;
        if (ctx.ratings.rate(pending[c - 1].rideId, u->email, pending[c - 1].ratee, stars, comment, err))
            std::cout << "Thanks for your feedback!\n";
        else std::cout << err << "\n";
        ctx.save();
    }
}

void historyFlow(Context& ctx) {
    NavScope nav(ctx, "History");
    ui::title("Ride history (newest first)");
    printBreadcrumb(ctx);
    if (ctx.current->history.empty()) std::cout << "No completed rides yet.\n";
    ctx.current->history.forEach([](const HistoryEntry& h) {
        std::cout << util::formatDateTime(h.ts) << " | ride #" << h.rideId << " | as " << h.role << " | " << h.route
                  << " | with: " << h.coTravelers << "\n";
    });
    ui::pause();
}
