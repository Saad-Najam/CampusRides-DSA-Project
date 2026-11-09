#include "member2_ayaan/RideStore.h"

#include <cstdlib>
#include <fstream>

#include "common/Utils.h"

namespace RideStore {

bool save(const RideManager& rm, const std::string& path) {
    std::ofstream out(path.c_str());
    if (!out) return false;
    rm.forEachRide([&](Ride* r) {
        std::string route;
        for (int i = 0; i < r->route.size(); i++) {
            if (i) route += '>';
            route += r->route[i];
        }
        out << r->id << ',' << util::csvEscape(r->driver) << ',' << util::csvEscape(route) << ',' << r->departKey
            << ',' << r->totalSeats << ',' << r->seatsAvailable << ',' << util::csvEscape(r->vehicle) << ','
            << util::csvEscape(r->notes) << ',' << statusName(r->status) << '\n';
    });
    return true;
}

bool load(RideManager& rm, const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (util::trim(line).empty()) continue;
        std::string f[9];
        if (util::splitCsv(line, f, 9) < 9) continue;
        Ride* r = new Ride();
        r->id = std::atoi(f[0].c_str());
        r->driver = f[1];
        std::string cur;
        for (size_t i = 0; i <= f[2].size(); i++) {
            if (i == f[2].size() || f[2][i] == '>') {
                if (!cur.empty()) r->route.push(cur);
                cur.clear();
            } else {
                cur += f[2][i];
            }
        }
        r->departKey = std::atoll(f[3].c_str());
        r->totalSeats = std::atoi(f[4].c_str());
        r->seatsAvailable = std::atoi(f[5].c_str());
        r->vehicle = f[6];
        r->notes = f[7];
        r->status = parseStatus(f[8]);
        rm.adoptRide(r);  // deletes r itself on a duplicate id
    }
    return true;
}

}  // namespace RideStore
