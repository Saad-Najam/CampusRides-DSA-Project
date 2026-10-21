# Campus Rides - NUCES Karachi carpooling (DSA project, BCS-3G)

Console carpooling app in C++17. Every core data structure is written from scratch; the STL is used
only for I/O, strings and file handling (as the proposal states).

## Build and run

| Platform | Commands |
|---|---|
| Linux / Git Bash / MinGW | `make` then `./campusrides` (or `make seed` for demo data, `make test` for the tests) |
| Windows without make | `build.bat` (builds `campusrides.exe` and `test_runner.exe`) |

```
campusrides [--data <dir>] [--seed]
```

`--seed` fills an empty database with demo users and rides. Demo logins (password `pass1234`):
`k250807@nu.edu.pk` (driver of rides #1 and #3), `k251101@nu.edu.pk` (passenger).
Administrator: `admin@nu.edu.pk` / `admin123` (created automatically; change it in a real deployment).

Data is saved as CSV files in `data/` after every action: `users`, `rides`, `bookings`, `requests`,
`messages`, `notifications`, `history`, `ratings`.

## Who wrote what

Each member owns one folder under `src/` and one test file under `tests/`. Commit under your own name.

| Member | Folder | Data structures | Features |
|---|---|---|---|
| **Syed Saad Najam** (25K-0807) | `src/member1_saad/` | Hash Table (chaining, auto-resize), Stack | Module 1: registration with `@nu.edu.pk` check, login, profile, user persistence, app shell (menus, `main`, navigation breadcrumbs), demo seed |
| **Ayaan Kashif** (25K-0572) | `src/member2_ayaan/` | Graph (adjacency list, Dijkstra, BFS), Min-Heap, Doubly Linked List, BST (AVL-balanced) | Module 2: ride posting with shortest-route suggestion, route-aware matching, feed browsing (next/previous), earliest-first listing, date range search, ride persistence |
| **Shamaem Saqib** (25K-0692) | `src/member3_shamaem/` | Singly Linked List, Queue (FIFO) | Module 3: seat requests and confirmations, seat management, notifications, ratings, ride history, chat, admin panel with undo, social persistence |
| shared | `src/common/` | Dynamic array | string/date/CSV helpers, console input |

Tests: `tests/test_saad.cpp`, `tests/test_ayaan.cpp`, `tests/test_shamaem.cpp`, run together by `tests/main_test.cpp`.

### Where each data structure from the proposal is used

| Structure | Where | File |
|---|---|---|
| Hash Table | users by email / student ID, rides by id, zone-name lookup | `member1_saad/HashTable.h`, `UserManager.cpp` |
| Graph | Karachi zone map, shortest route, route-overlap matching | `member2_ayaan/Graph.cpp`, `RideMatcher.cpp`, `CampusMap.cpp` |
| Min-Heap | ranking matches, earliest rides, Dijkstra frontier | `member2_ayaan/MinHeap.h` |
| Doubly Linked List | ride feed sorted by departure | `member2_ayaan/DoublyLinkedList.h`, `RideManager.cpp` |
| BST | ride index by (date/time, id), range queries | `member2_ayaan/AVLTree.h` |
| Queue | per-ride FIFO seat requests | `member3_shamaem/Queue.h`, `RequestService.cpp` |
| Linked List | notifications, history, chat, ratings | `member3_shamaem/LinkedList.h` |
| Stack | menu navigation trail, admin undo | `member1_saad/Stack.h`, `Context.h`, `AdminPanel.cpp` |

### How the pieces depend on each other

`User` (Saad) holds lists from Shamaem; `Ride` (Ayaan) holds Shamaem's `Queue`/`LinkedList`; `Graph` (Ayaan)
uses Saad's `HashTable` and Shamaem's `Queue`; `RequestService` and `AdminPanel` (Shamaem) operate on Saad's
users and Ayaan's rides. `src/member1_saad/Ui.h` lists every console flow and who implements it.

## Design notes (useful for the viva)

- **BST choice:** the index is an AVL tree (a self-balancing BST). Rides are often posted in chronological
  order, which would turn a plain BST into a linked list; AVL keeps insert / search at O(log n) and a date
  range query at O(log n + k).
- **Matching:** a ride matches if its route visits a stop near the passenger's pickup and later a stop near
  the drop-off. "Near" is graph shortest-path distance (0 km for an exact stop). Results are ordered by a
  Min-Heap: least extra distance first, then earliest departure.
- **Fairness:** requests wait in a FIFO queue per ride; the driver always handles the oldest one first.
- **Passwords** are salted and hashed with iterated FNV-1a. That is fine for a course project but is not a
  production-grade scheme (use bcrypt/argon2 in a real system).
- **Email verification** checks the `@nu.edu.pk` domain only; there is no confirmation email.
- Zone distances in `CampusMap.cpp` are approximate and easy to edit.
