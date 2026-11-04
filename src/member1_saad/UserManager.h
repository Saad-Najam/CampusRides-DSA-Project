// Member 1 (Syed Saad Najam) - Registration, authentication and profile lookup.
// Two hash tables index the same User objects: by email and by student ID.
#pragma once
#include <string>

#include "member1_saad/HashTable.h"
#include "member1_saad/User.h"

class UserManager {
    HashTable<User*> byEmail_;
    HashTable<User*> byStudentId_;

public:
    UserManager() {}
    ~UserManager();
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;

    static const char* kDomain;  // "@nu.edu.pk"
    static bool validEmail(const std::string& email);
    static bool validStudentId(const std::string& id);   // 25K-0807 style
    static std::string normalizeStudentId(const std::string& id);

    // Returns nullptr and fills err on failure.
    User* registerUser(const std::string& name, const std::string& email, const std::string& studentId,
                       const std::string& password, const std::string& phone, std::string& err,
                       bool isAdmin = false);
    User* login(const std::string& email, const std::string& password, std::string& err);

    User* findByEmail(const std::string& email) const;
    User* findByStudentId(const std::string& id) const;
    int count() const { return byEmail_.size(); }

    bool setSuspended(const std::string& email, bool value, std::string& err);
    bool changePassword(User* u, const std::string& oldPw, const std::string& newPw, std::string& err);

    // Takes ownership of an already-built user (used by the file loader).
    bool adopt(User* u);
    void ensureAdminExists();

    template <typename F>
    void forEach(F f) const {  // f(User*)
        byEmail_.forEach([&](const std::string&, User* const& u) { f(u); });
    }
};
