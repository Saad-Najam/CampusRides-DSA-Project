// Member 1 (Syed Saad Najam) - User profile record.
#pragma once
#include <string>

#include "member3_shamaem/LinkedList.h"
#include "member3_shamaem/Notification.h"

struct User {
    std::string email;       // lower-case, unique, ends with @nu.edu.pk
    std::string studentId;   // e.g. 25K-0807, unique
    std::string name;
    std::string passHash;
    std::string phone;
    std::string vehicle;     // drivers only (free text, e.g. "Honda City - KAB-123")
    bool isAdmin = false;
    bool suspended = false;

    int ratingSum = 0;       // maintained by RatingSystem
    int ratingCount = 0;

    LinkedList<Notification> notifications;  // per-user notification queue (tail enqueue, head dequeue)
    LinkedList<HistoryEntry> history;        // completed rides, newest first

    double avgRating() const { return ratingCount == 0 ? 0.0 : (double)ratingSum / ratingCount; }

    int unreadCount() const {
        int n = 0;
        notifications.forEach([&](const Notification& x) {
            if (!x.read) n++;
        });
        return n;
    }
};
