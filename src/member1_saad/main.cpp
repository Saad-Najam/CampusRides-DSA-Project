// Member 1 (Syed Saad Najam) - program entry point.
// Usage: campusrides [--data <dir>] [--seed]
//   --seed  fills an empty database with demo students and rides (password for all demo users: pass1234)
#include <cstring>
#include <iostream>

#include "common/Utils.h"
#include "member1_saad/Ui.h"

int main(int argc, char** argv) {
    std::string dir = "data";
    bool seed = false;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--seed")) seed = true;
        else if (!std::strcmp(argv[i], "--data") && i + 1 < argc) dir = argv[++i];
        else {
            std::cout << "Usage: campusrides [--data <dir>] [--seed]\n";
            return 1;
        }
    }

    Context ctx(dir);
    ctx.load();
    if (seed) {
        if (ctx.users.count() > 1) std::cout << "Database is not empty - skipping demo data.\n";
        else seedDemoData(ctx);
    }

    try {
        runApp(ctx);
        std::cout << "\nGoodbye!\n";
    } catch (const ui::InputClosed&) {
        std::cout << "\nInput closed - saving and exiting.\n";
    }
    ctx.save();
    return 0;
}
