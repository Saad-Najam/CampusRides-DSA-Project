#include "common/Utils.h"

#include <cctype>
#include <cstdio>
#include <ctime>
#include <iostream>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace util {

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

std::string toLower(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)std::tolower((unsigned char)s[i]);
    return s;
}

std::string toUpper(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)std::toupper((unsigned char)s[i]);
    return s;
}

bool equalsIgnoreCase(const std::string& a, const std::string& b) { return toLower(a) == toLower(b); }

bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string toStr(long long v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    unsigned long long u = neg ? 0ULL - (unsigned long long)v : (unsigned long long)v;
    std::string s;
    while (u > 0) {
        s.insert(s.begin(), (char)('0' + (int)(u % 10)));
        u /= 10;
    }
    return neg ? "-" + s : s;
}

static std::string pad(long long v, int width) {
    std::string s = toStr(v);
    while ((int)s.size() < width) s.insert(s.begin(), '0');
    return s;
}

std::string csvEscape(const std::string& s) {
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '\\') o += "\\\\";
        else if (c == ',') o += "\\c";
        else if (c == '\n') o += "\\n";
        else if (c == '\r') continue;
        else o += c;
    }
    return o;
}

std::string csvUnescape(const std::string& s) {
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            if (n == 'c') o += ',';
            else if (n == 'n') o += '\n';
            else o += n;
        } else {
            o += s[i];
        }
    }
    return o;
}

int splitCsv(const std::string& rawLine, std::string out[], int maxFields) {
    std::string line = rawLine;
    while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n')) line.erase(line.size() - 1);
    int n = 0;
    std::string cur;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ',') {
            if (n < maxFields) out[n++] = csvUnescape(cur);
            cur.clear();
        } else {
            cur += line[i];
        }
    }
    if (n < maxFields) out[n++] = csvUnescape(cur);
    return n;
}

unsigned long long hash64(const std::string& s) {
    unsigned long long h = 1469598103934665603ULL;
    for (size_t i = 0; i < s.size(); i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

// Salted, iterated FNV hash. Good enough for a coursework project; a real system would use bcrypt/argon2.
std::string hashPassword(const std::string& pw, const std::string& salt) {
    unsigned long long h = hash64(salt + ":" + pw);
    for (int i = 0; i < 2000; i++) h = hash64(toStr((long long)(h & 0x7fffffffffffffffULL)) + salt);
    static const char* digits = "0123456789abcdef";
    std::string out(16, '0');
    for (int i = 15; i >= 0; i--) {
        out[i] = digits[h & 0xF];
        h >>= 4;
    }
    return out;
}

static bool validDate(int y, int m, int d) {
    if (y < 2000 || y > 2100 || m < 1 || m > 12 || d < 1) return false;
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxd = dim[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) maxd = 29;
    return d <= maxd;
}

long long parseDateTime(const std::string& raw) {
    std::string s = trim(raw);
    int y, mo, d, h, mi, n = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d %d:%d%n", &y, &mo, &d, &h, &mi, &n) != 5) return -1;
    if (n != (int)s.size()) return -1;
    if (!validDate(y, mo, d) || h < 0 || h > 23 || mi < 0 || mi > 59) return -1;
    return ((((long long)y * 100 + mo) * 100 + d) * 100 + h) * 100 + mi;
}

bool parseDate(const std::string& raw, long long& startKey, long long& endKey) {
    std::string s = trim(raw);
    int y, mo, d, n = 0;
    if (std::sscanf(s.c_str(), "%d-%d-%d%n", &y, &mo, &d, &n) != 3) return false;
    if (n != (int)s.size() || !validDate(y, mo, d)) return false;
    startKey = (((long long)y * 100 + mo) * 100 + d) * 10000;
    endKey = startKey + 2359;
    return true;
}

std::string formatDateTime(long long key) {
    long long mi = key % 100, h = (key / 100) % 100, d = (key / 10000) % 100, mo = (key / 1000000) % 100,
              y = key / 100000000;
    return pad(y, 4) + "-" + pad(mo, 2) + "-" + pad(d, 2) + " " + pad(h, 2) + ":" + pad(mi, 2);
}

long long nowKey() {
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    return ((((long long)(lt->tm_year + 1900) * 100 + lt->tm_mon + 1) * 100 + lt->tm_mday) * 100 + lt->tm_hour) * 100 +
           lt->tm_min;
}

bool makeDir(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

}  // namespace util

namespace ui {

std::string ask(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string s;
    if (!std::getline(std::cin, s)) throw InputClosed();
    return util::trim(s);
}

int askInt(const std::string& prompt, int lo, int hi) {
    for (;;) {
        std::string s = ask(prompt);
        int v, n = 0;
        if (std::sscanf(s.c_str(), "%d%n", &v, &n) == 1 && n == (int)s.size() && v >= lo && v <= hi) return v;
        std::cout << "  Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

bool askYesNo(const std::string& prompt) {
    for (;;) {
        std::string s = util::toLower(ask(prompt + " (y/n): "));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no") return false;
    }
}

void pause() { ask("\nPress Enter to continue..."); }

void line(char c, int n) { std::cout << std::string(n, c) << "\n"; }

void title(const std::string& t) {
    std::cout << "\n";
    line('=');
    std::cout << "  " << t << "\n";
    line('=');
}

}  // namespace ui
