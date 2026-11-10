#include "member1_saad/Context.h"

#include <iostream>

#include "common/Utils.h"
#include "member1_saad/UserStore.h"
#include "member2_ayaan/RideStore.h"
#include "member3_shamaem/SocialStore.h"

void Context::load() {
    UserStore::load(users, dataDir + "/users.csv");
    users.ensureAdminExists();
    RideStore::load(rides, dataDir + "/rides.csv");
    SocialStore::load(users, rides, ratings, dataDir);
}

void Context::save() {
    util::makeDir(dataDir);  // no-op if it already exists
    bool ok = UserStore::save(users, dataDir + "/users.csv") && RideStore::save(rides, dataDir + "/rides.csv") &&
              SocialStore::save(users, rides, ratings, dataDir);
    if (!ok) std::cerr << "Warning: could not write data files to '" << dataDir << "'.\n";
}

void printBreadcrumb(const Context& ctx) {
    std::cout << "  [";
    for (int i = 0; i < ctx.nav.size(); i++) std::cout << (i ? " > " : "") << ctx.nav.at(i);
    std::cout << "]\n";
}
