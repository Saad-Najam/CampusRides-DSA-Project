// Member 2 (Ayaan Kashif) - tests for MinHeap, AVLTree, DoublyLinkedList, Graph, RideManager, RideMatcher, RideStore.
#include <cstdlib>

#include "TestFramework.h"
#include "common/Utils.h"
#include "member2_ayaan/AVLTree.h"
#include "member2_ayaan/CampusMap.h"
#include "member2_ayaan/DoublyLinkedList.h"
#include "member2_ayaan/Graph.h"
#include "member2_ayaan/MinHeap.h"
#include "member2_ayaan/RideManager.h"
#include "member2_ayaan/RideMatcher.h"
#include "member2_ayaan/RideStore.h"

struct IntLess {
    bool operator()(int a, int b) const { return a < b; }
};

static void testMinHeap() {
    SECTION("MinHeap");
    MinHeap<int, IntLess> h;
    int v;
    CHECK(h.empty() && !h.pop(v) && h.top() == nullptr);
    h.push(5);
    CHECK(h.size() == 1 && *h.top() == 5);
    CHECK(h.pop(v) && v == 5 && h.empty());

    std::srand(42);
    for (int i = 0; i < 10000; i++) h.push(std::rand() % 500);  // many duplicates
    int prev = -1;
    bool sorted = true;
    int n = 0;
    while (h.pop(v)) {
        sorted = sorted && v >= prev;
        prev = v;
        n++;
    }
    CHECK(sorted && n == 10000);
}

struct KeyLess {
    bool operator()(long long a, long long b) const { return a < b; }
};

static void testAVL() {
    SECTION("AVL / BST basics");
    AVLTree<long long, KeyLess> t;
    CHECK(t.empty() && !t.contains(1) && !t.remove(1));
    CHECK(t.insert(10) && t.insert(5) && t.insert(20));
    CHECK(!t.insert(10));  // duplicate ignored
    CHECK(t.size() == 3 && t.contains(5));

    SECTION("AVL stays balanced on sorted input (worst case for a plain BST)");
    AVLTree<long long, KeyLess> s;
    for (long long i = 0; i < 10000; i++) s.insert(i);
    CHECK(s.size() == 10000);
    CHECK(s.height() <= 18);  // 1.44 * log2(10000) ~ 19, perfect tree = 14
    long long prev = -1;
    bool sorted = true;
    s.forEach([&](long long x) { sorted = sorted && x > prev; prev = x; });
    CHECK(sorted);

    SECTION("AVL range query and removal");
    int cnt = 0;
    s.rangeQuery(100, 199, [&](long long) { cnt++; });
    CHECK(cnt == 100);
    cnt = 0;
    s.rangeQuery(20000, 30000, [&](long long) { cnt++; });  // nothing in range
    CHECK(cnt == 0);
    for (long long i = 0; i < 10000; i += 2) CHECK(s.remove(i));
    CHECK(s.size() == 5000 && !s.contains(4) && s.contains(5));
    CHECK(s.height() <= 17);
}

static void testDLL() {
    SECTION("DoublyLinkedList");
    DoublyLinkedList<int> l;
    CHECK(l.empty() && l.head() == nullptr);
    IntLess less;
    int vals[] = {5, 1, 9, 3, 7, 3};
    for (int i = 0; i < 6; i++) l.insertSorted(vals[i], less);
    CHECK(l.size() == 6);
    bool asc = true;
    for (auto* n = l.head(); n && n->next; n = n->next) asc = asc && n->data <= n->next->data;
    CHECK(asc);
    bool back = true;
    for (auto* n = l.tail(); n && n->prev; n = n->prev) back = back && n->data >= n->prev->data;
    CHECK(back);
    auto* mid = l.findIf([](int x) { return x == 7; });
    CHECK(mid != nullptr);
    l.remove(mid);
    l.remove(l.head());
    l.remove(l.tail());
    CHECK(l.size() == 3 && l.head()->data == 3 && l.tail()->data == 5);
    l.pushFront(0);
    l.pushBack(99);
    CHECK(l.head()->data == 0 && l.tail()->data == 99 && l.size() == 5);
}

