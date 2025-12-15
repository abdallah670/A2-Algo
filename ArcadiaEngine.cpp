// ArcadiaEngine.cpp - STUDENT TEMPLATE
// TODO: Implement all the functions below according to the assignment requirements

#include "ArcadiaEngine.h"
#include <algorithm>
#include <queue>
#include <numeric>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <set>

using namespace std;

// =========================================================
// PART A: DATA STRUCTURES (Concrete Implementations)
// =========================================================

// --- 1. PlayerTable (Double Hashing) ---

class ConcretePlayerTable : public PlayerTable {
private:
    // TODO: Define your data structures here
    // Hint: You'll need a hash table with double hashing collision resolution
    struct HashEntry {
        int playerID;
        string name;
        bool occupied;

        HashEntry() : playerID(-1), name(""), occupied(false) {}
        HashEntry(int id, const string& n) : playerID(id), name(n), occupied(true) {}
    };
    static const int TABLE_SIZE = 101;  // Fixed size as per requirements
    // Remove the vector and replace with:
    HashEntry table[TABLE_SIZE];  // Fixed array instead of vector
    // Add these constants at the top of class:
    int currentSize = 0;
    // Primary hash function
    int hash1(int key) {
        return key % TABLE_SIZE;
    }
    // Secondary hash function for double hashing
    // Must return value in [1, TABLE_SIZE-1] and never 0
    int hash2(int key) {
        return 1 + (key % (TABLE_SIZE - 1));
    }

    // Combined double hash for probing
    int doubleHash(int key, int attempt) {
        return (hash1(key) + attempt * hash2(key)) % TABLE_SIZE;
    }
    int findIndex(int playerID) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            int index = doubleHash(playerID, i);
            //insert
            if (!table[index].occupied || table[index].playerID == playerID) {
                return index;
            }
        }
        return -1;
    }
    int findExistingIndex(int playerID) {
        for (int attempt = 0; attempt < TABLE_SIZE; attempt++) {
            int index = doubleHash(playerID, attempt);

            // Empty slot means not found
            if (!table[index].occupied) {
                return -1;
            }

            // Found and valid
            if (table[index].occupied &&
                table[index].playerID == playerID) {
                return index;
            }
        }
        return -1;
    }


public:
    ConcretePlayerTable() {
        currentSize = 0;
    }
    void insert(int playerID, string name) override {
        // Check if table is full
        if (currentSize >= TABLE_SIZE) {
            throw "Table is full";
        }

        int index = findIndex(playerID);
        if (index == -1) {
            throw "Table is full";
        }

        // If inserting into empty slot (not update), increase size
        if (!table[index].occupied) {
            currentSize++;
        }

        // Insert/update
        table[index] = HashEntry(playerID, name);
    }

    string search(int playerID) override {
        int index = findExistingIndex(playerID);
        if (index != -1) {
            return table[index].name;
        }
        return "";
    }
};

// --- 2. Leaderboard (Skip List) ---

class ConcreteLeaderboard : public Leaderboard {
private:
    struct Node {
        int playerID, score;
        vector<Node*> next;
        Node(int id, int s, int level) : playerID(id), score(s), next(level + 1, nullptr) {}
    };

    Node* head;
    const int MAX_LEVEL = 16;
    int currentLevel;  // Track current max level in use
    // Helper to compare two nodes based on requirements
    // Primary: higher score comes first, Secondary: lower ID comes first for same score
    bool shouldPlaceBefore(Node* a, Node* b) {
        if (!b) return true; // NULL comes at the end
        if (a->score > b->score) return true;
        if (a->score == b->score && a->playerID < b->playerID) return true;
        return false;
    }
    int randomLevel() {
        int level = 0;
        while (rand() % 2 == 0 && level < MAX_LEVEL - 1) {
            level++;
        }
        return level;
    }
    // Linear scan to find node by ID 
    Node* findNodeByID(int playerID) {
        Node* current = head->next[0];
        while (current) {
            if (current->playerID == playerID) {
                return current;
            }
            current = current->next[0];
        }
        return nullptr;
    }

public:
    ConcreteLeaderboard() {
        srand(time(0));
        currentLevel = 0;
        head = new Node(-1, INT_MIN, MAX_LEVEL);  // Dummy head with lowest possible score
    }

    ~ConcreteLeaderboard() {
        Node* curr = head->next[0];
        while (curr) {
            Node* next = curr->next[0];
            delete curr;
            curr = next;
        }
        delete head;
    }

