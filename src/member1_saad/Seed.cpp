// Member 1 (Syed Saad Najam) - demo data so the app can be shown straight away.
#include <iostream>

#include "common/Utils.h"
#include "member1_saad/Ui.h"

void seedDemoData(Context& ctx) {
    struct Student { const char* name; const char* email; const char* id; };
    static const Student students[] = {
        {"Syed Saad Najam", "k250807@nu.edu.pk", "25K-0807"},
        {"Ayaan Kashif", "k250572@nu.edu.pk", "25K-0572"},
        {"Shamaem Saqib", "k250692@nu.edu.pk", "25K-0692"},
        {"Hira Malik", "k251101@nu.edu.pk", "25K-1101"},
        {"Bilal Ahmed", "k251202@nu.edu.pk", "25K-1202"},
    };
    std::string err;
    for (size_t i = 0; i < sizeof(students) / sizeof(students[0]); i++)
        ctx.users.registerUser(students[i].name, students[i].email, students[i].id, "pass1234", "0300-0000000", err);
    ctx.users.findByEmail("k250807@nu.edu.pk")->vehicle = "Honda City - KAB-123";
    ctx.users.findByEmail("k250572@nu.edu.pk")->vehicle = "Suzuki Cultus - BQR-456";

    DynArray<std::string> r1;
    r1.push("Gulshan-e-Iqbal"); r1.push("Johar"); r1.push("Shah Faisal"); r1.push("Campus");
    ctx.rides.createRide("k250807@nu.edu.pk", r1, util::parseDateTime("2026-12-01 07:30"), 3, "Honda City - KAB-123",
                         "Leaving sharp at 7:30", err);

    DynArray<std::string> r2;
    r2.push("DHA"); r2.push("Korangi"); r2.push("Campus");
    ctx.rides.createRide("k250572@nu.edu.pk", r2, util::parseDateTime("2026-12-01 08:00"), 2, "Suzuki Cultus - BQR-456",
                         "AC available", err);

    DynArray<std::string> r3;
    r3.push("North Nazimabad"); r3.push("Gulshan-e-Iqbal"); r3.push("Johar"); r3.push("Campus");
    ctx.rides.createRide("k250807@nu.edu.pk", r3, util::parseDateTime("2026-12-02 07:45"), 3, "Honda City - KAB-123", "", err);

    Ride* ride1 = ctx.rides.findRide(1);
    ctx.requests.requestSeat(ride1, ctx.users.findByEmail("k251101@nu.edu.pk"), "Johar", "Campus", err);
    ctx.requests.requestSeat(ride1, ctx.users.findByEmail("k251202@nu.edu.pk"), "Gulshan-e-Iqbal", "Campus", err);

    std::cout << "Demo data created. Log in as k250807@nu.edu.pk / pass1234 (driver of rides #1 and #3),\n"
                 "k251101@nu.edu.pk (passenger), or the administrator admin@nu.edu.pk / admin123.\n";
}
