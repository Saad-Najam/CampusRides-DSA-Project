// Member 1 (Syed Saad Najam) - registration, login and profile management screens.
#include <iostream>

#include "common/Utils.h"
#include "member1_saad/Ui.h"

void registerFlow(Context& ctx) {
    NavScope nav(ctx, "Register");
    ui::title("Create your Campus Rides account");
    std::cout << "Only NUCES students can join: use your @nu.edu.pk email.\n\n";
    std::string name = ui::ask("Full name: ");
    std::string email = ui::ask("University email (e.g. k250807@nu.edu.pk): ");
    std::string sid = ui::ask("Student ID (e.g. 25K-0807): ");
    std::string phone = ui::ask("Phone (optional): ");
    std::string pw = ui::ask("Password (min 6 chars): ");
    if (pw != ui::ask("Confirm password: ")) {
        std::cout << "\nPasswords do not match.\n";
        return;
    }
    std::string err;
    User* u = ctx.users.registerUser(name, email, sid, pw, phone, err);
    if (!u) std::cout << "\nRegistration failed: " << err << "\n";
    else std::cout << "\nWelcome, " << u->name << "! You can now log in.\n";
}

bool loginFlow(Context& ctx) {
    NavScope nav(ctx, "Login");
    ui::title("Login");
    std::string email = ui::ask("Email: ");
    std::string pw = ui::ask("Password: ");
    std::string err;
    User* u = ctx.users.login(email, pw, err);
    if (!u) {
        std::cout << "\n" << err << "\n";
        return false;
    }
    ctx.current = u;
    ctx.role = Role::Passenger;
    std::cout << "\nLogged in as " << u->name << ".\n";
    return true;
}

void profileMenu(Context& ctx) {
    NavScope nav(ctx, "Profile");
    User* u = ctx.current;
    for (;;) {
        ui::title("My Profile");
        printBreadcrumb(ctx);
        std::cout << "Name:        " << u->name << "\n"
                  << "Email:       " << u->email << "\n"
                  << "Student ID:  " << u->studentId << "\n"
                  << "Phone:       " << (u->phone.empty() ? "-" : u->phone) << "\n"
                  << "Vehicle:     " << (u->vehicle.empty() ? "-" : u->vehicle) << "\n";
        if (u->ratingCount) std::cout << "Rating:      " << u->avgRating() << " / 5 (" << u->ratingCount << " reviews)\n";
        else std::cout << "Rating:      no reviews yet\n";
        std::cout << "\n1. Edit name\n2. Edit phone\n3. Edit vehicle details (drivers)\n4. Change password\n0. Back\n";
        int c = ui::askInt("> ", 0, 4);
        std::string err;
        if (c == 0) return;
        if (c == 1) {
            std::string v = ui::ask("New name: ");
            if (v.empty()) std::cout << "Name cannot be empty.\n";
            else u->name = v;
        } else if (c == 2) {
            u->phone = ui::ask("New phone: ");
        } else if (c == 3) {
            u->vehicle = ui::ask("Vehicle (make/model/plate): ");
        } else {
            std::string oldPw = ui::ask("Current password: ");
            std::string newPw = ui::ask("New password: ");
            if (ctx.users.changePassword(u, oldPw, newPw, err)) std::cout << "Password updated.\n";
            else std::cout << err << "\n";
        }
    }
}
