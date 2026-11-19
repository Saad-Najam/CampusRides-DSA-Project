// Member 3 (Shamaem Saqib) - tests for LinkedList, Queue, RequestService, RatingSystem, AdminPanel, SocialStore.
#include "TestFramework.h"
#include "common/Utils.h"
#include "member1_saad/Context.h"
#include "member3_shamaem/LinkedList.h"
#include "member3_shamaem/NotificationService.h"
#include "member3_shamaem/Queue.h"
#include "member3_shamaem/SocialStore.h"

static void testListAndQueue() {
    SECTION("LinkedList");
    LinkedList<int> l;
    int v;
    CHECK(l.empty() && !l.popFront(v) && l.front() == nullptr);
    l.pushBack(2);
    l.pushBack(3);
    l.pushFront(1);
    CHECK(l.size() == 3 && *l.front() == 1);
    CHECK(l.removeFirst([](int x) { return x == 3; }));     // remove tail
    l.pushBack(4);                                            // tail pointer must still be valid
    CHECK(l.removeFirst([](int x) { return x == 2; }));     // remove middle
    CHECK(!l.removeFirst([](int x) { return x == 99; }));
    int sum = 0;
    l.forEach([&](int x) { sum += x; });
    CHECK(sum == 5 && l.size() == 2);
    CHECK(l.popFront(v) && v == 1 && l.popFront(v) && v == 4 && l.empty());

    SECTION("Queue is FIFO");
    Queue<int> q;
    CHECK(q.empty() && !q.dequeue(v) && q.peek() == nullptr);
    for (int i = 0; i < 1000; i++) q.enqueue(i);
    bool fifo = true;
    for (int i = 0; i < 1000; i++) fifo = fifo && q.dequeue(v) && v == i;
    CHECK(fifo && q.empty());
    q.enqueue(1); q.enqueue(2); q.enqueue(3);
    CHECK(q.removeIf([](int x) { return x == 2; }) && q.size() == 2);
    CHECK(*q.peek() == 1);
}

// Builds: driver d, passengers p1..p3, ride #1 with 2 seats on Gulshan -> Johar -> Shah Faisal -> Campus.
struct Fixture {
    Context ctx;
    User *d, *p1, *p2, *p3;
    Ride* ride;
    std::string err;

    Fixture() : ctx("test_data_ctx") {
        d = ctx.users.registerUser("Driver D", "d@nu.edu.pk", "25K-0001", "secret1", "", err);
        p1 = ctx.users.registerUser("Pass One", "p1@nu.edu.pk", "25K-0002", "secret1", "", err);
        p2 = ctx.users.registerUser("Pass Two", "p2@nu.edu.pk", "25K-0003", "secret1", "", err);
        p3 = ctx.users.registerUser("Pass Three", "p3@nu.edu.pk", "25K-0004", "secret1", "", err);
        DynArray<std::string> route;
        route.push("Gulshan-e-Iqbal"); route.push("Johar"); route.push("Shah Faisal"); route.push("Campus");
        ride = ctx.rides.createRide(d->email, route, 202612010730LL, 2, "car", "", err);
    }
};

static int countNotes(User* u, const char* needle) {
    int n = 0;
    u->notifications.forEach([&](const Notification& x) {
        if (x.text.find(needle) != std::string::npos) n++;
    });
    return n;
}

