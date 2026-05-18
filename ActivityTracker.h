#ifndef ACTIVITYTRACKER_H
#define ACTIVITYTRACKER_H

#include <iostream>
#include <string>
#include "Graph.h"
using namespace std;

//  Action types
enum ActionType {
    ACTION_LIKE,
    ACTION_COMMENT,
    ACTION_POST,
    ACTION_FRIEND_ADD,
    ACTION_FRIEND_REMOVE,
    ACTION_MESSAGE
};

//  Action record
struct Action {
    int        actionID;
    int        userID;
    ActionType type;
    string     description;
    int        refID;     

    Action() : actionID(0), userID(0), type(ACTION_POST), refID(0) {}
    Action(int aid, int uid, ActionType t, const string& desc, int ref = 0)
        : actionID(aid), userID(uid), type(t), description(desc), refID(ref) {}

    string typeName() const {
        switch (type) {
            case ACTION_LIKE:          return "LIKE";
            case ACTION_COMMENT:       return "COMMENT";
            case ACTION_POST:          return "POST";
            case ACTION_FRIEND_ADD:    return "FRIEND_ADD";
            case ACTION_FRIEND_REMOVE: return "FRIEND_REMOVE";
            case ACTION_MESSAGE:       return "MESSAGE";
            default:                   return "UNKNOWN";
        }
    }
};

//  Stack node  (for undo)
struct StackNode {
    Action     action;
    StackNode* next;
    StackNode(const Action& a) : action(a), next(nullptr) {}
};

//  Undo Stack  (LIFO)

class UndoStack {
private:
    StackNode* top;
    int        sz;

public:
    UndoStack() : top(nullptr), sz(0) {}

    ~UndoStack() {
        while (top) {
            StackNode* tmp = top->next;
            delete top;
            top = tmp;
        }
    }

    void push(const Action& a) {
        StackNode* n = new StackNode(a);
        n->next = top;
        top = n;
        sz++;
    }

    Action pop() {
        if (!top) { cout << "[!] Undo stack is empty." << endl; return Action(); }
        Action a = top->action;
        StackNode* tmp = top->next;
        delete top;
        top = tmp;
        sz--;
        return a;
    }

    bool  empty() const { return sz == 0; }
    int   size()  const { return sz; }

    Action peek() const {
        if (!top) return Action();
        return top->action;
    }
};

struct LogNode {
    Action   action;
    LogNode* next;
    LogNode(const Action& a) : action(a), next(nullptr) {}
};

class ActivityTracker {
private:
    UndoStack  undoStack;     
    LogNode*   logHead;       
    int        actionCounter;
    Graph&     graph;

public:
    ActivityTracker(Graph& g)
        : logHead(nullptr), actionCounter(0), graph(g) {}

    ~ActivityTracker() {
        while (logHead) {
            LogNode* tmp = logHead->next;
            delete logHead;
            logHead = tmp;
        }
    }

    void recordAction(int userID, ActionType type,
                      const string& desc, int refID = 0)
    {
        Action a(++actionCounter, userID, type, desc, refID);
        undoStack.push(a);

        // Append to log
        LogNode* node = new LogNode(a);
        node->next = logHead;
        logHead    = node;

        User* u = graph.getUser(userID);
        if (u) {
            switch (type) {
                case ACTION_POST:          u->activityScore += 5.0; break;
                case ACTION_LIKE:          u->activityScore += 1.0; break;
                case ACTION_COMMENT:       u->activityScore += 2.0; break;
                case ACTION_MESSAGE:       u->activityScore += 1.5; break;
                case ACTION_FRIEND_ADD:    u->activityScore += 3.0; break;
                case ACTION_FRIEND_REMOVE: break;
                default: break;
            }
        }

        cout << "[*] Activity recorded: [" << a.typeName() << "] " << desc << endl;
    }

    void undoLastAction() {
        if (undoStack.empty()) {
            cout << "[!] Nothing to undo." << endl;
            return;
        }
        Action a = undoStack.pop();
        cout << "[<] Undo: [" << a.typeName() << "] " << a.description << endl;

        User* u = graph.getUser(a.userID);
        if (u) {
            switch (a.type) {
                case ACTION_POST:       u->activityScore -= 5.0; break;
                case ACTION_LIKE:       u->activityScore -= 1.0; break;
                case ACTION_COMMENT:    u->activityScore -= 2.0; break;
                case ACTION_MESSAGE:    u->activityScore -= 1.5; break;
                case ACTION_FRIEND_ADD: u->activityScore -= 3.0; break;
                default: break;
            }
            if (u->activityScore < 0) u->activityScore = 0;
        }
    }

    void displayLog(int userID = -1) const {
        cout << endl << "===== Activity Log";
        if (userID != -1) {
            User* u = graph.getUser(userID);
            cout << " for " << (u ? u->name : "?");
        }
        cout << " =====" << endl;

        LogNode* cur = logHead;
        bool found = false;
        while (cur) {
            if (userID == -1 || cur->action.userID == userID) {
                User* u = graph.getUser(cur->action.userID);
                cout << "  [#" << cur->action.actionID << "] "
                     << "[" << cur->action.typeName() << "] "
                     << (u ? u->name : "?") << ": "
                     << cur->action.description << endl;
                found = true;
            }
            cur = cur->next;
        }
        if (!found) cout << "  (no activities)" << endl;
        cout << "========================" << endl;
    }

    int undoStackSize() const { return undoStack.size(); }
};

#endif
