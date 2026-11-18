// Member 2 (Ayaan Kashif) - posting, searching and browsing rides.
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "common/DynArray.h"
#include "common/Utils.h"
#include "member1_saad/Ui.h"
#include "member2_ayaan/CampusMap.h"
#include "member2_ayaan/RideMatcher.h"

namespace {

void listZones(const Graph& g) {
    std::cout << "Available zones: ";
    for (int i = 0; i < g.zoneCount(); i++) std::cout << (i ? ", " : "") << g.nameOf(i);
    std::cout << "\n";
}

// Keeps asking until a known zone is typed. `def` is used when the user just presses Enter.
std::string askZone(const Graph& g, const std::string& prompt, const std::string& def = "") {
    for (;;) {
        std::string s = ui::ask(prompt + (def.empty() ? ": " : " [" + def + "]: "));
        if (s.empty() && !def.empty()) s = def;
        int v = g.indexOf(s);
        if (v >= 0) return g.nameOf(v);
        std::cout << "  Unknown zone '" << s << "'.\n";
        listZones(g);
    }
}

void splitList(const std::string& s, DynArray<std::string>& out) {
    out.clear();
    std::string cur;
    for (size_t i = 0; i <= s.size(); i++) {
        if (i == s.size() || s[i] == ',') {
            std::string t = util::trim(cur);
            if (!t.empty()) out.push(t);
            cur.clear();
        } else {
            cur += s[i];
        }
    }
}

std::string driverName(const Context& ctx, const Ride* r) {
    User* d = ctx.users.findByEmail(r->driver);
    return d ? d->name : r->driver;
}

// Shared by the search and the feed screens.
void requestSeatUi(Context& ctx, Ride* ride, const std::string& pickup, const std::string& dropoff) {
    std::string err;
    if (ctx.requests.requestSeat(ride, ctx.current, pickup, dropoff, err))
        std::cout << "\nRequest sent! The driver will confirm in order of arrival. You'll get a notification.\n";
    else
        std::cout << "\nCould not request seat: " << err << "\n";
}

}  // namespace

void printRideLine(const Context& ctx, const Ride* r) {
    std::cout << "#" << r->id << " | " << util::formatDateTime(r->departKey) << " | " << r->routeString() << " | "
              << driverName(ctx, r) << " | seats " << r->seatsAvailable << "/" << r->totalSeats << " | "
              << statusName(r->status) << "\n";
}

void printRideDetails(const Context& ctx, const Ride* r) {
    User* d = ctx.users.findByEmail(r->driver);
    std::cout << "Ride #" << r->id << " [" << statusName(r->status) << "]\n"
              << "  Departure : " << util::formatDateTime(r->departKey) << "\n"
              << "  Route     : " << r->routeString() << "\n"
              << "  Distance  : " << ctx.rides.map().pathDistance(r->route, 0, r->route.size() - 1) << " km\n"
              << "  Driver    : " << driverName(ctx, r);
    if (d && d->ratingCount) std::cout << " (" << d->avgRating() << "/5 from " << d->ratingCount << " reviews)";
    std::cout << "\n  Vehicle   : " << (r->vehicle.empty() ? "-" : r->vehicle) << "\n"
              << "  Seats     : " << r->seatsAvailable << " free of " << r->totalSeats << "\n";
    if (!r->notes.empty()) std::cout << "  Notes     : " << r->notes << "\n";
}

void postRideFlow(Context& ctx) {
    NavScope nav(ctx, "Post ride");
    ui::title("Post a ride");
    printBreadcrumb(ctx);
    const Graph& g = ctx.rides.map();

    if (ctx.current->vehicle.empty()) {
        std::cout << "Add your vehicle details first (shown to passengers).\n";
        ctx.current->vehicle = ui::ask("Vehicle (make/model/plate): ");
    }
    listZones(g);
    std::string origin = askZone(g, "Starting zone");
    std::string dest = askZone(g, "Destination zone", kCampusZone);
    if (util::equalsIgnoreCase(origin, dest)) {
        std::cout << "Start and destination must differ.\n";
        return;
    }

    // Suggest the shortest road route with Dijkstra, or let the driver type their own stops.
    DynArray<int> path;
    int km = g.shortestPath(g.indexOf(origin), g.indexOf(dest), &path);
    DynArray<std::string> route;
    if (km < 0) {
        std::cout << "No road connection between those zones.\n";
        return;
    }
    for (int i = 0; i < path.size(); i++) route.push(g.nameOf(path[i]));
    std::cout << "\nShortest route: ";
    for (int i = 0; i < route.size(); i++) std::cout << (i ? " > " : "") << route[i];
    std::cout << "  (" << km << " km)\n";
    if (!ui::askYesNo("Use this route?")) {
        DynArray<std::string> custom;
        splitList(ui::ask("Enter your stops in order, separated by commas: "), custom);
        route = custom;
    }

    long long when = -1;
    for (;;) {
        when = util::parseDateTime(ui::ask("Departure (YYYY-MM-DD HH:MM): "));
        if (when < 0) std::cout << "  Invalid date/time.\n";
        else if (when <= util::nowKey()) std::cout << "  Departure must be in the future.\n";
        else break;
    }
    int seats = ui::askInt("Available seats (1-8): ", 1, 8);
    std::string notes = ui::ask("Notes for passengers (optional): ");

    std::string err;
    Ride* r = ctx.rides.createRide(ctx.current->email, route, when, seats, ctx.current->vehicle, notes, err);
    if (!r) {
        std::cout << "\nCould not post ride: " << err << "\n";
        return;
    }
    std::cout << "\nRide posted!\n";
    printRideDetails(ctx, r);
}

