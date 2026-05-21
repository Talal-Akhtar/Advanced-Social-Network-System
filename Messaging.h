#ifndef MESSAGING_H
#define MESSAGING_H

#include <iostream>
#include <string>
#include "Graph.h"
using namespace std;

struct Message {
    int    msgID;
    int    fromID;
    int    toID;
    string content;
    string timestamp;   

    Message() : msgID(0), fromID(0), toID(0) {}
    Message(int id, int from, int to, const string& c, const string& ts)
        : msgID(id), fromID(from), toID(to), content(c), timestamp(ts) {}
};

struct MsgNode {
    Message  msg;
    MsgNode* next;
    MsgNode(const Message& m) : msg(m), next(nullptr) {}
};

class MessageQueue {
private:
    MsgNode* front;
    MsgNode* rear;
    int      sz;

public:
    MessageQueue() : front(nullptr), rear(nullptr), sz(0) {}

    ~MessageQueue() {
        while (front) {
            MsgNode* tmp = front->next;
            delete front;
            front = tmp;
        }
    }

    void enqueue(const Message& m) {
        MsgNode* n = new MsgNode(m);
        if (!rear) { front = rear = n; }
        else { rear->next = n; rear = n; }
        sz++;
    }

    Message dequeue() {
        if (!front) { cout << "[!] Queue is empty." << endl; return Message(); }
        Message m = front->msg;
        MsgNode* tmp = front->next;
        delete front;
        front = tmp;
        if (!front) rear = nullptr;
        sz--;
        return m;
    }

    bool empty() const { return sz == 0; }
    int  size()  const { return sz; }

    void displayAll() const {
        MsgNode* cur = front;
        if (!cur) { cout << "  (no messages)" << endl; return; }
        while (cur) {
            cout << "  [" << cur->msg.msgID << "] "
                 << "From:" << cur->msg.fromID
                 << "  | " << cur->msg.content
                 << "  (" << cur->msg.timestamp << ")" << endl;
            cur = cur->next;
        }
    }
};

struct InboxEntry {
    int           userID;
    MessageQueue  inbox;
    InboxEntry*   next;
    InboxEntry(int id) : userID(id), next(nullptr) {}
};

class MessagingSystem {
private:
    InboxEntry* inboxList;
    int         msgCounter;
    Graph&      graph;

    InboxEntry* getInbox(int userID) {
        InboxEntry* cur = inboxList;
        while (cur) { if (cur->userID == userID) return cur; cur = cur->next; }
        // create
        InboxEntry* e = new InboxEntry(userID);
        e->next  = inboxList;
        inboxList = e;
        return e;
    }
    string makeTimestamp() {
        string ts = "MSG#";
        int n = ++msgCounter;
        char buf[20]; int i = 0;
        if (n == 0) { buf[i++] = '0'; }
        else { int tmp = n; while (tmp > 0) { buf[i++] = '0' + tmp % 10; tmp /= 10; } }
        // reverse
        for (int l = 0, r = i-1; l < r; l++, r--) { char c = buf[l]; buf[l] = buf[r]; buf[r] = c; }
        buf[i] = '\0';
        return ts + buf;
    }

public:
    MessagingSystem(Graph& g) : inboxList(nullptr), msgCounter(0), graph(g) {}

    ~MessagingSystem() {
        while (inboxList) {
            InboxEntry* tmp = inboxList->next;
            delete inboxList;
            inboxList = tmp;
        }
    }

    void sendMessage(int fromID, int toID, const string& content) {
        if (!graph.getUser(fromID) || !graph.getUser(toID)) {
            cout << "[!] One or both users not found." << endl;
            return;
        }

        string ts  = makeTimestamp();
        Message m(msgCounter, fromID, toID, content, ts);
        getInbox(toID)->inbox.enqueue(m);

        if (graph.areFriends(fromID, toID)) {
            AdjRow* row = graph.getAdjList();
            while (row && row->userID != fromID) row = row->next;
            if (row) {
                EdgeNode* e = row->edges;
                while (e) {
                    if (e->toID == toID) {
                        e->msgCount++;
                        // recompute weight
                        double w = graph.computeWeight(fromID, toID, e->interactionFreq, e->msgCount);
                        e->weight = w;
                        AdjRow* r2 = graph.getAdjList();
                        while (r2 && r2->userID != toID) r2 = r2->next;
                        if (r2) {
                            EdgeNode* e2 = r2->edges;
                            while (e2) {
                                if (e2->toID == fromID) { e2->msgCount++; e2->weight = w; break; }
                                e2 = e2->next;
                            }
                        }
                        break;
                    }
                    e = e->next;
                }
            }
        }

        User* sender = graph.getUser(fromID);
        User* recvr  = graph.getUser(toID);
        cout << "[>] Message sent from " << (sender ? sender->name : "?")
             << " to " << (recvr ? recvr->name : "?")
             << " [" << ts << "]" << endl;
    }

    void readMessages(int userID) {
        User* u = graph.getUser(userID);
        if (!u) { cout << "[!] User not found." << endl; return; }

        InboxEntry* box = getInbox(userID);
        cout << endl << "===== Inbox of " << u->name << " (" << box->inbox.size() << " messages) =====" << endl;
        box->inbox.displayAll();
        cout << "================================================" << endl;
    }

    void readNextMessage(int userID) {
        User* u = graph.getUser(userID);
        if (!u) { cout << "[!] User not found." << endl; return; }

        InboxEntry* box = getInbox(userID);
        if (box->inbox.empty()) {
            cout << "[i] No new messages for " << u->name << "." << endl;
            return;
        }
        Message m = box->inbox.dequeue();
        User* sender = graph.getUser(m.fromID);
        cout << endl << "--- New Message ---" << endl;
        cout << "  From    : " << (sender ? sender->name : "?") << " [" << m.fromID << "]" << endl;
        cout << "  Content : " << m.content << endl;
        cout << "  Time    : " << m.timestamp << endl;
        cout << "------------------" << endl;
    }
};

#endif