static void testGraph() {
    SECTION("Graph / Dijkstra on the campus map");
    RideManager rm;
    const Graph& g = rm.map();
    int gul = g.indexOf("gulshan-e-iqbal"), campus = g.indexOf("Campus");
    CHECK(gul >= 0 && campus >= 0);
    DynArray<int> path;
    CHECK(g.shortestPath(gul, campus, &path) == 18);   // Gulshan -> Johar -> Shah Faisal -> Campus
    CHECK(path.size() == 4 && path[0] == gul && path[3] == campus);
    CHECK(g.shortestPath(campus, campus) == 0);
    CHECK(g.indexOf("Atlantis") == -1);
    CHECK(g.shortestPath(-1, 0) == -1);

    SECTION("Graph unreachable vertices and BFS");
    Graph h;
    int a = h.addZone("A"), b = h.addZone("B"), c = h.addZone("C");
    CHECK(h.addZone("a") == a);  // case-insensitive, no duplicate vertex
    h.addEdge(a, b, 3);
    CHECK(h.hasEdge(a, b) && !h.hasEdge(b, a));  // directed
    CHECK(h.reachable(a, b) && !h.reachable(b, a) && !h.reachable(a, c));
    CHECK(h.shortestPath(a, c) == -1);
    CHECK(!h.addEdge(a, a, 1) && !h.addEdge(a, b, 0));
    h.addEdge(a, b, 2);  // shorter parallel edge wins
    CHECK(h.shortestPath(a, b) == 2);
}

static DynArray<std::string> routeOf(const char* a, const char* b = nullptr, const char* c = nullptr, const char* d = nullptr) {
    DynArray<std::string> r;
    if (a) r.push(a);
    if (b) r.push(b);
    if (c) r.push(c);
    if (d) r.push(d);
    return r;
}

static void testRideManager() {
    RideManager rm;
    std::string err;
    SECTION("RideManager validation");
    CHECK(rm.createRide("d@nu.edu.pk", routeOf("Johar"), 202612010800LL, 3, "car", "", err) == nullptr);  // one stop
    CHECK(rm.createRide("d@nu.edu.pk", routeOf("Johar", "Atlantis"), 202612010800LL, 3, "car", "", err) == nullptr);
    CHECK(rm.createRide("d@nu.edu.pk", routeOf("Johar", "Campus", "johar"), 202612010800LL, 3, "car", "", err) == nullptr);  // repeat
    CHECK(rm.createRide("d@nu.edu.pk", routeOf("Johar", "Campus"), 202612010800LL, 0, "car", "", err) == nullptr);
    CHECK(rm.createRide("d@nu.edu.pk", routeOf("Johar", "Campus"), 202612010800LL, 9, "car", "", err) == nullptr);
    CHECK(rm.count() == 0);

    SECTION("RideManager indexes (hash, BST, linked list, heap)");
    Ride* late = rm.createRide("d@nu.edu.pk", routeOf("Johar", "Campus"), 202612030800LL, 3, "car", "", err);
    Ride* early = rm.createRide("d@nu.edu.pk", routeOf("Malir", "Campus"), 202612010800LL, 2, "car", "", err);
    Ride* mid = rm.createRide("e@nu.edu.pk", routeOf("DHA", "Korangi", "Campus"), 202612020800LL, 2, "car", "", err);
    CHECK(late && early && mid && rm.count() == 3);
    CHECK(late->id == 1 && early->id == 2 && mid->id == 3);
    CHECK(rm.findRide(2) == early && rm.findRide(99) == nullptr);
    CHECK(mid->route[0] == "DHA");  // canonical zone names

    int expected[] = {2, 3, 1};
    int i = 0;
    bool feedOrdered = true;
    rm.forEachRide([&](Ride* r) { feedOrdered = feedOrdered && r->id == expected[i++]; });
    CHECK(feedOrdered && i == 3);
    CHECK(rm.feed().tail()->data == late && rm.feed().head()->data == early);

    DynArray<Ride*> out;
    rm.rangeByTime(202612020000LL, 202612022359LL, out);   // BST range: only Dec 2
    CHECK(out.size() == 1 && out[0] == mid);
    rm.rangeByTime(202612010000LL, 202612032359LL, out);
    CHECK(out.size() == 3);
    rm.rangeByTime(202701010000LL, 202701012359LL, out);
    CHECK(out.empty());

    mid->status = RideStatus::Cancelled;
    rm.upcomingOpen(0, out);   // heap: earliest first, skips cancelled
    CHECK(out.size() == 2 && out[0] == early && out[1] == late);
    rm.upcomingOpen(1, out);
    CHECK(out.size() == 1 && out[0] == early);

    rm.ridesOfDriver("d@nu.edu.pk", out);
    CHECK(out.size() == 2);
    CHECK(rm.deleteRide(2) && !rm.deleteRide(2) && rm.count() == 2 && rm.feed().size() == 2);

    SECTION("RideManager stress: 3,000 rides");
    RideManager big;
    std::srand(7);
    for (int k = 0; k < 3000; k++) {
        long long t = 202612010000LL + (std::rand() % 28) * 10000LL + (std::rand() % 24) * 100LL;
        big.createRide("d@nu.edu.pk", routeOf("Johar", "Campus"), t, 3, "car", "", err);
    }
    CHECK(big.count() == 3000);
    long long prev = 0;
    bool sorted = true;
    big.forEachRide([&](Ride* r) { sorted = sorted && r->departKey >= prev; prev = r->departKey; });
    CHECK(sorted);
}