static void testRequests() {
    SECTION("Request validation");
    Fixture f;
    RequestService& rs = f.ctx.requests;
    std::string err;
    CHECK(!rs.requestSeat(f.ride, f.d, "Johar", "Campus", err));               // own ride
    CHECK(!rs.requestSeat(f.ride, f.p1, "Campus", "Johar", err));              // wrong order
    CHECK(!rs.requestSeat(f.ride, f.p1, "Johar", "Johar", err));               // same stop
    CHECK(!rs.requestSeat(f.ride, f.p1, "Johar", "DHA", err));                 // stop not on route
    CHECK(!rs.requestSeat(nullptr, f.p1, "Johar", "Campus", err));
    CHECK(rs.requestSeat(f.ride, f.p1, "johar", "campus", err));               // case-insensitive
    CHECK(!rs.requestSeat(f.ride, f.p1, "Johar", "Campus", err));              // duplicate request
    CHECK(countNotes(f.d, "requested a seat") == 1);

    SECTION("FIFO processing and seat management");
    CHECK(rs.requestSeat(f.ride, f.p2, "Gulshan-e-Iqbal", "Campus", err));
    CHECK(rs.requestSeat(f.ride, f.p3, "Johar", "Shah Faisal", err));
    CHECK(f.ride->requests.size() == 3);
    CHECK(rs.processNext(f.ride, f.d, true, err));                              // p1 first (oldest)
    CHECK(f.ride->bookingIndex("p1@nu.edu.pk") >= 0 && f.ride->seatsAvailable == 1);
    CHECK(countNotes(f.p1, "CONFIRMED") == 1);
    CHECK(rs.processNext(f.ride, f.d, true, err));                              // p2 second
    CHECK(f.ride->bookingIndex("p2@nu.edu.pk") >= 0 && f.ride->seatsAvailable == 0);
    CHECK(countNotes(f.d, "now full") == 1);
    CHECK(!rs.processNext(f.ride, f.d, true, err));                             // p3 cannot be accepted: full
    CHECK(f.ride->requests.size() == 1);                                        // ... and stays queued
    CHECK(rs.processNext(f.ride, f.d, false, err));                             // decline p3
    CHECK(countNotes(f.p3, "declined") == 1);
    CHECK(!rs.processNext(f.ride, f.d, true, err));                             // empty queue
    CHECK(!rs.processNext(f.ride, f.p1, true, err));                            // not the driver
    CHECK(!rs.requestSeat(f.ride, f.p3, "Johar", "Campus", err));               // ride full now
    CHECK(!rs.requestSeat(f.ride, f.p1, "Johar", "Campus", err));               // already confirmed

    SECTION("Cancelling a seat frees it");
    CHECK(rs.cancelMyBooking(f.ride, f.p1, err));
    CHECK(f.ride->seatsAvailable == 1 && f.ride->bookingIndex("p1@nu.edu.pk") < 0);
    CHECK(countNotes(f.d, "cancelled their seat") == 1);
    CHECK(!rs.cancelMyBooking(f.ride, f.p1, err));                              // nothing left to cancel
    CHECK(rs.requestSeat(f.ride, f.p3, "Johar", "Campus", err));
    CHECK(rs.cancelMyBooking(f.ride, f.p3, err));                               // withdraw a pending request
    CHECK(f.ride->requests.empty() && f.ride->seatsAvailable == 1);

    SECTION("Chat is limited to participants");
    CHECK(rs.sendMessage(f.ride, f.d, "On my way", err));
    CHECK(rs.sendMessage(f.ride, f.p2, "Thanks!", err));
    CHECK(!rs.sendMessage(f.ride, f.p3, "hello", err));                         // outsider
    CHECK(!rs.sendMessage(f.ride, f.d, "   ", err));                            // blank
    CHECK(f.ride->chat.size() == 2);

    SECTION("Completing a ride -> history, ratings");
    CHECK(!rs.completeRide(f.ride, f.p2, err));                                 // only the driver
    CHECK(rs.completeRide(f.ride, f.d, err));
    CHECK(f.ride->status == RideStatus::Completed);
    CHECK(!rs.completeRide(f.ride, f.d, err));                                  // already completed
    CHECK(!rs.cancelRide(f.ride, f.d->email, false, err));                      // cannot cancel afterwards
    CHECK(f.d->history.size() == 1 && f.p2->history.size() == 1 && f.p3->history.size() == 0);
    CHECK(f.d->history.front()->coTravelers.find("Pass Two") != std::string::npos);

    RatingSystem& rt = f.ctx.ratings;
    CHECK(!rt.rate(1, "p2@nu.edu.pk", "p2@nu.edu.pk", 5, "", err));             // self
    CHECK(!rt.rate(1, "p2@nu.edu.pk", "d@nu.edu.pk", 6, "", err));              // invalid stars
    CHECK(!rt.rate(1, "p2@nu.edu.pk", "d@nu.edu.pk", 0, "", err));
    CHECK(!rt.rate(1, "p3@nu.edu.pk", "d@nu.edu.pk", 5, "", err));              // wasn't on the ride
    CHECK(!rt.rate(99, "p2@nu.edu.pk", "d@nu.edu.pk", 5, "", err));             // unknown ride
    CHECK(rt.rate(1, "p2@nu.edu.pk", "d@nu.edu.pk", 5, "great, punctual", err));
    CHECK(!rt.rate(1, "p2@nu.edu.pk", "d@nu.edu.pk", 1, "", err));              // duplicate
    CHECK(rt.rate(1, "d@nu.edu.pk", "p2@nu.edu.pk", 4, "", err));
    CHECK(f.d->ratingCount == 1 && f.d->avgRating() == 5.0);
    DynArray<PendingRating> pending;
    rt.pendingFor("p2@nu.edu.pk", pending);
    CHECK(pending.empty());
    rt.pendingFor("d@nu.edu.pk", pending);
    CHECK(pending.size() == 0);   // the only passenger (p2) was rated, p1 cancelled

    SECTION("Rating before completion is rejected");
    Fixture g;
    std::string e2;
    g.ctx.requests.requestSeat(g.ride, g.p1, "Johar", "Campus", e2);
    g.ctx.requests.processNext(g.ride, g.d, true, e2);
    CHECK(!g.ctx.ratings.rate(1, "p1@nu.edu.pk", "d@nu.edu.pk", 5, "", e2));
}

