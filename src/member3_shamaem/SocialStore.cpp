#include "member3_shamaem/SocialStore.h"

#include <cstdlib>
#include <fstream>

#include "common/Utils.h"

namespace SocialStore {

bool save(const UserManager& users, const RideManager& rides, const RatingSystem& ratings, const std::string& dir) {
    std::ofstream notes((dir + "/notifications.csv").c_str());
    std::ofstream hist((dir + "/history.csv").c_str());
    std::ofstream rate((dir + "/ratings.csv").c_str());
    std::ofstream book((dir + "/bookings.csv").c_str());
    std::ofstream req((dir + "/requests.csv").c_str());
    std::ofstream msg((dir + "/messages.csv").c_str());
    if (!notes || !hist || !rate || !book || !req || !msg) return false;

    users.forEach([&](User* u) {
        u->notifications.forEach([&](const Notification& n) {
            notes << util::csvEscape(u->email) << ',' << n.ts << ',' << (n.read ? 1 : 0) << ',' << util::csvEscape(n.text) << '\n';
        });
        u->history.forEach([&](const HistoryEntry& h) {
            hist << util::csvEscape(u->email) << ',' << h.ts << ',' << h.rideId << ',' << util::csvEscape(h.role) << ','
                 << util::csvEscape(h.coTravelers) << ',' << util::csvEscape(h.route) << '\n';
        });
    });
    ratings.forEach([&](const RatingRecord& r) {
        rate << r.rideId << ',' << util::csvEscape(r.rater) << ',' << util::csvEscape(r.ratee) << ',' << r.stars << ','
             << r.ts << ',' << util::csvEscape(r.comment) << '\n';
    });
    rides.forEachRide([&](Ride* r) {
        for (int i = 0; i < r->bookings.size(); i++)
            book << r->id << ',' << util::csvEscape(r->bookings[i].passenger) << ',' << util::csvEscape(r->bookings[i].pickup)
                 << ',' << util::csvEscape(r->bookings[i].dropoff) << '\n';
        r->requests.forEach([&](const SeatRequest& q) {
            req << r->id << ',' << util::csvEscape(q.passenger) << ',' << util::csvEscape(q.pickup) << ','
                << util::csvEscape(q.dropoff) << ',' << q.ts << '\n';
        });
        r->chat.forEach([&](const ChatMessage& m) {
            msg << r->id << ',' << m.ts << ',' << util::csvEscape(m.sender) << ',' << util::csvEscape(m.text) << '\n';
        });
    });
    return true;
}

void load(UserManager& users, RideManager& rides, RatingSystem& ratings, const std::string& dir) {
    std::string line;
    std::string f[7];

    {
        std::ifstream in((dir + "/notifications.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 4) continue;
            User* u = users.findByEmail(f[0]);
            if (!u) continue;
            Notification n;
            n.ts = std::atoll(f[1].c_str());
            n.read = f[2] == "1";
            n.text = f[3];
            u->notifications.pushBack(n);
        }
    }
    {
        std::ifstream in((dir + "/history.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 6) continue;
            User* u = users.findByEmail(f[0]);
            if (!u) continue;
            HistoryEntry h;
            h.ts = std::atoll(f[1].c_str());
            h.rideId = std::atoi(f[2].c_str());
            h.role = f[3];
            h.coTravelers = f[4];
            h.route = f[5];
            u->history.pushBack(h);  // file is written newest-first, keep that order
        }
    }
    {
        std::ifstream in((dir + "/ratings.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 6) continue;
            RatingRecord r;
            r.rideId = std::atoi(f[0].c_str());
            r.rater = f[1];
            r.ratee = f[2];
            r.stars = std::atoi(f[3].c_str());
            r.ts = std::atoll(f[4].c_str());
            r.comment = f[5];
            ratings.restore(r);
        }
    }
    {
        std::ifstream in((dir + "/bookings.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 4) continue;
            Ride* r = rides.findRide(std::atoi(f[0].c_str()));
            if (!r) continue;
            Booking b;
            b.passenger = f[1];
            b.pickup = f[2];
            b.dropoff = f[3];
            r->bookings.push(b);
        }
    }
    {
        std::ifstream in((dir + "/requests.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 5) continue;
            Ride* r = rides.findRide(std::atoi(f[0].c_str()));
            if (!r) continue;
            SeatRequest q;
            q.passenger = f[1];
            q.pickup = f[2];
            q.dropoff = f[3];
            q.ts = std::atoll(f[4].c_str());
            r->requests.enqueue(q);
        }
    }
    {
        std::ifstream in((dir + "/messages.csv").c_str());
        while (std::getline(in, line)) {
            if (util::splitCsv(line, f, 7) < 4) continue;
            Ride* r = rides.findRide(std::atoi(f[0].c_str()));
            if (!r) continue;
            ChatMessage m;
            m.ts = std::atoll(f[1].c_str());
            m.sender = f[2];
            m.text = f[3];
            r->chat.pushBack(m);
        }
    }
}

}  // namespace SocialStore