    void addScore(int playerID, int score) override {
        // Remove existing player first
        removePlayer(playerID);

        // Generate random level
        int level = randomLevel();

        // Update current level if needed
        if (level > currentLevel) {
            for (int i = currentLevel + 1; i <= level; i++) {
                head->next[i] = nullptr;
            }
            currentLevel = level;
        }

        // Create new node
        Node* newNode = new Node(playerID, score, level);
        Node* curr = head;

        // Find insertion position and insert at each level
        for (int i = currentLevel; i >= 0; i--) {
            // CORRECTED: Proper tie-breaking
            // Move while next node has HIGHER score OR same score with LOWER ID
            while (curr->next[i] &&
                (curr->next[i]->score > score ||
                    (curr->next[i]->score == score && curr->next[i]->playerID < playerID))) {
                curr = curr->next[i];
            }

            // Insert at this level
            if (i <= level) {
                newNode->next[i] = curr->next[i];
                curr->next[i] = newNode;
            }
        }
    }

    void removePlayer(int playerID) override {
        // First find the node and its score (linear search at level 0)
        Node* target = findNodeByID(playerID);
        if (!target) return;  // Player not found

        int targetScore = target->score;

        // Now use skip list search with score AND tie-breaking
        vector<Node*> update(MAX_LEVEL + 1, nullptr);
        Node* current = head;

        // Find update nodes using the SAME comparison as insertion
        for (int i = MAX_LEVEL; i >= 0; i--) {
            // Use proper comparison: higher scores first, then lower IDs for same score
            while (current->next[i] &&
                (current->next[i]->score > targetScore ||
                    (current->next[i]->score == targetScore &&
                        current->next[i]->playerID < playerID))) {
                current = current->next[i];
            }
            update[i] = current;
        }

        // At this point, current->next[0] should be our target OR 
        // a node with same score but lower ID

        // Check if we found the exact node
        if (current->next[0] && current->next[0]->playerID == playerID) {
            Node* toRemove = current->next[0];

            // Remove from all levels
            for (int i = 0; i <= MAX_LEVEL; i++) {
                // The update[i] should already point to predecessor
                // But we need to verify and possibly adjust
                Node* pred = update[i];

                // If pred->next[i] is not our target, we need to find it
                while (pred->next[i] && pred->next[i] != toRemove) {
                    pred = pred->next[i];
                }

                // Now pred should point to node before target (or be target itself)
                if (pred->next[i] == toRemove) {
                    pred->next[i] = toRemove->next[i];
                }
            }

            delete toRemove;

            // Update currentLevel
            while (currentLevel > 0 && head->next[currentLevel] == nullptr) {
                currentLevel--;
            }
        }
    }

    vector<int> getTopN(int n) override {
        vector<int> result;
        Node* curr = head->next[0];
        while (curr && result.size() < n) {
            result.push_back(curr->playerID);
            curr = curr->next[0];
        }

        return result;
    }
};
// --- 3. AuctionTree (Red-Black Tree) ---

class ConcreteAuctionTree : public AuctionTree {
private:
    enum Color { RED, BLACK };

    struct Node {
        int itemID;
        int price;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(int id, int p) : itemID(id), price(p), color(RED),
            left(nullptr), right(nullptr), parent(nullptr) {
        }
    };

    Node* root;
    Node* nil;

    // ========== BASIC ROTATIONS ==========
    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;

        if (y->left != nil) {
            y->left->parent = x;
        }

        y->parent = x->parent;

        if (x->parent == nil) {
            root = y;
        }
        else if (x == x->parent->left) {
            x->parent->left = y;
        }
        else {
            x->parent->right = y;
        }

