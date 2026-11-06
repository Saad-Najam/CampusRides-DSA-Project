#include "member1_saad/UserManager.h"

#include <cctype>

#include "common/Utils.h"

const char* UserManager::kDomain = "@nu.edu.pk";

UserManager::~UserManager() {
    // Both tables point at the same objects; delete through the email table only.
    byEmail_.forEach([](const std::string&, User*& u) { delete u; });
}

bool UserManager::validEmail(const std::string& email) {
    std::string e = util::toLower(email);
    if (!util::endsWith(e, kDomain)) return false;
    std::string local = e.substr(0, e.size() - std::string(kDomain).size());
    if (local.empty()) return false;
    for (size_t i = 0; i < local.size(); i++) {
        char c = local[i];
        if (!(std::isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

std::string UserManager::normalizeStudentId(const std::string& id) { return util::toUpper(util::trim(id)); }

bool UserManager::validStudentId(const std::string& rawId) {
    std::string id = normalizeStudentId(rawId);
    if (id.size() != 8) return false;
    return std::isdigit((unsigned char)id[0]) && std::isdigit((unsigned char)id[1]) && id[2] == 'K' && id[3] == '-' &&
           std::isdigit((unsigned char)id[4]) && std::isdigit((unsigned char)id[5]) &&
           std::isdigit((unsigned char)id[6]) && std::isdigit((unsigned char)id[7]);
}

User* UserManager::registerUser(const std::string& name, const std::string& rawEmail, const std::string& rawStudentId,
                                const std::string& password, const std::string& phone, std::string& err,
                                bool isAdmin) {
    std::string email = util::toLower(util::trim(rawEmail));
    std::string sid = normalizeStudentId(rawStudentId);
    if (util::trim(name).empty()) { err = "Name cannot be empty."; return nullptr; }
    if (!validEmail(email)) { err = "Email must be a valid university address ending in @nu.edu.pk."; return nullptr; }
    if (!isAdmin && !validStudentId(sid)) { err = "Student ID must look like 25K-0807."; return nullptr; }
    if (password.size() < 6) { err = "Password must be at least 6 characters."; return nullptr; }
    if (findByEmail(email)) { err = "An account with this email already exists."; return nullptr; }
    if (!isAdmin && findByStudentId(sid)) { err = "An account with this student ID already exists."; return nullptr; }

    User* u = new User();
    u->email = email;
    u->studentId = sid;
    u->name = util::trim(name);
    u->phone = util::trim(phone);
    u->passHash = util::hashPassword(password, email);
    u->isAdmin = isAdmin;
    adopt(u);
    return u;
}

bool UserManager::adopt(User* u) {
    if (!byEmail_.insert(u->email, u)) { delete u; return false; }
    if (!u->studentId.empty()) byStudentId_.insert(u->studentId, u);
    return true;
}

User* UserManager::login(const std::string& rawEmail, const std::string& password, std::string& err) {
    std::string email = util::toLower(util::trim(rawEmail));
    User* u = findByEmail(email);
    if (!u || u->passHash != util::hashPassword(password, email)) {
        err = "Invalid email or password.";
        return nullptr;
    }
    if (u->suspended) {
        err = "This account has been suspended. Contact an administrator.";
        return nullptr;
    }
    return u;
}

User* UserManager::findByEmail(const std::string& email) const {
    User** p = byEmail_.find(util::toLower(util::trim(email)));
    return p ? *p : nullptr;
}

User* UserManager::findByStudentId(const std::string& id) const {
    User** p = byStudentId_.find(normalizeStudentId(id));
    return p ? *p : nullptr;
}

bool UserManager::setSuspended(const std::string& email, bool value, std::string& err) {
    User* u = findByEmail(email);
    if (!u) { err = "No such user."; return false; }
    if (u->isAdmin) { err = "Administrators cannot be suspended."; return false; }
    u->suspended = value;
    return true;
}

bool UserManager::changePassword(User* u, const std::string& oldPw, const std::string& newPw, std::string& err) {
    if (u->passHash != util::hashPassword(oldPw, u->email)) { err = "Current password is incorrect."; return false; }
    if (newPw.size() < 6) { err = "Password must be at least 6 characters."; return false; }
    u->passHash = util::hashPassword(newPw, u->email);
    return true;
}

void UserManager::ensureAdminExists() {
    if (findByEmail("admin@nu.edu.pk")) return;
    std::string err;
    User* a = registerUser("System Admin", "admin@nu.edu.pk", "", "admin123", "", err, true);
    (void)a;
}
