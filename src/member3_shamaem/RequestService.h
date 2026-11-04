// Member 3 (Shamaem Saqib) - Seat requests, bookings, ride lifecycle and chat.
// Every ride has a FIFO Queue of SeatRequests: the driver always handles the oldest one first.
// Seat counts are updated on confirm / cancel, and the affected users are notified.
#pragma once
#include <string>

#include "member1_saad/UserManager.h"
#include "member2_ayaan/RideManager.h"

class RequestService {
    UserManager& users_;
    RideManager& rides_;

    std::string describe(const Ride* r) const;
    void addHistory(Ride* r, long long ts);

public:
    RequestService(UserManager& u, RideManager& r) : users_(u), rides_(r) {}

    // Passenger asks for a seat between two stops of the ride's route (pickup must come before dropoff).
    bool requestSeat(Ride* ride, User* passenger, const std::string& pickup, const std::string& dropoff,
                     std::string& err);

    // Driver handles the oldest pending request: accept (needs a free seat) or decline.
    bool processNext(Ride* ride, User* driver, bool accept, std::string& err);

    // Passenger withdraws a pending request or cancels a confirmed seat (seat is released).
    bool cancelMyBooking(Ride* ride, User* passenger, std::string& err);

    // Driver (or an admin acting on their behalf) cancels the whole ride.
    bool cancelRide(Ride* ride, const std::string& byEmail, bool byAdmin, std::string& err);
    // Admin undo: reopen a ride that was cancelled by an admin.
    bool reopenRide(Ride* ride, std::string& err);

    // Driver marks a ride as done: writes ride history, unlocks ratings.
    bool completeRide(Ride* ride, User* driver, std::string& err);

    // Text chat between confirmed participants (driver + confirmed passengers).
    bool sendMessage(Ride* ride, User* sender, const std::string& text, std::string& err);
};
