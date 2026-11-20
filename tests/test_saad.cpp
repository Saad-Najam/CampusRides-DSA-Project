// Member 1 (Syed Saad Najam) - tests for HashTable, Stack, Utils, UserManager, UserStore.

#include "TestFramework.h"
#include "common/Utils.h"
#include "member1_saad/HashTable.h"
#include "member1_saad/Stack.h"
#include "member1_saad/UserManager.h"
#include "member1_saad/UserStore.h"

static void testHashTable() {
    SECTION("HashTable basics");
    HashTable<int> h;
    CHECK(h.size() == 0);
    CHECK(h.find("x") == nullptr);  // empty table
    CHECK(h.insert("a", 1));
    CHECK(!h.insert("a", 2));       // duplicate key rejected
    CHECK(h.find("a") && *h.find("a") == 1);
    CHECK(h.remove("a"));
    CHECK(!h.remove("a"));
    CHECK(h.size() == 0);

    SECTION("HashTable collisions (tiny table) and resize");
    HashTable<int> tiny(1);
    for (int i = 0; i < 100; i++) CHECK(tiny.insert(util::toStr(i), i));
    bool allFound = true;
    for (int i = 0; i < 100; i++) allFound = allFound && tiny.find(util::toStr(i)) && *tiny.find(util::toStr(i)) == i;
    CHECK(allFound);
    CHECK(tiny.bucketCount() > 1);

    SECTION("HashTable stress: 50,000 keys");
    HashTable<int> big;
    for (int i = 0; i < 50000; i++) big.insert("key" + util::toStr(i), i);
    CHECK(big.size() == 50000);
    bool ok = true;
    for (int i = 0; i < 50000; i += 7) ok = ok && big.find("key" + util::toStr(i)) && *big.find("key" + util::toStr(i)) == i;
    CHECK(ok);
    for (int i = 0; i < 50000; i += 2) big.remove("key" + util::toStr(i));
    CHECK(big.size() == 25000);
    CHECK(big.find("key2") == nullptr && big.find("key3") != nullptr);
    int visited = 0;
    big.forEach([&](const std::string&, int&) { visited++; });
    CHECK(visited == 25000);
}

static void testStack() {
    SECTION("Stack LIFO");
    Stack<int> s;
    int v = 0;
    CHECK(s.empty());
    CHECK(!s.pop(v));
    CHECK(s.top() == nullptr);
    for (int i = 1; i <= 100; i++) s.push(i);
    CHECK(s.size() == 100 && *s.top() == 100);
    bool order = true;
    for (int i = 100; i >= 1; i--) order = order && s.pop(v) && v == i;
    CHECK(order && s.empty());
}

static void testUtils() {
    SECTION("CSV escaping round trip");
    std::string nasty = "a,b\\c\nd,,";
    std::string line = util::csvEscape(nasty) + "," + util::csvEscape("plain");
    std::string f[3];
    CHECK(util::splitCsv(line, f, 3) == 2);
    CHECK(f[0] == nasty && f[1] == "plain");

    SECTION("Date parsing");
    CHECK(util::parseDateTime("2026-12-01 07:30") == 202612010730LL);
    CHECK(util::parseDateTime("2026-02-30 07:30") == -1);   // no such day
    CHECK(util::parseDateTime("2026-12-01 24:00") == -1);
    CHECK(util::parseDateTime("2026-12-01") == -1);
    CHECK(util::parseDateTime("garbage") == -1);
    CHECK(util::parseDateTime("2028-02-29 10:00") != -1);   // leap year
    CHECK(util::formatDateTime(202612010730LL) == "2026-12-01 07:30");
    long long lo, hi;
    CHECK(util::parseDate("2026-12-01", lo, hi) && lo == 202612010000LL && hi == 202612012359LL);

    SECTION("Password hashing");
    CHECK(util::hashPassword("secret1", "a@nu.edu.pk") == util::hashPassword("secret1", "a@nu.edu.pk"));
    CHECK(util::hashPassword("secret1", "a@nu.edu.pk") != util::hashPassword("secret2", "a@nu.edu.pk"));
    CHECK(util::hashPassword("secret1", "a@nu.edu.pk") != util::hashPassword("secret1", "b@nu.edu.pk"));
}

