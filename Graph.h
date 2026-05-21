#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <climits>
#include "User.h"
using namespace std;

struct EdgeNode {
    int      toID;          // friend's userID
    double   weight;        // friendship strength
    int      msgCount;      // messages exchanged
    int      interactionFreq; // interaction frequency
    EdgeNode* next;

    EdgeNode(int id, double w, int msg, int freq)
        : toID(id), weight(w), msgCount(msg),
          interactionFreq(freq), next(nullptr) {}
};

struct AdjRow {
    int      userID;
    EdgeNode* edges;   
    AdjRow*  next;     

    AdjRow(int id) : userID(id), edges(nullptr), next(nullptr) {}
};

struct UserNode {
    User     user;
    UserNode* next;
    UserNode(const User& u) : user(u), next(nullptr) {}
};

class Graph {
private:
    UserNode* userList;   
    AdjRow*   adjList;    
    int       userCount;

    AdjRow* getRow(int id) const {
        AdjRow* r = adjList;
        while (r) { if (r->userID == id) return r; r = r->next; }
        return nullptr;
    }

    UserNode* getUserNode(int id) const {
        UserNode* n = userList;
        while (n) { if (n->user.userID == id) return n; n = n->next; }
        return nullptr;
    }

public:
    Graph() : userList(nullptr), adjList(nullptr), userCount(0) {}

    ~Graph() {
        // free adj list
        while (adjList) {
            EdgeNode* e = adjList->edges;
            while (e) { EdgeNode* tmp = e->next; delete e; e = tmp; }
            AdjRow* tmp = adjList->next;
            delete adjList;
            adjList = tmp;
        }
        // free user list
        while (userList) {
            UserNode* tmp = userList->next;
            delete userList;
            userList = tmp;
        }
    }

    bool addUser(const User& u) {
        if (getUserNode(u.userID)) {
            cout << "[!] User with ID " << u.userID << " already exists." << endl;
            return false;
        }
        // add to user list
        UserNode* un = new UserNode(u);
        un->next = userList;
        userList = un;

        // add adjacency row
        AdjRow* row = new AdjRow(u.userID);
        row->next = adjList;
        adjList = row;

        userCount++;
        cout << "[+] User '" << u.name << "' added successfully." << endl;
        return true;
    }

    bool removeUser(int id) {
        UserNode* prev = nullptr;
        UserNode* cur  = userList;
        while (cur) {
            if (cur->user.userID == id) {
                if (prev) prev->next = cur->next;
                else       userList  = cur->next;
                delete cur;
                break;
            }
            prev = cur; cur = cur->next;
        }
        if (!cur) { cout << "[!] User not found." << endl; return false; }

        // remove the row for this user
        AdjRow* rPrev = nullptr;
        AdjRow* r     = adjList;
        while (r) {
            if (r->userID == id) {
                if (rPrev) rPrev->next = r->next;
                else        adjList    = r->next;
                EdgeNode* e = r->edges;
                while (e) { EdgeNode* t = e->next; delete e; e = t; }
                delete r;
                break;
            }
            rPrev = r; r = r->next;
        }

        // remove all edges pointing TO this user
        r = adjList;
        while (r) {
            EdgeNode* ePrev = nullptr;
            EdgeNode* e = r->edges;
            while (e) {
                if (e->toID == id) {
                    if (ePrev) ePrev->next = e->next;
                    else        r->edges   = e->next;
                    EdgeNode* t = e->next;
                    delete e;
                    e = t;
                } else { ePrev = e; e = e->next; }
            }
            r = r->next;
        }

        userCount--;
        cout << "[-] User " << id << " removed." << endl;
        return true;
    }

    User* getUser(int id) const {
        UserNode* n = getUserNode(id);
        return n ? &n->user : nullptr;
    }

    // -- Friendship Management -----------------
    // Weight = interaction_freq*0.4 + msg_count*0.4 + common_interests*0.2
    double computeWeight(int id1, int id2, int freq, int msgs) const {
        User* u1 = getUser(id1);
        User* u2 = getUser(id2);
        int common = (u1 && u2) ? u1->commonInterests(*u2) : 0;
        return freq * 0.4 + msgs * 0.4 + common * 0.2;
    }

