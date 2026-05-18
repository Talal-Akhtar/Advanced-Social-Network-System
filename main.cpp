#include <windows.h>

#include <iostream>
#include <string>
#include <limits>
#include "User.h"
#include "Graph.h"
#include "Recommendation.h"
#include "PathFinder.h"
#include "Messaging.h"
#include "NewsFeed.h"
#include "ActivityTracker.h"
#include "Community.h"
#include "Ranking.h"
using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) { clearInput(); return val; }
        cout << "[!] Invalid input. Please enter an integer." << endl;
        clearInput();
    }
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

void menuUserManagement(Graph& graph, ActivityTracker& tracker) {
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   1. User & Network Management   |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Add User                     |" << endl;
        cout << "|  2. Remove User                  |" << endl;
        cout << "|  3. View User Profile            |" << endl;
        cout << "|  4. Display All Users            |" << endl;
        cout << "|  5. Add Interest to User         |" << endl;
        cout << "|  6. Toggle Online Status         |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) {
            int id   = readInt("  Enter UserID: ");
            string name = readLine("  Enter Name: ");
            int age  = readInt("  Enter Age: ");
            User u(id, name, age);
            graph.addUser(u);
            tracker.recordAction(id, ACTION_POST, "User profile created");
        }
        else if (choice == 2) {
            int id = readInt("  Enter UserID to remove: ");
            graph.removeUser(id);
        }
        else if (choice == 3) {
            int id = readInt("  Enter UserID: ");
            User* u = graph.getUser(id);
            if (u) u->display();
            else cout << "[!] User not found." << endl;
        }
        else if (choice == 4) {
            graph.displayAllUsers();
        }
        else if (choice == 5) {
            int id = readInt("  Enter UserID: ");
            User* u = graph.getUser(id);
            if (u) {
                string interest = readLine("  Enter Interest: ");
                u->addInterest(interest);
                cout << "[+] Interest added." << endl;
            } else cout << "[!] User not found." << endl;
        }
        else if (choice == 6) {
            int id = readInt("  Enter UserID: ");
            User* u = graph.getUser(id);
            if (u) {
                u->isOnline = !u->isOnline;
                cout << "[~] " << u->name << " is now "
                     << (u->isOnline ? "Online" : "Offline") << endl;
            } else cout << "[!] User not found." << endl;
        }
    } while (choice != 0);
}
void menuFriendship(Graph& graph, ActivityTracker& tracker) {
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   2. Friendship Management       |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Add Friendship               |" << endl;
        cout << "|  2. Remove Friendship            |" << endl;
        cout << "|  3. View Friends of User         |" << endl;
        cout << "|  4. Check Friendship             |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) {
            int id1  = readInt("  UserID 1: ");
            int id2  = readInt("  UserID 2: ");
            int freq = readInt("  Interaction Frequency: ");
            int msgs = readInt("  Messages Exchanged: ");
            if (graph.addFriendship(id1, id2, freq, msgs)) {
                tracker.recordAction(id1, ACTION_FRIEND_ADD,
                    "Became friends with user " + to_string(id2), id2);
                tracker.recordAction(id2, ACTION_FRIEND_ADD,
                    "Became friends with user " + to_string(id1), id1);
            }
        }
        else if (choice == 2) {
            int id1 = readInt("  UserID 1: ");
            int id2 = readInt("  UserID 2: ");
            if (graph.removeFriendship(id1, id2)) {
                tracker.recordAction(id1, ACTION_FRIEND_REMOVE,
                    "Removed friend user " + to_string(id2), id2);
            }
        }
        else if (choice == 3) {
            int id = readInt("  UserID: ");
            graph.displayFriends(id);
        }
        else if (choice == 4) {
            int id1 = readInt("  UserID 1: ");
            int id2 = readInt("  UserID 2: ");
            cout << "[i] " << id1 << " and " << id2
                 << (graph.areFriends(id1, id2) ? " ARE friends." : " are NOT friends.") << endl;
        }
    } while (choice != 0);
}
void menuRecommendations(const Graph& graph) {
    FriendRecommender recommender(graph);
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   3. Friend Recommendations      |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Get Recommendations          |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) {
            int id   = readInt("  UserID: ");
            int topN = readInt("  How many recommendations? ");
            recommender.recommend(id, topN);
        }
    } while (choice != 0);
}
void menuPaths(const Graph& graph) {
    PathFinder pf(graph);
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   4. Connection Paths            |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Strongest Connection Path    |" << endl;
        cout << "|  2. Shortest Hops Path           |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1 || choice == 2) {
            int src = readInt("  Source UserID: ");
            int dst = readInt("  Destination UserID: ");
            if (choice == 1) pf.findStrongestPath(src, dst);
            else             pf.findShortestHops(src, dst);
        }
    } while (choice != 0);
}
void menuMessaging(MessagingSystem& ms, Graph& /*graph*/, ActivityTracker& tracker) {
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   5. Messaging System            |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Send Message                 |" << endl;
        cout << "|  2. View Inbox (preview)         |" << endl;
        cout << "|  3. Read Next Message            |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) {
            int from = readInt("  From UserID: ");
            int to   = readInt("  To UserID: ");
            string content = readLine("  Message: ");
            ms.sendMessage(from, to, content);
            tracker.recordAction(from, ACTION_MESSAGE,
                "Sent message to user " + to_string(to), to);
        }
        else if (choice == 2) {
            int id = readInt("  UserID: ");
            ms.readMessages(id);
        }
        else if (choice == 3) {
            int id = readInt("  UserID: ");
            ms.readNextMessage(id);
        }
    } while (choice != 0);
}
void menuNewsFeed(NewsFeedSystem& nfs, Graph& /*graph*/, ActivityTracker& tracker) {
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   6. News Feed                   |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Create Post                  |" << endl;
        cout << "|  2. Like Post                    |" << endl;
        cout << "|  3. Comment on Post              |" << endl;
        cout << "|  4. Generate Feed                |" << endl;
        cout << "|  5. View All Posts               |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) {
            int id = readInt("  Author UserID: ");
            string content = readLine("  Post content: ");
            nfs.createPost(id, content);
            tracker.recordAction(id, ACTION_POST, "Created a post");
        }
        else if (choice == 2) {
            int postID  = readInt("  Post ID: ");
            int likerID = readInt("  Liker UserID: ");
            nfs.likePost(postID, likerID);
            tracker.recordAction(likerID, ACTION_LIKE,
                "Liked post #" + to_string(postID), postID);
        }
        else if (choice == 3) {
            int postID      = readInt("  Post ID: ");
            int commenterID = readInt("  Commenter UserID: ");
            string comment  = readLine("  Comment: ");
            nfs.commentPost(postID, commenterID, comment);
            tracker.recordAction(commenterID, ACTION_COMMENT,
                "Commented on post #" + to_string(postID), postID);
        }
        else if (choice == 4) {
            int id   = readInt("  UserID: ");
            int topK = readInt("  Show top-K posts: ");
            nfs.generateFeed(id, topK);
        }
        else if (choice == 5) {
            nfs.displayAllPosts();
        }
    } while (choice != 0);
}
void menuActivity(ActivityTracker& tracker) {
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   7. Activity Tracking           |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. View Activity Log (All)      |" << endl;
        cout << "|  2. View Activity Log (User)     |" << endl;
        cout << "|  3. Undo Last Action             |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) tracker.displayLog();
        else if (choice == 2) {
            int id = readInt("  UserID: ");
            tracker.displayLog(id);
        }
        else if (choice == 3) tracker.undoLastAction();
    } while (choice != 0);
}
void menuCommunity(const Graph& graph) {
    CommunityDetector cd(graph);
    InterestMatcher   im(graph);
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   8/9. Community & Interests     |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Detect Communities           |" << endl;
        cout << "|  2. Interest-Based Matching      |" << endl;
        cout << "|  3. Display All Interest Groups  |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) cd.detectCommunities();
        else if (choice == 2) {
            int id = readInt("  UserID: ");
            im.buildIndex();
            im.matchUsers(id);
        }
        else if (choice == 3) {
            im.buildIndex();
            im.displayAllInterestGroups();
        }
    } while (choice != 0);
}
void menuRanking(const Graph& graph) {
    UserRankingSystem urs(graph);
    int choice;
    do {
        cout << endl;
        cout << "+==================================+" << endl;
        cout << "|   10. User Ranking System        |" << endl;
        cout << "+==================================+" << endl;
        cout << "|  1. Display Global Rankings      |" << endl;
        cout << "|  2. Get Rank of Specific User    |" << endl;
        cout << "|  0. Back                         |" << endl;
        cout << "+==================================+" << endl;
        choice = readInt("  Choice: ");

        if (choice == 1) urs.displayRankings();
        else if (choice == 2) {
            int id = readInt("  UserID: ");
            urs.getUserRank(id);
        }
    } while (choice != 0);
}

