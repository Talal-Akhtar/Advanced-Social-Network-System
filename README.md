🚀 Advanced Social Network System

A terminal-based social networking backend system developed in C++, designed to simulate platforms like Facebook and LinkedIn using core data structures and algorithms.

This project focuses on efficiently managing users, relationships, interactions, and intelligent features such as recommendations, ranking, and community detection.

📖 Overview
With the rapid growth of social media platforms, efficient data handling is critical. This project demonstrates how data structures power real-world applications by building a complete backend simulation of a social network.

🧠 Key Features

👤 User & Network Management
Create and manage user profiles
Add/remove users
Manage friendships
Store:
UserID
Name
Age
Interests
Activity Score
Online/Offline Status

🌐 Weighted Friendship Graph
Social network represented as a weighted graph
Friendship strength based on:
Interaction frequency
Number of messages
Shared interests

🤝 Friend Recommendation System
Suggest friends using:
Mutual connections
Common interests
Interaction levels
Ranked recommendations using efficient structures

🔗 Shortest Connection Path
Finds the strongest/shortest path between users
Demonstrates graph traversal algorithms (like Dijkstra/BFS)

💬 Messaging System
Send and receive messages
Maintains correct order using appropriate structures (e.g., queues)

📰 News Feed Generation
Displays posts from friends
Prioritization based on:
Recency
Popularity

🔄 Activity Tracking with Undo
Tracks:
Likes
Comments
Posts
Undo functionality using stack-based logic

👥 Community Detection
Identifies clusters/groups of users
Based on connection patterns

🎯 Interest-Based Matching
Matches users with similar interests
Efficient searching and filtering

🏆 User Ranking System
Ranks users based on:
Activity
Engagement level
🧱 Data Structures Used

This project demonstrates practical usage of:

Graph (Adjacency List) → Social network representation
Queue → Messaging system
Stack → Undo operations
Priority Queue / Heap → Ranking & recommendations
Hash Map / Unordered Map → Fast user lookup
Vectors / Lists → Storing user data & interests
Sets → Managing unique interests & connections
⚙️ Technologies
Language: C++
Interface: Terminal-based (CLI)
Architecture: Modular (Separate classes/modules)

📂 Project Structure
/src
 ├── User.h
 ├── Graph.h
 ├── Recommendation.h
 ├── PathFinder.h
 ├── Messaging.h
 ├── NewsFeed.h
 ├── ActivityTracker.h
 ├── Community.h
 ├── Ranking.h
 └── main.cpp
 
▶️ How to Run
Clone the repository:
git clone https://github.com/your-username/advanced-social-network.git
Compile the project: g++ main.cpp -o social_network
Run: ./social_network


Contributions are welcome! Feel free to fork and improve the system.

📜 License
This project is for educational purposes.
