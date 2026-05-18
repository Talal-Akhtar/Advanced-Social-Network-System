#ifndef COMMUNITY_H
#define COMMUNITY_H

#include <iostream>
#include <string>
#include "Graph.h"
using namespace std;


class UnionFind {
private:
    int* parent;
    int* rank;
    int  capacity;

    void ensureCapacity(int id) {
        if (id < capacity) return;
        int newCap = (id + 1) * 2;
        int* np = new int[newCap];
        int* nr = new int[newCap];
        for (int i = 0; i < newCap; i++) { np[i] = i; nr[i] = 0; }
        for (int i = 0; i < capacity; i++) { np[i] = parent[i]; nr[i] = rank[i]; }
        delete[] parent; delete[] rank;
        parent = np; rank = nr;
        capacity = newCap;
    }

public:
    UnionFind(int cap = 256) : capacity(cap) {
        parent = new int[cap];
        rank   = new int[cap];
        for (int i = 0; i < cap; i++) { parent[i] = i; rank[i] = 0; }
    }

    ~UnionFind() { delete[] parent; delete[] rank; }

    int find(int x) {
        ensureCapacity(x);
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }

    void unite(int x, int y) {
        ensureCapacity(x); ensureCapacity(y);
        int px = find(x), py = find(y);
        if (px == py) return;
        if (rank[px] < rank[py]) { int t = px; px = py; py = t; }
        parent[py] = px;
        if (rank[px] == rank[py]) rank[px]++;
    }

    bool connected(int x, int y) { return find(x) == find(y); }
};


struct IntNode {
    int      id;
    IntNode* next;
    IntNode(int i) : id(i), next(nullptr) {}
};

struct Community {
    int      root;        // DSU root representative
    IntNode* members;
    int      memberCount;
    Community* next;

    Community(int r) : root(r), members(nullptr), memberCount(0), next(nullptr) {}

    void addMember(int id) {
        IntNode* n = new IntNode(id);
        n->next  = members;
        members  = n;
        memberCount++;
    }

    ~Community() {
        while (members) { IntNode* t = members->next; delete members; members = t; }
    }
};


class CommunityDetector {
private:
    const Graph& graph;

public:
    CommunityDetector(const Graph& g) : graph(g) {}

    void detectCommunities() const {
        int maxID = 0;
        UserNode* un = graph.getUserList();
        while (un) { if (un->user.userID > maxID) maxID = un->user.userID; un = un->next; }
        maxID++;

        UnionFind uf(maxID);

        // Union all connected friends
        AdjRow* row = graph.getAdjList();
        while (row) {
            EdgeNode* e = row->edges;
            while (e) {
                uf.unite(row->userID, e->toID);
                e = e->next;
            }
            row = row->next;
        }

        Community* commHead = nullptr;

        un = graph.getUserList();
        while (un) {
            int uid  = un->user.userID;
            int root = uf.find(uid);

            // Find existing community with this root
            Community* c = commHead;
            while (c && c->root != root) c = c->next;

            if (!c) {
                c = new Community(root);
                c->next  = commHead;
                commHead = c;
            }
            c->addMember(uid);
            un = un->next;
        }

        cout << endl << "===== Community Detection Results =====" << endl;
        int idx = 1;
        Community* cur = commHead;
        while (cur) {
            cout << "  Community " << idx++ << " (" << cur->memberCount << " members): ";
            IntNode* m = cur->members;
            while (m) {
                User* u = graph.getUser(m->id);
                cout << (u ? u->name : "?") << "[" << m->id << "]";
                if (m->next) cout << ", ";
                m = m->next;
            }
            cout << endl;
            cur = cur->next;
        }
        cout << "=======================================" << endl;

        while (commHead) { Community* tmp = commHead->next; delete commHead; commHead = tmp; }
    }
};

struct InterestBucket {
    string   interest;
    IntNode* users;   // list of userIDs with this interest
    int      count;
    InterestBucket* next;   // for hash chaining

    InterestBucket(const string& s)
        : interest(s), users(nullptr), count(0), next(nullptr) {}

    ~InterestBucket() {
        while (users) { IntNode* t = users->next; delete users; users = t; }
    }

    void addUser(int uid) {
        IntNode* n = new IntNode(uid);
        n->next = users; users = n; count++;
    }
};

class InterestMatcher {
private:
    static const int TABLE_SIZE = 101;
    InterestBucket* table[TABLE_SIZE];
    const Graph& graph;

    int hashStr(const string& s) const {
        unsigned long h = 5381;
        for (char c : s) h = ((h << 5) + h) + c;
        return (int)(h % TABLE_SIZE);
    }

    InterestBucket* findBucket(const string& interest) const {
        int idx = hashStr(interest);
        InterestBucket* b = table[idx];
        while (b) { if (b->interest == interest) return b; b = b->next; }
        return nullptr;
    }

public:
    InterestMatcher(const Graph& g) : graph(g) {
        for (int i = 0; i < TABLE_SIZE; i++) table[i] = nullptr;
        buildIndex();
    }

    ~InterestMatcher() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            while (table[i]) {
                InterestBucket* tmp = table[i]->next;
                delete table[i];
                table[i] = tmp;
            }
        }
    }

    void buildIndex() {
        // Clear old data
        for (int i = 0; i < TABLE_SIZE; i++) {
            while (table[i]) {
                InterestBucket* tmp = table[i]->next;
                delete table[i];
                table[i] = tmp;
            }
        }

        UserNode* un = graph.getUserList();
        while (un) {
            StringNode* interest = un->user.interests.head;
            while (interest) {
                int idx = hashStr(interest->data);
                InterestBucket* b = table[idx];
                while (b && b->interest != interest->data) b = b->next;
                if (!b) {
                    b = new InterestBucket(interest->data);
                    b->next    = table[idx];
                    table[idx] = b;
                }
                b->addUser(un->user.userID);
                interest = interest->next;
            }
            un = un->next;
        }
    }

    void matchUsers(int userID) const {
        User* u = graph.getUser(userID);
        if (!u) { cout << "[!] User not found." << endl; return; }

        cout << endl << "===== Interest-Based Matches for " << u->name << " =====" << endl;

        StringNode* interest = u->interests.head;
        if (!interest) { cout << "  (user has no interests set)" << endl; return; }

        bool anyMatch = false;
        while (interest) {
            InterestBucket* b = findBucket(interest->data);
            if (b) {
                cout << "  [" << interest->data << "]: ";
                IntNode* m = b->users;
                bool first = true;
                while (m) {
                    if (m->id != userID) {
                        User* mu = graph.getUser(m->id);
                        if (!first) cout << ", ";
                        cout << (mu ? mu->name : "?") << "[" << m->id << "]";
                        first = false;
                        anyMatch = true;
                    }
                    m = m->next;
                }
                if (!first) cout << endl;
                else cout << "(only you)" << endl;
            }
            interest = interest->next;
        }

        if (!anyMatch) cout << "  No matches found." << endl;
        cout << "================================================" << endl;
    }

    void displayAllInterestGroups() const {
        cout << endl << "===== All Interest Groups =====" << endl;
        for (int i = 0; i < TABLE_SIZE; i++) {
            InterestBucket* b = table[i];
            while (b) {
                cout << "  [" << b->interest << "] (" << b->count << " users): ";
                IntNode* m = b->users;
                while (m) {
                    User* u = graph.getUser(m->id);
                    cout << (u ? u->name : "?");
                    if (m->next) cout << ", ";
                    m = m->next;
                }
                cout << endl;
                b = b->next;
            }
        }
        cout << "===============================" << endl;
    }
};

#endif
