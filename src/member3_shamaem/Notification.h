// Member 3 (Shamaem Saqib) - plain record types stored in per-user linked lists.
#pragma once
#include <string>

struct Notification {
    long long ts = 0;
    bool read = false;
    std::string text;
};

struct HistoryEntry {
    long long ts = 0;       // when the ride was completed
    int rideId = 0;
    std::string role;       // "Driver" or "Passenger"
    std::string coTravelers;
    std::string route;
};
