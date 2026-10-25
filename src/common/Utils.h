// Shared helpers: strings, CSV escaping, hashing, date/time, console input.
// STL is used here only for I/O and string utilities (allowed by the project rules).
#pragma once
#include <string>

namespace util {

std::string trim(const std::string& s);
std::string toLower(std::string s);
std::string toUpper(std::string s);
bool equalsIgnoreCase(const std::string& a, const std::string& b);
bool containsIgnoreCase(const std::string& haystack, const std::string& needle);
bool endsWith(const std::string& s, const std::string& suffix);
std::string toStr(long long v);

// CSV helpers: commas / newlines inside fields are escaped so every record is one line.
std::string csvEscape(const std::string& s);
std::string csvUnescape(const std::string& s);
int splitCsv(const std::string& line, std::string out[], int maxFields);

// Hashing
unsigned long long hash64(const std::string& s);          // FNV-1a
std::string hashPassword(const std::string& pw, const std::string& salt);

// Date/time keys are YYYYMMDDHHMM stored in a long long, so they sort chronologically.
long long parseDateTime(const std::string& s);            // "YYYY-MM-DD HH:MM" -> key, -1 if invalid
bool parseDate(const std::string& s, long long& startKey, long long& endKey); // "YYYY-MM-DD"
std::string formatDateTime(long long key);
long long nowKey();

// Creates a single directory level; returns false if it already exists or cannot be created.
bool makeDir(const std::string& path);

}  // namespace util

namespace ui {

// Thrown when stdin is closed so that menus can unwind cleanly instead of looping forever.
struct InputClosed {};

std::string ask(const std::string& prompt);
int askInt(const std::string& prompt, int lo, int hi);
bool askYesNo(const std::string& prompt);
void pause();
void line(char c = '-', int n = 60);
void title(const std::string& t);

}  // namespace ui