static void testUsers() {
    SECTION("Registration validation");
    UserManager um;
    std::string err;
    CHECK(um.registerUser("Saad", "k250807@nu.edu.pk", "25K-0807", "secret1", "0300", err) != nullptr);
    CHECK(um.registerUser("Dup", "K250807@NU.EDU.PK", "25K-9999", "secret1", "", err) == nullptr);   // duplicate email (case-insens.)
    CHECK(um.registerUser("Dup", "other@nu.edu.pk", "25k-0807", "secret1", "", err) == nullptr);      // duplicate student id
    CHECK(um.registerUser("Bad", "someone@gmail.com", "25K-1111", "secret1", "", err) == nullptr);
    CHECK(err.find("nu.edu.pk") != std::string::npos);
    CHECK(um.registerUser("Bad", "@nu.edu.pk", "25K-1111", "secret1", "", err) == nullptr);
    CHECK(um.registerUser("Bad", "a b@nu.edu.pk", "25K-1111", "secret1", "", err) == nullptr);
    CHECK(um.registerUser("Bad", "x@nu.edu.pk", "1234", "secret1", "", err) == nullptr);
    CHECK(um.registerUser("Bad", "x@nu.edu.pk", "25K-1111", "123", "", err) == nullptr);              // short password
    CHECK(um.registerUser("  ", "x@nu.edu.pk", "25K-1111", "secret1", "", err) == nullptr);           // blank name
    CHECK(um.count() == 1);

    SECTION("Lookup by email and student id");
    CHECK(um.findByEmail("K250807@nu.edu.pk") != nullptr);
    CHECK(um.findByStudentId("25k-0807") == um.findByEmail("k250807@nu.edu.pk"));
    CHECK(um.findByEmail("nobody@nu.edu.pk") == nullptr);

    SECTION("Login / suspension / password change");
    CHECK(um.login("k250807@nu.edu.pk", "secret1", err) != nullptr);
    CHECK(um.login("k250807@nu.edu.pk", "wrong", err) == nullptr);
    CHECK(um.login("nobody@nu.edu.pk", "secret1", err) == nullptr);
    CHECK(um.setSuspended("k250807@nu.edu.pk", true, err));
    CHECK(um.login("k250807@nu.edu.pk", "secret1", err) == nullptr);
    CHECK(err.find("suspended") != std::string::npos);
    CHECK(um.setSuspended("k250807@nu.edu.pk", false, err));
    User* u = um.findByEmail("k250807@nu.edu.pk");
    CHECK(!um.changePassword(u, "bad", "newpass1", err));
    CHECK(um.changePassword(u, "secret1", "newpass1", err));
    CHECK(um.login("k250807@nu.edu.pk", "newpass1", err) != nullptr);

    SECTION("Admin account cannot be suspended");
    um.ensureAdminExists();
    um.ensureAdminExists();  // idempotent
    CHECK(um.count() == 2);
    CHECK(!um.setSuspended("admin@nu.edu.pk", true, err));

    SECTION("Stress: 5,000 users");
    UserManager many;
    for (int i = 0; i < 5000; i++) {
        char idbuf[16], mail[48];
        std::snprintf(idbuf, sizeof idbuf, "25K-%04d", i);
        std::snprintf(mail, sizeof mail, "k25%04d@nu.edu.pk", i);
        many.registerUser("User " + util::toStr(i), mail, idbuf, "password", "", err);
    }
    CHECK(many.count() == 5000);
    CHECK(many.findByStudentId("25K-4999") != nullptr);
    CHECK(many.login("k254999@nu.edu.pk", "password", err) != nullptr);
}

static void testUserStore() {
    SECTION("UserStore save/load round trip");
    util::makeDir("test_data_users");
    UserManager a;
    std::string err;
    User* u = a.registerUser("Comma, Name", "k250807@nu.edu.pk", "25K-0807", "secret1", "0300-1", err);
    u->vehicle = "Honda, City";
    a.setSuspended("k250807@nu.edu.pk", true, err);
    CHECK(UserStore::save(a, "test_data_users/users.csv"));

    UserManager b;
    CHECK(UserStore::load(b, "test_data_users/users.csv"));
    User* r = b.findByEmail("k250807@nu.edu.pk");
    CHECK(r != nullptr);
    CHECK(r && r->name == "Comma, Name" && r->vehicle == "Honda, City" && r->suspended);
    CHECK(r && r->passHash == u->passHash);
    CHECK(b.findByStudentId("25K-0807") == r);

    UserManager c;
    CHECK(!UserStore::load(c, "test_data_users/does_not_exist.csv"));
}

void runSaadTests() {
    testHashTable();
    testStack();
    testUtils();
    testUsers();
    testUserStore();
}
