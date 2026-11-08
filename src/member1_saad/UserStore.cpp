#include "member1_saad/UserStore.h"

#include <fstream>

#include "common/Utils.h"

namespace UserStore {

bool save(const UserManager& users, const std::string& path) {
    std::ofstream out(path.c_str());
    if (!out) return false;
    users.forEach([&](User* u) {
        out << util::csvEscape(u->email) << ',' << util::csvEscape(u->studentId) << ',' << util::csvEscape(u->name)
            << ',' << u->passHash << ',' << util::csvEscape(u->phone) << ',' << util::csvEscape(u->vehicle) << ','
            << (u->isAdmin ? 1 : 0) << ',' << (u->suspended ? 1 : 0) << '\n';
    });
    return true;
}

bool load(UserManager& users, const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (util::trim(line).empty()) continue;
        std::string f[8];
        if (util::splitCsv(line, f, 8) < 8) continue;  // skip corrupt rows
        User* u = new User();
        u->email = f[0];
        u->studentId = f[1];
        u->name = f[2];
        u->passHash = f[3];
        u->phone = f[4];
        u->vehicle = f[5];
        u->isAdmin = f[6] == "1";
        u->suspended = f[7] == "1";
        users.adopt(u);  // deletes u itself if the email is a duplicate
    }
    return true;
}

}  // namespace UserStore