        y->left = x;
        x->parent = y;
    }

    void rightRotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;

        if (x->right != nil) {
            x->right->parent = y;
        }

        x->parent = y->parent;

        if (y->parent == nil) {
            root = x;
        }
        else if (y == y->parent->right) {
            y->parent->right = x;
        }
        else {
            y->parent->left = x;
        }

        x->right = y;
        y->parent = x;
    }
    // ========== INSERTION ==========
    void insertFixup(Node* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;

                if (y->color == RED) {
                    // Case 1: Uncle is RED
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->right) {
                        // Case 2: z is right child
                        z = z->parent;
                        leftRotate(z);
                    }
                    // Case 3: z is left child
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            }
            else {
                // Mirror cases
                Node* y = z->parent->parent->left;

                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }
                else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    void bstInsert(Node* z) {
        Node* y = nil;
        Node* x = root;

        while (x != nil) {
            y = x;
            if (z->price < x->price) {
                x = x->left;
            }
            else if (z->price > x->price) {
                x = x->right;
            }
            else {
                // Same price: use itemID as tie-breaker
                if (z->itemID < x->itemID) {
                    x = x->left;
                }
                else {
                    x = x->right;
                }
            }
        }

        z->parent = y;

        if (y == nil) {
            root = z;
        }
        else if (z->price < y->price) {
            y->left = z;
        }
        else if (z->price > y->price) {
            y->right = z;
        }
        else {
            // Same price: use itemID
            if (z->itemID < y->itemID) {
                y->left = z;
            }
            else {
                y->right = z;
            }
        }

        z->left = nil;
        z->right = nil;
        z->color = RED;

        insertFixup(z);
    }

    // ========== DELETION - ALL CASES ==========

    // Helper: Transplant node u with node v
    void transplant(Node* u, Node* v) {
        if (u->parent == nil) {
            root = v;
        }
        else if (u == u->parent->left) {
            u->parent->left = v;
        }
        else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // Helper: Find minimum node in subtree
    Node* minimum(Node* node) {
        while (node->left != nil) {
            node = node->left;
        }
        return node;
    }

    // Helper: Find node by itemID
    Node* findNode(int itemID) {
        return findHelper(root, itemID);
    }

    Node* findHelper(Node* node, int itemID) {
        if (node == nil) return nil;

        if (node->itemID == itemID) return node;

        Node* leftResult = findHelper(node->left, itemID);
        if (leftResult != nil) return leftResult;

        return findHelper(node->right, itemID);
    }

    // MAIN DELETE FIXUP FUNCTION - Handles all 8 cases
    void deleteFixup(Node* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;

                // Case 1: w is RED
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }

                // Case 2: w's children are both BLACK
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                }
                else {
                    // Case 3: w's right child is BLACK
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }

                    // Case 4
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            }
            else {
                // Mirror cases (x is right child)
                Node* w = x->parent->left;

                // Case 1: w is RED
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }

                // Case 2: w's children are both BLACK
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                }
                else {
                    // Case 3: w's left child is BLACK
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }

                    // Case 4
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    // MAIN DELETE FUNCTION
    void rbDelete(Node* z) {
        Node* y = z;
        Node* x;
        Color yOriginalColor = y->color;

        if (z->left == nil) {
            // Case 1: No left child
            x = z->right;
            transplant(z, z->right);
        }
        else if (z->right == nil) {
            // Case 2: No right child
            x = z->left;
            transplant(z, z->left);
        }
        else {
            // Case 3: Two children
            y = minimum(z->right);
            yOriginalColor = y->color;
            x = y->right;

            if (y->parent == z) {
                x->parent = y;
            }
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }

            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        if (yOriginalColor == BLACK) {
            deleteFixup(x);
        }

        delete z;
    }

    // ========== CLEANUP ==========
    void clearTree(Node* node) {
        if (node == nil) return;
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }
public:
    ConcreteAuctionTree() {
        nil = new Node(-1, -1);
        nil->color = BLACK;
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }


    ~ConcreteAuctionTree() {
        clearTree(root);
        delete nil;
    }

    void insertItem(int itemID, int price) override {
        // Check if item exists
        deleteItem(itemID);
        Node* newNode = new Node(itemID, price);
        bstInsert(newNode);

    }

    void deleteItem(int itemID) override {
        Node* z = findNode(itemID);
        if (z == nil) return;
        rbDelete(z);

    }
};

// =========================================================
// PART B: INVENTORY SYSTEM (Dynamic Programming)
// =========================================================

int InventorySystem::optimizeLootSplit(int n, vector<int>& coins) {
    // TODO: Implement partition problem using DP
    // Goal: Minimize |sum(subset1) - sum(subset2)|
    // Hint: Use subset sum DP to find closest sum to total/2
    return 0;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    return 0;
}

long long InventorySystem::countStringPossibilities(string s) {
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings
    return 0;
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================

bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {
    // TODO: Implement path existence check using BFS or DFS
    // edges are bidirectional
    return false;
}

long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate,
    vector<vector<int>>& roadData) {
    // TODO: Implement Minimum Spanning Tree (Kruskal's or Prim's)
    // roadData[i] = {u, v, goldCost, silverCost}
    // Total cost = goldCost * goldRate + silverCost * silverRate
    // Return -1 if graph cannot be fully connected
    return -1;
}

string WorldNavigator::sumMinDistancesBinary(int n, vector<vector<int>>& roads) {
    // TODO: Implement All-Pairs Shortest Path (Floyd-Warshall)
    // Sum all shortest distances between unique pairs (i < j)
    // Return the sum as a binary string
    // Hint: Handle large numbers carefully
    return "0";
}

// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char>& tasks, int n) {
    // TODO: Implement task scheduler with cooling time
    // Same task must wait 'n' intervals before running again
    // Return minimum total intervals needed (including idle time)
    // Hint: Use greedy approach with frequency counting
    return 0;
}

//=========================================================
//FACTORY FUNCTIONS (Required for Testing)
//=========================================================

extern "C" {
    PlayerTable* createPlayerTable() {
        return new ConcretePlayerTable();
    }

    Leaderboard* createLeaderboard() {
        return new ConcreteLeaderboard();
    }

    AuctionTree* createAuctionTree() {
        return new ConcreteAuctionTree();
    }
}
