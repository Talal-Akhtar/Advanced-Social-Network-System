#ifndef USER_H
#define USER_H

#include <iostream>
#include <string>
using namespace std;

struct StringNode {
    string   data;
    StringNode* next;
    StringNode(const string& s) : data(s), next(nullptr) {}
};

struct StringList {
    StringNode* head;
    int         size;

    StringList() : head(nullptr), size(0) {}

    // deep-copy constructor
    StringList(const StringList& o) : head(nullptr), size(0) {
        StringNode* cur = o.head;
        while (cur) { append(cur->data); cur = cur->next; }
    }

    StringList& operator=(const StringList& o) {
        if (this == &o) return *this;
        clear();
        StringNode* cur = o.head;
        while (cur) { append(cur->data); cur = cur->next; }
        return *this;
    }

    ~StringList() { clear(); }

    void append(const string& s) {
        StringNode* n = new StringNode(s);
        if (!head) { head = n; }
        else {
            StringNode* cur = head;
            while (cur->next) cur = cur->next;
            cur->next = n;
        }
        size++;
    }

    bool contains(const string& s) const {
        StringNode* cur = head;
        while (cur) {
            if (cur->data == s) return true;
            cur = cur->next;
        }
        return false;
    }

    void clear() {
        while (head) {
            StringNode* tmp = head->next;
            delete head;
            head = tmp;
        }
        size = 0;
    }

    void print() const {
        StringNode* cur = head;
        while (cur) {
            cout << cur->data;
            if (cur->next) cout << ", ";
            cur = cur->next;
        }
    }
};

struct User {
    int         userID;
    string      name;
    int         age;
    StringList  interests;   
    double      activityScore;
    bool        isOnline;

    User()
        : userID(-1), name(""), age(0),
          activityScore(0.0), isOnline(false) {}

    User(int id, const string& n, int a)
        : userID(id), name(n), age(a),
          activityScore(0.0), isOnline(false) {}

    void addInterest(const string& interest) {
        if (!interests.contains(interest))
            interests.append(interest);
    }

    int commonInterests(const User& other) const {
        int count = 0;
        StringNode* cur = interests.head;
        while (cur) {
            if (other.interests.contains(cur->data)) count++;
            cur = cur->next;
        }
        return count;
    }

    void display() const {
        cout << "---------------------------------" << endl;
        cout << "  UserID : " << userID << endl;
        cout << "  Name   : " << name   << endl;
        cout << "  Age    : " << age    << endl;
        cout << "  Status : " << (isOnline ? "Online" : "Offline") << endl;
        cout << "  Score  : " << activityScore << endl;
        cout << "  Interests: ";
        interests.print();
        cout << endl;
        cout << "---------------------------------" << endl;
    }
};

#endif
