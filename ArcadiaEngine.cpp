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
#include <utility>
#include <tuple>

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
    int sum = 0;
    for (int c : coins) sum += c;

    vector<bool> dp(sum + 1, false);
    dp[0] = true;

    for (int c : coins)
        for (int s = sum; s >= c; s--)
            dp[s] = dp[s] || dp[s - c];

    int best = sum;
    for (int s = 0; s <= sum; s++)
        if (dp[s])
            best = min(best, abs(sum - 2 * s));

    return best;
}

int InventorySystem::maximizeCarryValue(int capacity, vector<pair<int, int>>& items) {
    // TODO: Implement 0/1 Knapsack using DP
    // items = {weight, value} pairs
    // Return maximum value achievable within capacity
    vector<int> dp(capacity + 1, 0);

    for (auto& it : items)
        for (int w = capacity; w >= it.first; w--)
            dp[w] = max(dp[w], dp[w - it.first] + it.second);

    return dp[capacity];
}

long long InventorySystem::countStringPossibilities(string s) {
    // TODO: Implement string decoding DP
    // Rules: "uu" can be decoded as "w" or "uu"
    //        "nn" can be decoded as "m" or "nn"
    // Count total possible decodings
    int n = s.size();
    vector<long long> dp(n + 1, 0);
    dp[n] = 1;  // empty string has one decoding

    for (int i = n - 1; i >= 0; i--) {
        // Take single character
        dp[i] = dp[i + 1];

        // Take special pairs
        if (i + 1 < n) {
            if ((s[i] == 'u' && s[i + 1] == 'u') ||
                (s[i] == 'n' && s[i + 1] == 'n')) {
                dp[i] += dp[i + 2];
            }
        }
    }

    return dp[0];
}

// =========================================================
// PART C: WORLD NAVIGATOR (Graphs)
// =========================================================
struct Edge {
    long long cost;
    int u, v;
    bool operator<(const Edge& other) const {
        return cost < other.cost;
    }
};

struct DSU {
    std::vector<int> parent;
    std::vector<int> rank;

    DSU(int n) {
        parent.resize(n);
        std::iota(parent.begin(), parent.end(), 0);
        rank.assign(n, 0);
    }

    int Find(int i) {
        if (parent[i] == i)
            return i;
        return parent[i] = Find(parent[i]);
    }

    bool Union(int i, int j) {
        int root_i = Find(i);
        int root_j = Find(j);

        if (root_i != root_j) {
            if (rank[root_i] < rank[root_j]) {
                parent[root_i] = root_j;
            }
            else if (rank[root_i] > rank[root_j]) {
                parent[root_j] = root_i;
            }
            else {
                parent[root_j] = root_i;
                rank[root_i]++;
            }
            return true;
        }
        return false;
    }
};


    bool WorldNavigator::pathExists(int n, vector<vector<int>>& edges, int source, int dest) {
        std::vector<std::vector<int>> List(n);
        for (const auto& edge : edges) {
            int u = edge[0];
            int v = edge[1];

            List[u].push_back(v);
            List[v].push_back(u);
        }
        std::queue<int> q;
        q.push(source);
        std::vector<bool> visited(n, false);
        visited[source] = true;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            if (u == dest) {
                return true;
            }
            for (int v : List[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }
        return false;
    }

    long long WorldNavigator::minBribeCost(int n, int m, long long goldRate, long long silverRate, vector<vector<int>>& roads) {
        std::vector<Edge> edges;

        for (const auto& road : roads) {
            int u = road[0];
            int v = road[1];
            int gold = road[2];
            int silver = road[3];

            long long cost = (long long)gold * goldRate + (long long)silver * silverRate;
            edges.push_back({ cost, u, v });
        }

        std::sort(edges.begin(), edges.end());

        DSU dsu(n);
        long long MstCost = 0;
        int edgesCount = 0;

        for (const auto& edge : edges) {
            if (edgesCount == n - 1)
                break;

            if (dsu.Union(edge.u, edge.v)) {
                MstCost += edge.cost;
                edgesCount++;
            }
        }
        return MstCost;
    }

    string WorldNavigator::sumMinDistancesBinary(int n, std::vector<vector<int>>& roads) {
        const long long infinity = 1e15;
        std::vector<std::vector<long long>> dist(n, std::vector<long long>(n, infinity));

        for (int i = 0; i < n; ++i) {
            dist[i][i] = 0;
        }

        for (const auto& road : roads) {
            int u = road[0];
            int v = road[1];
            int length = road[2];

            dist[u][v] = std::min(dist[u][v], (long long)length);
            dist[v][u] = std::min(dist[v][u], (long long)length);
        }

        for (int k = 0; k < n; ++k) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (dist[i][k] != infinity && dist[k][j] != infinity) {
                        dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
                    }
                }
            }
        }

        long long totalSum = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (dist[i][j] != infinity) {
                    totalSum += dist[i][j];
                }
            }
        }

        std::string result = "";
        if (totalSum == 0) return "0";

        while (totalSum > 0) {
            result = (totalSum % 2 == 0 ? "0" : "1") + result;
            totalSum /= 2;
        }
        return result;
    }


// =========================================================
// PART D: SERVER KERNEL (Greedy)
// =========================================================

int ServerKernel::minIntervals(vector<char>& tasks, int n) {
    if (tasks.size() == 0)return 0;
    int freq[26] = { 0 };
    for (char c : tasks)
        freq[c - 'A']++;

    int maxFreq = 0;
    for (int i = 0; i < 26; i++)
        maxFreq = max(maxFreq, freq[i]);

    int count_maxFreq = 0;
    for (int i = 0; i < 26; i++) {
        if (freq[i] == maxFreq)
            count_maxFreq++;
    }
    int sizeTasks = tasks.size();
    return max(sizeTasks, (((maxFreq - 1) * (n + 1)) + count_maxFreq));

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
