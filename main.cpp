#include <iostream>
#include "ArcadiaEngine.h"  // Include the HEADER, not .cpp

using namespace std;

// Forward declarations of factory functions
extern "C" {
    PlayerTable* createPlayerTable();
    Leaderboard* createLeaderboard();
    AuctionTree* createAuctionTree();
}

void testPlayerTableBasic() {
    cout << "=== PlayerTable Basic Tests ===" << endl;
    PlayerTable* table = createPlayerTable();

    // Test 1: Insert and Search
    table->insert(1001, "Alice");
    table->insert(1002, "Bob");
    table->insert(1003, "Charlie");

    cout << "Search 1001: " << table->search(1001) << " (Expected: Alice)" << endl;
    cout << "Search 1002: " << table->search(1002) << " (Expected: Bob)" << endl;
    cout << "Search 1003: " << table->search(1003) << " (Expected: Charlie)" << endl;

    // Test 2: Search non-existent
    cout << "Search 9999: " << table->search(9999) << " (Expected: empty string)" << endl;


    // Clean up
    delete table;
}

void testPlayerTableCollisions() {
    cout << "\n=== PlayerTable Collision Tests ===" << endl;
    PlayerTable* table = createPlayerTable();

    // Insert keys that might cause collisions
    vector<pair<int, string>> players = {
        {17, "Player17"},
        {34, "Player34"},  // Might collide with 17
        {51, "Player51"},  // Might collide
        {68, "Player68"},
        {85, "Player85"}
    };

    for (auto& p : players) {
        table->insert(p.first, p.second);
    }

    // Verify all can be found
    for (auto& p : players) {
        string result = table->search(p.first);
        cout << "Search " << p.first << ": " << result
            << " (Expected: " << p.second << ")" << endl;
    }

    delete table;
}

void testPlayerTableRehashing() {
    cout << "\n=== PlayerTable Rehashing Test ===" << endl;
    PlayerTable* table = createPlayerTable();

    // Insert many players to trigger rehashing
    int count = 100;
    for (int i = 1; i <= count; i++) {
        table->insert(i, "Player" + to_string(i));
    }

    // Verify all can be found
    int found = 0;
    for (int i = 1; i <= count; i++) {
        if (!table->search(i).empty()) {
            found++;
        }
    }
    cout << "Inserted " << count << " players, found " << found
        << " (Expected: " << count << ")" << endl;

    delete table;
}
void testLeaderboard() {
    Leaderboard* board=createLeaderboard();

    cout << "=== Testing Skip List Leaderboard ===" << endl;

    // Test 1: Basic operations
    board->addScore(1001, 1500);
    board->addScore(1002, 2000);
    board->addScore(1003, 1800);

    auto top2 = board->getTopN(2);
    cout << "Top 2: ";
    for (int id : top2) cout << id << " ";
    cout << endl;

    // Test 2: Update
    board->addScore(1001, 2200);
    top2 = board->getTopN(2);
    cout << "After update - Top 2: ";
    for (int id : top2) cout << id << " ";
    cout << endl;

    // Test 3: Remove
    board->removePlayer(1002);
    auto top3 = board->getTopN(3);
    cout << "After removal - Top 3: ";
    for (int id : top3) cout << id << " ";
    cout << endl;

    // Test 4: Same scores
    board->addScore(1004, 2200);  // Same as 1001
    board->addScore(1005, 2200);  // Same as 1001
    auto top5 = board->getTopN(5);
    cout << "With same scores - Top 5: ";
    for (int id : top5) cout << id << " ";
    cout << endl;
}
void testWithExpectedOutput() {
    cout << "\n\n=========================================" << endl;
    cout << "   TEST WITH STEP-BY-STEP EXPLANATION   " << endl;
    cout << "=========================================" << endl;
    AuctionTree* tree = createAuctionTree();
    

    cout << "\n--- Step 1: Insert prices in order 100, 50, 150 ---" << endl;
    cout << "This creates a simple tree that might need rebalancing:" << endl;
    tree-> insertItem(1, 100);
    tree->insertItem(2, 50);
    tree->insertItem(3, 150);
  

    cout << "\n--- Step 2: Insert 75 (causes rotation) ---" << endl;
    cout << "This might cause a rotation to maintain RB properties:" << endl;
    tree->insertItem(4, 75);
   

    cout << "\n--- Step 3: Insert 125 ---" << endl;
    tree->insertItem(5, 125);
   

    cout << "\n--- Step 4: Insert 25 ---" << endl;
    tree->insertItem(6, 25);
    

    cout << "\n--- Step 5: Delete leaf 25 ---" << endl;
    cout << "Deleting a leaf node (simple case):" << endl;
    tree->deleteItem(6);
  

    cout << "\n--- Step 6: Delete node with one child (75) ---" << endl;
    cout << "First, let's see current structure:" << endl;
   
    cout << "\nNow delete 75:" << endl;
    tree->deleteItem(4);
 

    cout << "\n--- Step 7: Delete node with two children (100) ---" << endl;
    cout << "This is the complex case - find successor:" << endl;
    tree->deleteItem(1);
  
    cout << "\n--- Step 8: Test duplicate prices ---" << endl;
    cout << "Insert items with same price (should use itemID as tie-breaker):" << endl;
    tree->insertItem(7, 200);
    tree->insertItem(8, 200);
    tree->insertItem(9, 200);
  

    cout << "\n--- Step 9: Delete from duplicates ---" << endl;
    tree->deleteItem(8);
   
}
void finalVerificationTest() {
    cout << "\n\n=========================================" << endl;
    cout << "   FINAL VERIFICATION TEST   " << endl;
    cout << "=========================================" << endl;

    AuctionTree* tree = createAuctionTree();

    cout << "\n1. Insert descending prices (worst case for BST):" << endl;
    for (int i = 10; i >= 1; i--) {
        tree->insertItem(i, i * 10);
    }
  

    cout << "\n2. Insert ascending prices:" << endl;
    for (int i = 11; i <= 20; i++) {
        tree->insertItem(i, i * 10);
    }
 

    cout << "\n3. Delete all even-numbered items:" << endl;
    for (int i = 2; i <= 20; i += 2) {
        tree->deleteItem(i);
    }
   

    cout << "\n4. Delete all odd-numbered items (should empty tree):" << endl;
    for (int i = 1; i <= 19; i += 2) {
        tree->deleteItem(i);
    }
 

    cout << "\n5. Final check - insert after empty:" << endl;
    tree->insertItem(100, 500);
   
    delete tree;
}
int main(){
    /*testPlayerTableBasic();
    testPlayerTableCollisions();
    testPlayerTableRehashing();
    testLeaderboard();
    testWithExpectedOutput();
    finalVerificationTest();*/
    return 0;
}
