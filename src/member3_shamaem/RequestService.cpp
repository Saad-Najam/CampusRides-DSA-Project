#include "member3_shamaem/RequestService.h"

#include "common/Utils.h"
#include "member3_shamaem/NotificationService.h"

std::string RequestService::describe(const Ride* r) const {
    return "ride #" + util::toStr(r->id) + " (" + r->routeString() + ", " + util::formatDateTime(r->departKey) + ")";
}

static std::string displayName(UserManager& users, const std::string& email) {
    User* u = users.findByEmail(email);
    return u ? u->name : email;
}

bool RequestService::requestSeat(Ride* ride, User* passenger, const std::string& pickup,
                                 const std::string& dropoff, std::string& err) {
    if (!ride || !passenger) { err = "Ride not found."; return false; }
    if (ride->status != RideStatus::Open) { err = "This ride is no longer open."; return false; }
    if (ride->driver == passenger->email) { err = "You cannot request a seat on your own ride."; return false; }
    if (ride->bookingIndex(passenger->email) >= 0) { err = "You already have a confirmed seat on this ride."; return false; }
    if (ride->hasPendingRequest(passenger->email)) { err = "You already have a pending request on this ride."; return false; }
    if (ride->seatsAvailable <= 0) { err = "This ride is full."; return false; }
    int p = ride->stopIndex(pickup), d = ride->stopIndex(dropoff);
    if (p < 0 || d < 0) { err = "Pickup and drop-off must both be stops on the driver's route."; return false; }
    if (p >= d) { err = "Pickup must come before drop-off along the route."; return false; }

    SeatRequest r;
    r.passenger = passenger->email;
    r.pickup = ride->route[p];
    r.dropoff = ride->route[d];
    r.ts = util::nowKey();
    ride->requests.enqueue(r);
    NotificationService::pushTo(users_, ride->driver,
                                passenger->name + " requested a seat on " + describe(ride) + " (" + r.pickup + " -> " +
                                    r.dropoff + ").");
    return true;
}

bool RequestService::processNext(Ride* ride, User* driver, bool accept, std::string& err) {
    if (!ride || !driver || ride->driver != driver->email) { err = "Only the driver can handle requests."; return false; }
    if (ride->status != RideStatus::Open) { err = "This ride is no longer open."; return false; }
    SeatRequest* front = ride->requests.peek();
    if (!front) { err = "There are no pending requests."; return false; }

    if (accept) {
        if (ride->seatsAvailable <= 0) { err = "No seats left. Decline the request instead."; return false; }
        SeatRequest r;
        ride->requests.dequeue(r);
        Booking b;
        b.passenger = r.passenger;
        b.pickup = r.pickup;
        b.dropoff = r.dropoff;
        ride->bookings.push(b);
        ride->seatsAvailable--;
        NotificationService::pushTo(users_, r.passenger,
                                    "Your seat on " + describe(ride) + " is CONFIRMED (" + r.pickup + " -> " + r.dropoff + ").");
        if (ride->seatsAvailable == 0)
            NotificationService::push(driver, "Ride #" + util::toStr(ride->id) + " is now full.");
    } else {
        SeatRequest r;
        ride->requests.dequeue(r);
        NotificationService::pushTo(users_, r.passenger, "Your request for " + describe(ride) + " was declined.");
    }
    return true;
}

bool RequestService::cancelMyBooking(Ride* ride, User* passenger, std::string& err) {
    if (!ride || !passenger) { err = "Ride not found."; return false; }
    if (ride->status != RideStatus::Open) { err = "Only open rides can be changed."; return false; }
    std::string who = passenger->email;
    int bi = ride->bookingIndex(who);
    if (bi >= 0) {
        ride->bookings.removeAt(bi);
        ride->seatsAvailable++;
        NotificationService::pushTo(users_, ride->driver,
                                    passenger->name + " cancelled their seat on " + describe(ride) + ". A seat is free again.");
        return true;
    }
    if (ride->requests.removeIf([&](const SeatRequest& r) { return r.passenger == who; })) {
        NotificationService::pushTo(users_, ride->driver, passenger->name + " withdrew their request for " + describe(ride) + ".");
        return true;
    }
    err = "You have no request or booking on this ride.";
    return false;
}

