#ifndef NEWSFEED_H
#define NEWSFEED_H

#include <iostream>
#include <string>
#include "Graph.h"
using namespace std;

struct Post {
    int    postID;
    int    authorID;
    string content;
    int    likes;
    int    comments;
    int    timestamp;   

    Post() : postID(0), authorID(0), likes(0), comments(0), timestamp(0) {}
    Post(int pid, int aid, const string& c, int ts)
        : postID(pid), authorID(aid), content(c),
          likes(0), comments(0), timestamp(ts) {}

    double feedScore() const {
        return timestamp * 0.5 + (likes + comments) * 0.5;
    }
};


struct PostNode {
    Post      post;
    PostNode* next;
    PostNode(const Post& p) : post(p), next(nullptr) {}
};

class FeedHeap {
private:
    Post* heap;
    int   capacity;
    int   sz;

    void resize() {
        capacity *= 2;
        Post* nh = new Post[capacity];
        for (int i = 0; i < sz; i++) nh[i] = heap[i];
        delete[] heap;
        heap = nh;
    }

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (heap[p].feedScore() < heap[i].feedScore()) {
                Post t = heap[p]; heap[p] = heap[i]; heap[i] = t;
                i = p;
            } else break;
        }
    }

    void siftDown(int i) {
        while (true) {
            int l = 2*i+1, r = 2*i+2, best = i;
            if (l < sz && heap[l].feedScore() > heap[best].feedScore()) best = l;
            if (r < sz && heap[r].feedScore() > heap[best].feedScore()) best = r;
            if (best == i) break;
            Post t = heap[i]; heap[i] = heap[best]; heap[best] = t;
            i = best;
        }
    }

public:
    FeedHeap(int cap = 128) : capacity(cap), sz(0) {
        heap = new Post[capacity];
    }
    ~FeedHeap() { delete[] heap; }

    void push(const Post& p) {
        if (sz == capacity) resize();
        heap[sz++] = p;
        siftUp(sz - 1);
    }

    Post pop() {
        Post top = heap[0];
        heap[0] = heap[--sz];
        siftDown(0);
        return top;
    }

    bool empty() const { return sz == 0; }
};

class NewsFeedSystem {
private:
    PostNode* allPosts;
    int       postCounter;
    int       tick;        
    Graph&    graph;

public:
    NewsFeedSystem(Graph& g)
        : allPosts(nullptr), postCounter(0), tick(0), graph(g) {}

    ~NewsFeedSystem() {
        while (allPosts) {
            PostNode* tmp = allPosts->next;
            delete allPosts;
            allPosts = tmp;
        }
    }

    void createPost(int authorID, const string& content) {
        User* u = graph.getUser(authorID);
        if (!u) { cout << "[!] Author not found." << endl; return; }

        Post p(++postCounter, authorID, content, ++tick);
        PostNode* node = new PostNode(p);
        node->next = allPosts;
        allPosts   = node;

        u->activityScore += 5.0;

        cout << "[+] " << u->name << " posted: \"" << content << "\"  [PostID:" << postCounter << "]" << endl;
    }

    void likePost(int postID, int likerID) {
        PostNode* cur = allPosts;
        while (cur) {
            if (cur->post.postID == postID) {
                cur->post.likes++;
                User* liker = graph.getUser(likerID);
                User* author = graph.getUser(cur->post.authorID);
                if (author) author->activityScore += 1.0;
                cout << "[+] " << (liker ? liker->name : "?") << " liked post #" << postID << endl;
                return;
            }
            cur = cur->next;
        }
        cout << "[!] Post not found." << endl;
    }

    void commentPost(int postID, int commenterID, const string& comment) {
        PostNode* cur = allPosts;
        while (cur) {
            if (cur->post.postID == postID) {
                cur->post.comments++;
                tick++;
                cur->post.timestamp = tick;
                User* commenter = graph.getUser(commenterID);
                User* author    = graph.getUser(cur->post.authorID);
                if (author) author->activityScore += 2.0;
                cout << "[~] " << (commenter ? commenter->name : "?")
                     << " commented on post #" << postID << ": \"" << comment << "\"" << endl;
                return;
            }
            cur = cur->next;
        }
        cout << "[!] Post not found." << endl;
    }

    void generateFeed(int userID, int topK) const {
        User* u = graph.getUser(userID);
        if (!u) { cout << "[!] User not found." << endl; return; }

        FeedHeap fh;

        PostNode* cur = allPosts;
        while (cur) {
            int aid = cur->post.authorID;
            if (aid == userID || graph.areFriends(userID, aid)) {
                fh.push(cur->post);
            }
            cur = cur->next;
        }

        cout << endl << "===== News Feed for " << u->name << " =====" << endl;
        if (fh.empty()) {
            cout << "  (nothing to show yet)" << endl;
            cout << "================================" << endl;
            return;
        }

        int shown = 0;
        while (!fh.empty() && shown < topK) {
            Post p = fh.pop();
            User* author = graph.getUser(p.authorID);
            cout << "  [Post #" << p.postID << "]  "
                 << (author ? author->name : "?")
                 << "  +" << p.likes
                 << "  ~" << p.comments
                 << "  Score:" << p.feedScore() << endl;
            cout << "    \"" << p.content << "\"" << endl;
            shown++;
        }
        cout << "================================" << endl;
    }

    void displayAllPosts() const {
        cout << endl << "===== All Posts =====" << endl;
        PostNode* cur = allPosts;
        if (!cur) { cout << "  (none)" << endl; return; }
        while (cur) {
            User* a = graph.getUser(cur->post.authorID);
            cout << "  [#" << cur->post.postID << "] "
                 << (a ? a->name : "?") << ": \""
                 << cur->post.content << "\""
                 << "  +" << cur->post.likes
                 << "  ~" << cur->post.comments << endl;
            cur = cur->next;
        }
        cout << "====================" << endl;
    }
};

#endif