static void testMatcher() {
    RideManager rm;
    std::string err;
    Ride* r = rm.createRide("d@nu.edu.pk", routeOf("Gulshan-e-Iqbal", "Johar", "Shah Faisal", "Campus"), 202612010730LL, 2, "car", "", err);
    MatchResult m;

    SECTION("Exact route overlap");
    CHECK(RideMatcher::matchRide(rm.map(), *r, "Johar", "Campus", 5, m));
    CHECK(m.walkKm == 0 && m.pickupIdx == 1 && m.dropIdx == 3 && m.sharedKm == 14);   // 6 + 8
    CHECK(RideMatcher::matchRide(rm.map(), *r, "johar", "shah faisal", 0, m) && m.sharedKm == 6);

    SECTION("Direction matters, nonsense input rejected");
    CHECK(!RideMatcher::matchRide(rm.map(), *r, "Campus", "Johar", 0, m));   // wrong direction on exact stops
    CHECK(!RideMatcher::matchRide(rm.map(), *r, "Johar", "Johar", 5, m));
    CHECK(!RideMatcher::matchRide(rm.map(), *r, "Atlantis", "Campus", 5, m));

    SECTION("Near matches use graph distance");
    CHECK(!RideMatcher::matchRide(rm.map(), *r, "North Nazimabad", "Campus", 5, m));   // 8 km from the route
    CHECK(RideMatcher::matchRide(rm.map(), *r, "North Nazimabad", "Campus", 10, m));
    CHECK(m.walkKm == 8 && m.pickupIdx == 0);

    SECTION("Search ranking, filters");
    Ride* nearRide = rm.createRide("e@nu.edu.pk", routeOf("Gulshan-e-Iqbal", "Johar", "Campus"), 202612010700LL, 3, "car", "", err);
    Ride* exact = rm.createRide("f@nu.edu.pk", routeOf("North Nazimabad", "Gulshan-e-Iqbal", "Campus"), 202612020700LL, 3, "car", "", err);
    (void)nearRide;
    DynArray<MatchResult> res;
    RideMatcher::search(rm, "North Nazimabad", "Campus", -1, -1, 10, "", res);
    CHECK(res.size() == 3);
    CHECK(res.size() > 0 && res[0].ride == exact && res[0].walkKm == 0);   // zero walking beats earlier departure
    CHECK(res.size() == 3 && res[1].ride->departKey <= res[2].ride->departKey);   // equal walk -> earliest first

    RideMatcher::search(rm, "North Nazimabad", "Campus", 202612020000LL, 202612022359LL, 10, "", res);
    CHECK(res.size() == 1 && res[0].ride == exact);                       // date filter through the BST
    RideMatcher::search(rm, "North Nazimabad", "Campus", -1, -1, 10, "f@nu.edu.pk", res);
    CHECK(res.size() == 2);                                               // own rides excluded
    r->seatsAvailable = 0;
    exact->status = RideStatus::Cancelled;
    RideMatcher::search(rm, "North Nazimabad", "Campus", -1, -1, 10, "", res);
    CHECK(res.size() == 1);                                               // full / cancelled rides hidden
}

static void testRideStore() {
    SECTION("RideStore save/load round trip");
    util::makeDir("test_data_rides");
    RideManager a;
    std::string err;
    Ride* r = a.createRide("d@nu.edu.pk", routeOf("DHA", "Korangi", "Campus"), 202612020800LL, 3, "Honda, City", "no, smoking\nplease", err);
    r->seatsAvailable = 1;
    r->status = RideStatus::Completed;
    a.createRide("e@nu.edu.pk", routeOf("Malir", "Campus"), 202612010800LL, 2, "car", "", err);
    CHECK(RideStore::save(a, "test_data_rides/rides.csv"));

    RideManager b;
    CHECK(RideStore::load(b, "test_data_rides/rides.csv"));
    CHECK(b.count() == 2);
    Ride* r2 = b.findRide(1);
    CHECK(r2 && r2->route.size() == 3 && r2->route[1] == "Korangi" && r2->seatsAvailable == 1);
    CHECK(r2 && r2->status == RideStatus::Completed && r2->vehicle == "Honda, City" && r2->notes == "no, smoking\nplease");
    CHECK(b.feed().head()->data->id == 2);   // re-sorted by departure
    Ride* r3 = b.createRide("x@nu.edu.pk", routeOf("Malir", "Campus"), 202612050800LL, 2, "car", "", err);
    CHECK(r3 && r3->id == 3);                // id counter continues after load
}

void runAyaanTests() {
    testMinHeap();
    testAVL();
    testDLL();
    testGraph();
    testRideManager();
    testMatcher();
    testRideStore();
}