bool RequestService::cancelRide(Ride* ride, const std::string& byEmail, bool byAdmin, std::string& err) {
    if (!ride) { err = "Ride not found."; return false; }
    if (!byAdmin && ride->driver != byEmail) { err = "Only the driver can cancel this ride."; return false; }
    if (ride->status != RideStatus::Open) { err = "Only open rides can be cancelled."; return false; }
    ride->status = RideStatus::Cancelled;
    std::string why = byAdmin ? " by an administrator" : " by the driver";
    for (int i = 0; i < ride->bookings.size(); i++)
        NotificationService::pushTo(users_, ride->bookings[i].passenger, "CANCELLED" + why + ": " + describe(ride) + ".");
    ride->requests.forEach([&](const SeatRequest& r) {
        NotificationService::pushTo(users_, r.passenger, "CANCELLED" + why + ": " + describe(ride) + ".");
    });
    if (byAdmin) NotificationService::pushTo(users_, ride->driver, "Your " + describe(ride) + " was cancelled by an administrator.");
    return true;
}

bool RequestService::reopenRide(Ride* ride, std::string& err) {
    if (!ride || ride->status != RideStatus::Cancelled) { err = "Ride is not cancelled."; return false; }
    ride->status = RideStatus::Open;
    NotificationService::pushTo(users_, ride->driver, "Your " + describe(ride) + " has been reinstated.");
    for (int i = 0; i < ride->bookings.size(); i++)
        NotificationService::pushTo(users_, ride->bookings[i].passenger, "Reinstated: " + describe(ride) + ".");
    return true;
}

void RequestService::addHistory(Ride* r, long long ts) {
    std::string route = r->routeString();
    User* drv = users_.findByEmail(r->driver);
    std::string passengers;
    for (int i = 0; i < r->bookings.size(); i++) {
        if (i) passengers += "; ";
        passengers += displayName(users_, r->bookings[i].passenger);
    }
    if (drv) {
        HistoryEntry h;
        h.ts = ts; h.rideId = r->id; h.role = "Driver"; h.route = route;
        h.coTravelers = passengers.empty() ? "-" : passengers;
        drv->history.pushFront(h);
    }
    for (int i = 0; i < r->bookings.size(); i++) {
        User* p = users_.findByEmail(r->bookings[i].passenger);
        if (!p) continue;
        std::string others = drv ? drv->name + " (driver)" : r->driver;
        for (int j = 0; j < r->bookings.size(); j++)
            if (j != i) others += "; " + displayName(users_, r->bookings[j].passenger);
        HistoryEntry h;
        h.ts = ts; h.rideId = r->id; h.role = "Passenger"; h.route = route; h.coTravelers = others;
        p->history.pushFront(h);
    }
}

bool RequestService::completeRide(Ride* ride, User* driver, std::string& err) {
    if (!ride || !driver || ride->driver != driver->email) { err = "Only the driver can complete this ride."; return false; }
    if (ride->status != RideStatus::Open) { err = "Only open rides can be completed."; return false; }
    if (ride->bookings.empty()) { err = "No confirmed passengers. Cancel the ride instead."; return false; }
    ride->status = RideStatus::Completed;
    SeatRequest r;
    while (ride->requests.dequeue(r))
        NotificationService::pushTo(users_, r.passenger, "Your request for " + describe(ride) + " expired because the ride has left.");
    addHistory(ride, util::nowKey());
    NotificationService::push(driver, "Ride #" + util::toStr(ride->id) + " completed. Please rate your passengers.");
    for (int i = 0; i < ride->bookings.size(); i++)
        NotificationService::pushTo(users_, ride->bookings[i].passenger,
                                    "Ride #" + util::toStr(ride->id) + " completed. Please rate your driver and co-riders.");
    return true;
}

bool RequestService::sendMessage(Ride* ride, User* sender, const std::string& text, std::string& err) {
    if (!ride || !sender) { err = "Ride not found."; return false; }
    if (!ride->isParticipant(sender->email)) { err = "Only the driver and confirmed passengers can chat."; return false; }
    if (ride->status == RideStatus::Cancelled) { err = "This ride was cancelled."; return false; }
    std::string t = util::trim(text);
    if (t.empty()) { err = "Message is empty."; return false; }
    ChatMessage m;
    m.ts = util::nowKey();
    m.sender = sender->email;
    m.text = t;
    ride->chat.pushBack(m);
    std::string note = "New message from " + sender->name + " in ride #" + util::toStr(ride->id) + ".";
    if (ride->driver != sender->email) NotificationService::pushTo(users_, ride->driver, note);
    for (int i = 0; i < ride->bookings.size(); i++)
        if (ride->bookings[i].passenger != sender->email) NotificationService::pushTo(users_, ride->bookings[i].passenger, note);
    return true;
}
