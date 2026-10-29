// Member 3 (Shamaem Saqib) - Per-user notification queue helpers.
// Each user owns a LinkedList<Notification>: enqueue at the tail, dequeue from the head (both O(1)).
#pragma once
#include <string>

#include "common/Utils.h"
#include "member1_saad/User.h"
#include "member1_saad/UserManager.h"

namespace NotificationService {

inline void push(User* u, const std::string& text) {
    if (!u) return;
    Notification n;
    n.ts = util::nowKey();
    n.read = false;
    n.text = text;
    u->notifications.pushBack(n);
}

inline void pushTo(UserManager& users, const std::string& email, const std::string& text) {
    push(users.findByEmail(email), text);
}

// Removes and returns the oldest notification (queue behaviour). False if the inbox is empty.
inline bool popOldest(User* u, Notification& out) { return u->notifications.popFront(out); }

inline void markAllRead(User* u) {
    u->notifications.forEach([](Notification& n) { n.read = true; });
}

}  // namespace NotificationService
