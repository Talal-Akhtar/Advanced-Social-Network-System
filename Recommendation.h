#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <iostream>
#include <string>
#include "Graph.h"
using namespace std;


struct RecEntry {
    int    userID;
    string name;
    double score;
    int    mutualCount;
    int    commonInterests;
};

class RecommendationHeap {
private:
    RecEntry* heap;
    int       capacity;
    int       sz;

    void resize() {
        capacity *= 2;
        RecEntry* newHeap = new RecEntry[capacity];
        for (int i = 0; i < sz; i++) newHeap[i] = heap[i];
        delete[] heap;
        heap = newHeap;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (heap[parent].score < heap[i].score) {
                RecEntry tmp = heap[parent];
                heap[parent] = heap[i];
                heap[i]      = tmp;
                i = parent;
            } else break;
        }
    }

    void siftDown(int i) {
        while (true) {
            int left  = 2 * i + 1;
            int right = 2 * i + 2;
            int largest = i;
            if (left  < sz && heap[left].score  > heap[largest].score) largest = left;
            if (right < sz && heap[right].score > heap[largest].score) largest = right;
            if (largest == i) break;
            RecEntry tmp   = heap[i];
            heap[i]        = heap[largest];
            heap[largest]  = tmp;
            i = largest;
        }
    }

public:
    RecommendationHeap(int cap = 64)
        : capacity(cap), sz(0) { heap = new RecEntry[capacity]; }

    ~RecommendationHeap() { delete[] heap; }

    void push(const RecEntry& e) {
        if (sz == capacity) resize();
        heap[sz++] = e;
        siftUp(sz - 1);
    }

    RecEntry pop() {
        RecEntry top = heap[0];
        heap[0] = heap[--sz];
        siftDown(0);
        return top;
    }

    bool empty() const { return sz == 0; }
    int  size()  const { return sz; }
};

class FriendRecommender {
private:
    const Graph& graph;

    // Count mutual friends between u and candidate
    int mutualFriends(int uID, int candID) const {
        int count = 0;
        AdjRow* ru = graph.getAdjList();
        // find u's row
        while (ru && ru->userID != uID) ru = ru->next;
        if (!ru) return 0;

        EdgeNode* eu = ru->edges;
        while (eu) {
            // check if eu->toID is also a friend of cand
            AdjRow* rc = graph.getAdjList();
            while (rc && rc->userID != candID) rc = rc->next;
            if (rc) {
                EdgeNode* ec = rc->edges;
                while (ec) {
                    if (ec->toID == eu->toID) { count++; break; }
                    ec = ec->next;
                }
            }
            eu = eu->next;
        }
        return count;
    }

public:
    FriendRecommender(const Graph& g) : graph(g) {}

    void recommend(int userID, int topN) const {
        User* u = graph.getUser(userID);
        if (!u) { cout << "[!] User not found." << endl; return; }

        RecommendationHeap heap;

        UserNode* cur = graph.getUserList();
        while (cur) {
            int candID = cur->user.userID;
            if (candID == userID || graph.areFriends(userID, candID)) {
                cur = cur->next;
                continue;
            }

            int mutual  = mutualFriends(userID, candID);
            int common  = u->commonInterests(cur->user);

            // Score: mutual friends carry most weight, then interests
            double score = mutual * 3.0 + common * 2.0;

            if (score > 0) {
                RecEntry entry;
                entry.userID          = candID;
                entry.name            = cur->user.name;
                entry.score           = score;
                entry.mutualCount     = mutual;
                entry.commonInterests = common;
                heap.push(entry);
            }
            cur = cur->next;
        }

        cout << endl << "===== Friend Recommendations for " << u->name << " =====" << endl;
        if (heap.empty()) {
            cout << "  No recommendations available right now." << endl;
            return;
        }

        int shown = 0;
        while (!heap.empty() && shown < topN) {
            RecEntry e = heap.pop();
            cout << "  " << (shown + 1) << ". " << e.name
                 << " [ID:" << e.userID << "]"
                 << "  Score:"           << e.score
                 << "  MutualFriends:"   << e.mutualCount
                 << "  CommonInterests:" << e.commonInterests << endl;
            shown++;
        }
        cout << "============================================" << endl;
    }
};

#endif