static void testCancelAndAdmin() {
    Fixture f;
    std::string err;
    RequestService& rs = f.ctx.requests;
    rs.requestSeat(f.ride, f.p1, "Johar", "Campus", err);
    rs.requestSeat(f.ride, f.p2, "Johar", "Campus", err);
    rs.processNext(f.ride, f.d, true, err);   // p1 confirmed, p2 pending

    SECTION("Ride cancellation");
    CHECK(!rs.cancelRide(f.ride, f.p1->email, false, err));   // not the driver
    CHECK(rs.cancelRide(f.ride, f.d->email, false, err));
    CHECK(f.ride->status == RideStatus::Cancelled);
    CHECK(countNotes(f.p1, "CANCELLED") == 1 && countNotes(f.p2, "CANCELLED") == 1);
    CHECK(!rs.cancelRide(f.ride, f.d->email, false, err));    // already cancelled
    CHECK(!rs.requestSeat(f.ride, f.p3, "Johar", "Campus", err));
    CHECK(!rs.completeRide(f.ride, f.d, err));

    SECTION("Admin: suspend / unsuspend / cancel / undo stack");
    AdminPanel& ap = f.ctx.admin;
    std::string msg;
    CHECK(!ap.canUndo() && !ap.undoLast(msg));
    CHECK(ap.suspendUser("p3@nu.edu.pk", err));
    CHECK(!ap.suspendUser("p3@nu.edu.pk", err));               // already suspended
    CHECK(!ap.suspendUser("ghost@nu.edu.pk", err));
    CHECK(!f.ctx.users.login("p3@nu.edu.pk", "secret1", err));
    CHECK(ap.undoLast(msg));                                    // undo suspension
    CHECK(f.ctx.users.login("p3@nu.edu.pk", "secret1", err) != nullptr);

    Ride* second = f.ctx.rides.createRide("d@nu.edu.pk", f.ride->route, 202612020730LL, 2, "car", "", err);
    CHECK(second && ap.cancelRide(second->id, err));
    CHECK(second->status == RideStatus::Cancelled);
    CHECK(!ap.cancelRide(second->id, err) && !ap.cancelRide(999, err));
    CHECK(ap.suspendUser("p2@nu.edu.pk", err));
    CHECK(ap.undoLast(msg));                                    // LIFO: first undoes the suspension of p2
    CHECK(f.p2->suspended == false && second->status == RideStatus::Cancelled);
    CHECK(ap.undoLast(msg));                                    // then reopens the ride
    CHECK(second->status == RideStatus::Open);
    CHECK(!ap.canUndo());

    SECTION("Admin search and statistics");
    DynArray<User*> users;
    ap.searchUsers("pass", users);
    CHECK(users.size() == 3);
    ap.searchUsers("25K-0001", users);
    CHECK(users.size() == 1 && users[0] == f.d);
    ap.searchUsers("nomatch", users);
    CHECK(users.empty());
    DynArray<Ride*> rides;
    ap.searchRides("cancelled", rides);
    CHECK(rides.size() == 1 && rides[0] == f.ride);
    PlatformStats st = ap.stats();
    CHECK(st.users == 4 && st.ridesOpen == 1 && st.ridesCancelled == 1 && st.seatsConfirmed == 1 && st.pendingRequests == 1);
}