void searchRidesFlow(Context& ctx) {
    NavScope nav(ctx, "Search rides");
    ui::title("Find a ride");
    printBreadcrumb(ctx);
    const Graph& g = ctx.rides.map();
    listZones(g);
    std::string pickup = askZone(g, "Your pickup zone");
    std::string drop = askZone(g, "Your drop-off zone", kCampusZone);
    if (util::equalsIgnoreCase(pickup, drop)) {
        std::cout << "Pickup and drop-off must differ.\n";
        return;
    }

    long long lo = -1, hi = -1;
    for (;;) {
        std::string d = ui::ask("Date (YYYY-MM-DD) or Enter for any date: ");
        if (d.empty() || util::parseDate(d, lo, hi)) {
            if (d.empty()) lo = hi = -1;
            break;
        }
        std::cout << "  Invalid date.\n";
    }
    std::string w = ui::ask("Max extra distance you can cover to reach the route, in km [5]: ");
    int walk = w.empty() ? 5 : std::atoi(w.c_str());
    if (walk < 0) walk = 0;

    DynArray<MatchResult> results;
    RideMatcher::search(ctx.rides, pickup, drop, lo, hi, walk, ctx.current->email, results);
    if (results.empty()) {
        std::cout << "\nNo matching rides. Try another date or a larger distance.\n";
        return;
    }
    std::cout << "\n" << results.size() << " matching ride(s), best match first:\n";
    ui::line();
    for (int i = 0; i < results.size(); i++) {
        const MatchResult& m = results[i];
        std::cout << (i + 1) << ") ";
        printRideLine(ctx, m.ride);
        std::cout << "     board at " << m.ride->route[m.pickupIdx] << ", leave at " << m.ride->route[m.dropIdx] << " ("
                  << m.sharedKm << " km together, " << m.walkKm << " km extra for you)\n";
    }
    int pick = ui::askInt("\nRequest a seat on result number (0 = back): ", 0, results.size());
    if (pick == 0) return;
    const MatchResult& m = results[pick - 1];
    requestSeatUi(ctx, m.ride, m.ride->route[m.pickupIdx], m.ride->route[m.dropIdx]);
}

void browseFeedFlow(Context& ctx) {
    NavScope nav(ctx, "Ride feed");
    const DoublyLinkedList<Ride*>& feed = ctx.rides.feed();
    if (feed.empty()) {
        std::cout << "\nNo rides have been posted yet.\n";
        return;
    }
    DoublyLinkedList<Ride*>::Node* cur = feed.head();
    for (;;) {
        ui::title("Ride feed (sorted by departure)");
        printBreadcrumb(ctx);
        printRideDetails(ctx, cur->data);
        std::cout << "\n[" << (cur->prev ? "p = previous  " : "") << (cur->next ? "n = next  " : "")
                  << "r = request seat  q = back]\n";
        std::string c = util::toLower(ui::ask("> "));
        if (c == "q") return;
        if (c == "n" && cur->next) cur = cur->next;
        else if (c == "p" && cur->prev) cur = cur->prev;
        else if (c == "r") {
            Ride* r = cur->data;
            std::cout << "Stops: ";
            for (int i = 0; i < r->route.size(); i++) std::cout << (i + 1) << "=" << r->route[i] << (i + 1 < r->route.size() ? ", " : "\n");
            int a = ui::askInt("Board at stop number: ", 1, r->route.size());
            int b = ui::askInt("Leave at stop number: ", 1, r->route.size());
            requestSeatUi(ctx, r, r->route[a - 1], r->route[b - 1]);
            ui::pause();
        }
    }
}

void upcomingRidesFlow(Context& ctx) {
    NavScope nav(ctx, "Upcoming rides");
    for (;;) {
        ui::title("Upcoming rides");
        printBreadcrumb(ctx);
        std::cout << "1. Next 10 open rides (earliest first)\n2. Rides on a given date (time-sorted index)\n0. Back\n";
        int c = ui::askInt("> ", 0, 2);
        if (c == 0) return;
        DynArray<Ride*> list;
        if (c == 1) {
            ctx.rides.upcomingOpen(10, list);
        } else {
            long long lo, hi;
            if (!util::parseDate(ui::ask("Date (YYYY-MM-DD): "), lo, hi)) {
                std::cout << "Invalid date.\n";
                continue;
            }
            ctx.rides.rangeByTime(lo, hi, list);
        }
        if (list.empty()) std::cout << "\nNo rides found.\n";
        for (int i = 0; i < list.size(); i++) printRideLine(ctx, list[i]);
        ui::pause();
    }
}
