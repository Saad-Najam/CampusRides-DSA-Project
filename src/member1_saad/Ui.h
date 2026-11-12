// Declarations of every console flow, grouped by the member who implements them.
#pragma once
#include "member1_saad/Context.h"

// ---- Member 1 (Saad): accounts, profile, application shell --------------------------------
void registerFlow(Context& ctx);
bool loginFlow(Context& ctx);
void profileMenu(Context& ctx);
void runApp(Context& ctx);
void seedDemoData(Context& ctx);

// ---- Member 2 (Ayaan): posting, searching and browsing rides --------------------------------
void printRideLine(const Context& ctx, const Ride* r);
void printRideDetails(const Context& ctx, const Ride* r);
void postRideFlow(Context& ctx);
void searchRidesFlow(Context& ctx);
void browseFeedFlow(Context& ctx);
void upcomingRidesFlow(Context& ctx);

// ---- Member 3 (Shamaem): requests, chat, notifications, ratings, history, admin --------------
void passengerBookingsFlow(Context& ctx);
void driverRidesFlow(Context& ctx);
void chatFlow(Context& ctx);
void inboxFlow(Context& ctx);
void ratingsFlow(Context& ctx);
void historyFlow(Context& ctx);
void adminMenu(Context& ctx);