    bool addFriendship(int id1, int id2, int freq, int msgs) {
        if (!getUser(id1) || !getUser(id2)) {
            cout << "[!] One or both users not found." << endl;
            return false;
        }
        if (id1 == id2) { cout << "[!] Cannot befriend yourself." << endl; return false; }

        double w = computeWeight(id1, id2, freq, msgs);

        AdjRow* r1 = getRow(id1);
        EdgeNode* e = r1->edges;
        while (e) {
            if (e->toID == id2) {
                e->weight = w; e->msgCount = msgs; e->interactionFreq = freq;
                // update reverse
                AdjRow* r2 = getRow(id2);
                EdgeNode* e2 = r2->edges;
                while (e2) {
                    if (e2->toID == id1) {
                        e2->weight = w; e2->msgCount = msgs; e2->interactionFreq = freq;
                        break;
                    }
                    e2 = e2->next;
                }
                cout << "[~] Friendship updated (weight=" << w << ")." << endl;
                return true;
            }
            e = e->next;
        }

        EdgeNode* e1 = new EdgeNode(id2, w, msgs, freq);
        e1->next = r1->edges; r1->edges = e1;

        // Add edge id2 -> id1  (undirected)
        AdjRow* r2 = getRow(id2);
        EdgeNode* e2 = new EdgeNode(id1, w, msgs, freq);
        e2->next = r2->edges; r2->edges = e2;

        cout << "[+] Friendship added between " << id1 << " <-> " << id2
             << " (weight=" << w << ")." << endl;
        return true;
    }

    bool removeFriendship(int id1, int id2) {
        bool removed = false;
        auto removeEdge = [](AdjRow* row, int targetID) -> bool {
            if (!row) return false;
            EdgeNode* prev = nullptr;
            EdgeNode* e    = row->edges;
            while (e) {
                if (e->toID == targetID) {
                    if (prev) prev->next = e->next;
                    else       row->edges = e->next;
                    delete e;
                    return true;
                }
                prev = e; e = e->next;
            }
            return false;
        };

        removed  = removeEdge(getRow(id1), id2);
        removed |= removeEdge(getRow(id2), id1);

        if (removed) cout << "[-] Friendship removed between " << id1 << " and " << id2 << "." << endl;
        else         cout << "[!] Friendship not found." << endl;
        return removed;
    }

    bool areFriends(int id1, int id2) const {
        AdjRow* r = getRow(id1);
        if (!r) return false;
        EdgeNode* e = r->edges;
        while (e) { if (e->toID == id2) return true; e = e->next; }
        return false;
    }

    double getWeight(int id1, int id2) const {
        AdjRow* r = getRow(id1);
        if (!r) return 0.0;
        EdgeNode* e = r->edges;
        while (e) { if (e->toID == id2) return e->weight; e = e->next; }
        return 0.0;
    }

    // -- Accessors -----------------------------
    UserNode* getUserList()  const { return userList; }
    AdjRow*   getAdjList()   const { return adjList;  }
    int       getUserCount() const { return userCount; }

    void displayAllUsers() const {
        cout << endl << "===== All Users =====" << endl;
        UserNode* cur = userList;
        if (!cur) { cout << "(no users)" << endl; return; }
        while (cur) { cur->user.display(); cur = cur->next; }
    }

    void displayFriends(int id) const {
        AdjRow* r = getRow(id);
        User*   u = getUser(id);
        if (!r || !u) { cout << "[!] User not found." << endl; return; }
        cout << endl << "Friends of " << u->name << ":" << endl;
        EdgeNode* e = r->edges;
        if (!e) { cout << "  (no friends)" << endl; return; }
        while (e) {
            User* f = getUser(e->toID);
            cout << "  -> " << (f ? f->name : "?")
                 << " [ID:" << e->toID << "]"
                 << "  Weight:" << e->weight
                 << "  Msgs:"   << e->msgCount
                 << "  Freq:"   << e->interactionFreq << endl;
            e = e->next;
        }
    }

    struct QNode { int id; QNode* next; QNode(int i) : id(i), next(nullptr) {} };

    int* bfsDistances(int srcID) const {
        int maxID = 0;
        UserNode* un = userList;
        while (un) { if (un->user.userID > maxID) maxID = un->user.userID; un = un->next; }
        maxID++;

        int* dist = new int[maxID];
        for (int i = 0; i < maxID; i++) dist[i] = -1;

        if (!getUser(srcID)) return dist;
        dist[srcID] = 0;

        // BFS queue
        QNode* qHead = new QNode(srcID);
        QNode* qTail = qHead;

        while (qHead) {
            int cur = qHead->id;
            QNode* tmp = qHead->next; delete qHead; qHead = tmp;
            if (!qHead) qTail = nullptr;

            AdjRow* row = getRow(cur);
            if (!row) continue;
            EdgeNode* e = row->edges;
            while (e) {
                if (e->toID < maxID && dist[e->toID] == -1) {
                    dist[e->toID] = dist[cur] + 1;
                    QNode* nq = new QNode(e->toID);
                    if (qTail) qTail->next = nq; else qHead = nq;
                    qTail = nq;
                }
                e = e->next;
            }
        }
        return dist;  
    }
};

#endif