static void testPersistence() {
    SECTION("SocialStore + stores full round trip");
    Fixture f;
    std::string err;
    RequestService& rs = f.ctx.requests;
    rs.requestSeat(f.ride, f.p1, "Johar", "Campus", err);
    rs.requestSeat(f.ride, f.p2, "Gulshan-e-Iqbal", "Johar", err);
    rs.requestSeat(f.ride, f.p3, "Johar", "Shah Faisal", err);
    rs.processNext(f.ride, f.d, true, err);                       // p1 confirmed; p2, p3 still queued in order
    rs.sendMessage(f.ride, f.d, "See you, at 7:30\nsharp", err);
    f.ctx.save();

    Context loaded("test_data_ctx");
    loaded.load();
    CHECK(loaded.users.count() == f.ctx.users.count() + 1);   // load() creates the admin account
    Ride* r = loaded.rides.findRide(1);
    CHECK(r != nullptr);
    if (!r) return;
    CHECK(r->bookings.size() == 1 && r->bookings[0].passenger == "p1@nu.edu.pk" && r->seatsAvailable == 1);
    CHECK(r->requests.size() == 2 && r->requests.peek()->passenger == "p2@nu.edu.pk");   // queue order preserved
    CHECK(r->chat.size() == 1 && r->chat.front()->text == "See you, at 7:30\nsharp");
    CHECK(countNotes(loaded.users.findByEmail("p1@nu.edu.pk"), "CONFIRMED") == 1);
    CHECK(loaded.users.findByEmail("d@nu.edu.pk")->notifications.size() == f.d->notifications.size());

    // finish the ride in the loaded copy and make sure ratings survive another reload
    std::string e2;
    CHECK(loaded.requests.processNext(r, loaded.users.findByEmail("d@nu.edu.pk"), false, e2));
    CHECK(loaded.requests.completeRide(r, loaded.users.findByEmail("d@nu.edu.pk"), e2));
    CHECK(loaded.ratings.rate(1, "p1@nu.edu.pk", "d@nu.edu.pk", 4, "ok, fine", e2));
    loaded.save();

    Context again("test_data_ctx");
    again.load();
    User* drv = again.users.findByEmail("d@nu.edu.pk");
    CHECK(drv->ratingCount == 1 && drv->ratingSum == 4);
    CHECK(drv->history.size() == 1 && again.rides.findRide(1)->status == RideStatus::Completed);
    CHECK(!again.ratings.rate(1, "p1@nu.edu.pk", "d@nu.edu.pk", 5, "", e2));   // duplicate still rejected after reload

    SECTION("Loading from an empty directory is safe");
    Context empty("test_data_empty_dir_that_does_not_exist");
    empty.load();
    CHECK(empty.users.count() == 1 && empty.rides.count() == 0);   // just the admin

    SECTION("Notification queue helpers");
    User* u = f.p3;
    int before = u->notifications.size();
    NotificationService::push(u, "first");
    NotificationService::push(u, "second");
    Notification n;
    CHECK(NotificationService::popOldest(u, n));
    CHECK(u->notifications.size() == before + 1);
    NotificationService::markAllRead(u);
    CHECK(u->unreadCount() == 0);
    Fixture h;
    CHECK(!NotificationService::popOldest(h.p1, n));   // empty inbox
}

void runShamaemTests() {
    testListAndQueue();
    testRequests();
    testCancelAndAdmin();
    testPersistence();
}
