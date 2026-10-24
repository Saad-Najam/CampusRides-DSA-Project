#include "member2_ayaan/CampusMap.h"

const char* const kCampusZone = "Campus";

void buildCampusMap(Graph& g) {
    g.addZone(kCampusZone);
    struct Road { const char* a; const char* b; int km; };
    static const Road roads[] = {
        {"Campus", "Landhi", 6},
        {"Campus", "Korangi", 9},
        {"Campus", "Shah Faisal", 8},
        {"Campus", "Malir", 10},
        {"Landhi", "Korangi", 6},
        {"Landhi", "Malir", 8},
        {"Malir", "Shah Faisal", 5},
        {"Malir", "Johar", 9},
        {"Shah Faisal", "Johar", 6},
        {"Shah Faisal", "Korangi", 7},
        {"Johar", "Gulshan-e-Iqbal", 4},
        {"Gulshan-e-Iqbal", "North Nazimabad", 8},
        {"Gulshan-e-Iqbal", "Nazimabad", 7},
        {"North Nazimabad", "Nazimabad", 3},
        {"Nazimabad", "Saddar", 8},
        {"Korangi", "DHA", 8},
        {"DHA", "Clifton", 6},
        {"Clifton", "Saddar", 7},
        {"DHA", "Saddar", 11},
        {"Gulshan-e-Iqbal", "Saddar", 12},
    };
    for (size_t i = 0; i < sizeof(roads) / sizeof(roads[0]); i++) g.addRoad(roads[i].a, roads[i].b, roads[i].km);
}
