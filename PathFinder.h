#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <iostream>
#include <climits>
#include <cfloat>
#include "Graph.h"
using namespace std;

struct DijkEntry {
    int    nodeID;
    double dist;
};

class MinHeapDijk {
private:
    DijkEntry* heap;
    int        capacity;
    int        sz;

    void resize() {
        capacity *= 2;
        DijkEntry* nh = new DijkEntry[capacity];
        for (int i = 0; i < sz; i++) nh[i] = heap[i];
        delete[] heap;
        heap = nh;
    }

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (heap[p].dist > heap[i].dist) {
                DijkEntry t = heap[p]; heap[p] = heap[i]; heap[i] = t;
                i = p;
            } else break;
        }
    }

    void siftDown(int i) {
        while (true) {
            int l = 2*i+1, r = 2*i+2, s = i;
            if (l < sz && heap[l].dist < heap[s].dist) s = l;
            if (r < sz && heap[r].dist < heap[s].dist) s = r;
            if (s == i) break;
            DijkEntry t = heap[i]; heap[i] = heap[s]; heap[s] = t;
            i = s;
        }
    }

public:
    MinHeapDijk(int cap = 128) : capacity(cap), sz(0) {
        heap = new DijkEntry[capacity];
    }
    ~MinHeapDijk() { delete[] heap; }

    void push(int id, double d) {
        if (sz == capacity) resize();
        heap[sz] = {id, d};
        siftUp(sz++);
    }

    DijkEntry pop() {
        DijkEntry top = heap[0];
        heap[0] = heap[--sz];
        siftDown(0);
        return top;
    }

    bool empty() const { return sz == 0; }
};
class PathFinder {
private:
    const Graph& graph;

    // Find max userID to size arrays
    int maxUserID() const {
        int mx = 0;
        UserNode* u = graph.getUserList();
        while (u) { if (u->user.userID > mx) mx = u->user.userID; u = u->next; }
        return mx + 1;
    }

public:
    PathFinder(const Graph& g) : graph(g) {}

    void findStrongestPath(int srcID, int dstID) const {
        if (!graph.getUser(srcID) || !graph.getUser(dstID)) {
            cout << "[!] One or both users not found." << endl;
            return;
        }
        if (srcID == dstID) {
            cout << "[!] Source and destination are the same." << endl;
            return;
        }

        int N = maxUserID();

        double* dist  = new double[N];
        int*    prev  = new int[N];
        bool*   visit = new bool[N];

        for (int i = 0; i < N; i++) {
            dist[i]  = -DBL_MAX;   // -∞ (we maximise weight)
            prev[i]  = -1;
            visit[i] = false;
        }
        dist[srcID] = 0.0;

        MinHeapDijk pq;
        pq.push(srcID, 0.0);   // we push negative dist for max-weight

        while (!pq.empty()) {
            DijkEntry cur = pq.pop();
            int u = cur.nodeID;
            if (visit[u]) continue;
            visit[u] = true;

            AdjRow* row = graph.getAdjList();
            while (row && row->userID != u) row = row->next;
            if (!row) continue;

            EdgeNode* e = row->edges;
            while (e) {
                int    v = e->toID;
                double w = e->weight;
                if (v < N && dist[u] + w > dist[v]) {
                    dist[v] = dist[u] + w;
                    prev[v] = u;
                    pq.push(v, -dist[v]);   // negate for min-heap to act as max-heap
                }
                e = e->next;
            }
        }

        // Reconstruct path
        if (dist[dstID] == -DBL_MAX) {
            cout << "[!] No path found between users " << srcID << " and " << dstID << "." << endl;
        } else {
            // Build path in reverse using a stack (linked list)
            struct SNode { int id; SNode* next; };
            SNode* stack = nullptr;
            int cur = dstID;
            while (cur != -1) {
                SNode* s = new SNode(); s->id = cur; s->next = stack; stack = s;
                cur = prev[cur];
            }

            cout << endl << "===== Strongest Connection Path =====" << endl;
            cout << "  Total Strength: " << dist[dstID] << endl;
            cout << "  Path: ";
            bool first = true;
            while (stack) {
                User* u = graph.getUser(stack->id);
                if (!first) cout << " -> ";
                cout << (u ? u->name : "?") << "[" << stack->id << "]";
                first = false;
                SNode* tmp = stack->next; delete stack; stack = tmp;
            }
            cout << endl;
            cout << "=====================================" << endl;
        }

        delete[] dist;
        delete[] prev;
        delete[] visit;
    }

    void findShortestHops(int srcID, int dstID) const {
        if (!graph.getUser(srcID) || !graph.getUser(dstID)) {
            cout << "[!] One or both users not found." << endl;
            return;
        }

        int N = maxUserID();
        int* dist = graph.bfsDistances(srcID);
        int* prev = new int[N];
        for (int i = 0; i < N; i++) prev[i] = -1;

        // Redo BFS saving predecessors
        bool* visited = new bool[N];
        for (int i = 0; i < N; i++) visited[i] = false;

        struct QN { int id; QN* next; };
        QN* qHead = new QN(); qHead->id = srcID; qHead->next = nullptr;
        QN* qTail = qHead;
        visited[srcID] = true;

        while (qHead) {
            int cur = qHead->id;
            QN* tmp = qHead->next; delete qHead; qHead = tmp;
            if (!qHead) qTail = nullptr;

            AdjRow* row = graph.getAdjList();
            while (row && row->userID != cur) row = row->next;
            if (!row) continue;
            EdgeNode* e = row->edges;
            while (e) {
                if (e->toID < N && !visited[e->toID]) {
                    visited[e->toID] = true;
                    prev[e->toID] = cur;
                    QN* nq = new QN(); nq->id = e->toID; nq->next = nullptr;
                    if (qTail) qTail->next = nq; else qHead = nq;
                    qTail = nq;
                }
                e = e->next;
            }
        }

        if (dist[dstID] < 0) {
            cout << "[!] No path found." << endl;
        } else {
            struct SNode { int id; SNode* next; };
            SNode* stack = nullptr;
            int cur = dstID;
            while (cur != -1) {
                SNode* s = new SNode(); s->id = cur; s->next = stack; stack = s;
                if (cur < N) cur = prev[cur]; else break;
            }
            cout << endl << "===== Shortest Hops Path =====" << endl;
            cout << "  Hops: " << dist[dstID] << endl;
            cout << "  Path: ";
            bool first = true;
            while (stack) {
                User* u = graph.getUser(stack->id);
                if (!first) cout << " -> ";
                cout << (u ? u->name : "?") << "[" << stack->id << "]";
                first = false;
                SNode* t = stack->next; delete stack; stack = t;
            }
            cout << endl;
            cout << "==============================" << endl;
        }

        delete[] dist;
        delete[] prev;
        delete[] visited;
    }
};

#endif
