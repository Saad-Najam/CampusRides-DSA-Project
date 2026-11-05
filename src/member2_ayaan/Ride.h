// Member 2 (Ayaan Kashif) - Ride record.
#pragma once
#include <string>

#include "common/DynArray.h"
#include "common/Utils.h"
#include "member3_shamaem/LinkedList.h"
#include "member3_shamaem/Queue.h"

enum class RideStatus { Open, Completed, Cancelled };

inline const char* statusName(RideStatus s) {
    switch (s) {
        case RideStatus::Open: return "Open";
        case RideStatus::Completed: return "Completed";
        default: return "Cancelled";
    }
}

inline RideStatus parseStatus(const std::string& s) {
    if (s == "Completed") return RideStatus::Completed;
    if (s == "Cancelled") return RideStatus::Cancelled;
    return RideStatus::Open;
}

struct SeatRequest {              // waiting in the ride's FIFO queue
    std::string passenger;
    std::string pickup;
    std::string dropoff;
    long long ts = 0;
};

struct Booking {                  // confirmed seat
    std::string passenger;
    std::string pickup;
    std::string dropoff;
};

struct ChatMessage {
    long long ts = 0;
    std::string sender;
    std::string text;
};

struct Ride {
    int id = 0;
    std::string driver;           // driver's email
    DynArray<std::string> route;  // ordered stops, e.g. Gulshan-e-Iqbal > Johar > Campus
    long long departKey = 0;      // YYYYMMDDHHMM
    int totalSeats = 0;
    int seatsAvailable = 0;
    std::string vehicle;
    std::string notes;
    RideStatus status = RideStatus::Open;

    Queue<SeatRequest> requests;  // FIFO: oldest request is processed first
    DynArray<Booking> bookings;
    LinkedList<ChatMessage> chat;

    int stopIndex(const std::string& zone) const {
        for (int i = 0; i < route.size(); i++)
            if (util::equalsIgnoreCase(route[i], zone)) return i;
        return -1;
    }

    std::string routeString() const {
        std::string s;
        for (int i = 0; i < route.size(); i++) {
            if (i) s += " > ";
            s += route[i];
        }
        return s;
    }

    int bookingIndex(const std::string& passengerEmail) const {
        for (int i = 0; i < bookings.size(); i++)
            if (bookings[i].passenger == passengerEmail) return i;
        return -1;
    }

    bool hasPendingRequest(const std::string& passengerEmail) const {
        bool found = false;
        requests.forEach([&](const SeatRequest& r) {
            if (r.passenger == passengerEmail) found = true;
        });
        return found;
    }

    bool isParticipant(const std::string& email) const { return email == driver || bookingIndex(email) >= 0; }
};