void loadSampleData(Graph& graph, MessagingSystem& ms,
                    NewsFeedSystem& nfs, ActivityTracker& tracker)
{
    cout << endl << "[*] Loading sample data..." << endl;

    User u1(1, "Alice",   22); u1.addInterest("AI"); u1.addInterest("Music");
    User u2(2, "Bob",     25); u2.addInterest("AI"); u2.addInterest("Sports");
    User u3(3, "Charlie", 21); u3.addInterest("Music"); u3.addInterest("Travel");
    User u4(4, "Diana",   23); u4.addInterest("Sports"); u4.addInterest("AI");
    User u5(5, "Eve",     27); u5.addInterest("Travel"); u5.addInterest("Music");
    User u6(6, "Frank",   24); u6.addInterest("Gaming"); u6.addInterest("Sports");
    User u7(7, "Grace",   20); u7.addInterest("AI"); u7.addInterest("Gaming");

    graph.addUser(u1); graph.addUser(u2); graph.addUser(u3);
    graph.addUser(u4); graph.addUser(u5); graph.addUser(u6);
    graph.addUser(u7);

    // Friendships (id1, id2, freq, msgs)
    graph.addFriendship(1, 2, 10, 50);
    graph.addFriendship(1, 3, 5,  20);
    graph.addFriendship(2, 4, 8,  30);
    graph.addFriendship(3, 5, 6,  15);
    graph.addFriendship(4, 6, 3,  10);
    graph.addFriendship(5, 6, 4,  12);
    graph.addFriendship(2, 7, 7,  25);

    // Messages
    ms.sendMessage(1, 2, "Hey Bob!");
    ms.sendMessage(2, 1, "Hi Alice, how are you?");
    ms.sendMessage(3, 1, "Alice, are you coming tonight?");

    // Posts
    nfs.createPost(1, "Just learned about neural networks. Fascinating!");
    nfs.createPost(2, "Great game last night! Go team!");
    nfs.createPost(3, "Amazing trip to Paris this weekend!");
    nfs.createPost(4, "Anyone up for a run tomorrow?");
    nfs.createPost(7, "Just completed my AI project!");

    // Likes and comments
    nfs.likePost(1, 2);
    nfs.likePost(1, 7);
    nfs.commentPost(1, 3, "Sounds cool!");
    nfs.likePost(2, 4);
    nfs.commentPost(3, 5, "Paris is lovely!");

    // Activity
    tracker.recordAction(1, ACTION_POST,    "Created first post");
    tracker.recordAction(2, ACTION_LIKE,    "Liked Alice's post", 1);
    tracker.recordAction(1, ACTION_MESSAGE, "Sent message to Bob", 2);

    cout << "[OK] Sample data loaded successfully!" << endl;
}
//  Main
int main() {

    Graph           graph;
    MessagingSystem ms(graph);
    NewsFeedSystem  nfs(graph);
    ActivityTracker tracker(graph);

    cout << "+==================================================+" << endl;
    cout << "|      Advanced Social Network System v1.0         |" << endl;
    cout << "|      Data Structures & Algorithms Project        |" << endl;
    cout << "+==================================================+" << endl;

    int loadChoice = readInt("\n  Load sample data? (1=Yes / 0=No): ");
    if (loadChoice == 1) {
        loadSampleData(graph, ms, nfs, tracker);
    }

    int choice;
    do {
        cout << endl;
        cout << "+==================================================+" << endl;
        cout << "|                    MAIN MENU                    |" << endl;
        cout << "+==================================================+" << endl;
        cout << "|  1.  User & Network Management                   |" << endl;
        cout << "|  2.  Friendship Management (Weighted Graph)      |" << endl;
        cout << "|  3.  Friend Recommendation System                |" << endl;
        cout << "|  4.  Shortest / Strongest Connection Path        |" << endl;
        cout << "|  5.  Messaging System                            |" << endl;
        cout << "|  6.  News Feed Generation                        |" << endl;
        cout << "|  7.  Activity Tracking (Undo)                    |" << endl;
        cout << "|  8.  Community Detection & Interest Matching     |" << endl;
        cout << "|  9.  User Ranking System                         |" << endl;
        cout << "|  0.  Exit                                        |" << endl;
        cout << "+==================================================+" << endl;
        choice = readInt("  Choice: ");

        switch (choice) {
            case 1: menuUserManagement(graph, tracker); break;
            case 2: menuFriendship(graph, tracker);     break;
            case 3: menuRecommendations(graph);         break;
            case 4: menuPaths(graph);                   break;
            case 5: menuMessaging(ms, graph, tracker);  break;
            case 6: menuNewsFeed(nfs, graph, tracker);  break;
            case 7: menuActivity(tracker);              break;
            case 8: menuCommunity(graph);               break;
            case 9: menuRanking(graph);                 break;
            case 0: cout << endl << "Goodbye!" << endl; break;
            default: cout << "[!] Invalid option." << endl;
        }
    } while (choice != 0);

    return 0;
}
