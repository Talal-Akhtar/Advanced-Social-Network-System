#ifndef RANKING_H
#define RANKING_H

#include <iostream>
#include "Graph.h"
using namespace std;

struct RankEntry {
    int    rank;
    int    userID;
    string name;
    double activityScore;
    int    friendCount;
};

struct RankNode {
    RankEntry  entry;
    RankNode*  next;
    RankNode(const RankEntry& e) : entry(e), next(nullptr) {}
};

class UserRankingSystem {
private:
    const Graph& graph;

    int countFriends(int userID) const {
        int count = 0;
        AdjRow* r = graph.getAdjList();
        while (r && r->userID != userID) r = r->next;
        if (!r) return 0;
        EdgeNode* e = r->edges;
        while (e) { count++; e = e->next; }
        return count;
    }

    double totalScore(const RankEntry& e) const {
        return e.activityScore + e.friendCount * 2.0;
    }

    // Insertion sort on linked list  (descending by totalScore)
    RankNode* insertionSort(RankNode* head) const {
        if (!head || !head->next) return head;
        RankNode* sorted = nullptr;

        while (head) {
            RankNode* cur = head;
            head = head->next;
            cur->next = nullptr;

            if (!sorted || totalScore(cur->entry) >= totalScore(sorted->entry)) {
                cur->next = sorted;
                sorted    = cur;
            } else {
                RankNode* tmp = sorted;
                while (tmp->next && totalScore(tmp->next->entry) > totalScore(cur->entry))
                    tmp = tmp->next;
                cur->next  = tmp->next;
                tmp->next  = cur;
            }
        }
        return sorted;
    }

public:
    UserRankingSystem(const Graph& g) : graph(g) {}

    void displayRankings() const {
        RankNode* list = nullptr;

        UserNode* un = graph.getUserList();
        while (un) {
            RankEntry e;
            e.rank          = 0;
            e.userID        = un->user.userID;
            e.name          = un->user.name;
            e.activityScore = un->user.activityScore;
            e.friendCount   = countFriends(un->user.userID);

            RankNode* rn = new RankNode(e);
            rn->next = list;
            list     = rn;
            un = un->next;
        }

        list = insertionSort(list);

        cout << endl << "===== User Rankings =====" << endl;
        cout << "  Rank | Name              | ActivityScore | Friends | TotalScore" << endl;
        cout << "  -----|-------------------|---------------|---------|----------" << endl;

        int rank = 1;
        RankNode* cur = list;
        while (cur) {
            cur->entry.rank = rank++;
            double ts = cur->entry.activityScore + cur->entry.friendCount * 2.0;

            cout << "   " << cur->entry.rank;
            cout << "   | ";

            // Pad name to 17 chars
            string padded = cur->entry.name;
            while ((int)padded.size() < 17) padded += ' ';
            cout << padded << " | ";

            cout << cur->entry.activityScore;
            // pad score
            string scoreStr = "";
            double sc = cur->entry.activityScore;
            if (sc < 10)   scoreStr = "       ";
            else if (sc < 100) scoreStr = "      ";
            else if (sc < 1000) scoreStr = "     ";
            else scoreStr = "    ";
            cout << scoreStr << "| ";

            cout << "   " << cur->entry.friendCount << "   | ";
            cout << ts << endl;

            cur = cur->next;
        }
        cout << "=========================" << endl;

        // Cleanup
        while (list) { RankNode* t = list->next; delete list; list = t; }
    }

    void getUserRank(int userID) const {
        RankNode* list = nullptr;

        UserNode* un = graph.getUserList();
        while (un) {
            RankEntry e;
            e.rank          = 0;
            e.userID        = un->user.userID;
            e.name          = un->user.name;
            e.activityScore = un->user.activityScore;
            e.friendCount   = countFriends(un->user.userID);
            RankNode* rn = new RankNode(e);
            rn->next = list; list = rn;
            un = un->next;
        }

        list = insertionSort(list);

        int rank = 1;
        RankNode* cur = list;
        while (cur) {
            if (cur->entry.userID == userID) {
                double ts = cur->entry.activityScore + cur->entry.friendCount * 2.0;
                cout << endl << "[#] " << cur->entry.name
                     << " is ranked #" << rank
                     << "  (Activity:" << cur->entry.activityScore
                     << ", Friends:" << cur->entry.friendCount
                     << ", Total:" << ts << ")" << endl;
                break;
            }
            rank++;
            cur = cur->next;
        }

        while (list) { RankNode* t = list->next; delete list; list = t; }
    }
};

#endif
