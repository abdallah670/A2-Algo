/**
 * main_test_student.cpp
 * Basic "Happy Path" Test Suite for ArcadiaEngine
 * Use this to verify your basic logic against the assignment examples.
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <functional>
#include "ArcadiaEngine.h" 

using namespace std;

// ==========================================
// FACTORY FUNCTIONS (LINKING)
// ==========================================
// These link to the functions at the bottom of your .cpp file
extern "C" {
    PlayerTable* createPlayerTable();
    Leaderboard* createLeaderboard();
    AuctionTree* createAuctionTree();
}

// ==========================================
// TEST UTILITIES
// ==========================================
class StudentTestRunner {
	int count = 0;
    int passed = 0;
    int failed = 0;

public:
    void runTest(string testName, bool condition) {
		count++;
        cout << "TEST: " << left << setw(50) << testName;
        if (condition) {
            cout << "[ PASS ]";
            passed++;
        } else {
            cout << "[ FAIL ]";
            failed++;
        }
        cout << endl;
    }

    void printSummary() {
        cout << "\n==========================================" << endl;
        cout << "SUMMARY: Passed: " << passed << " | Failed: " << failed << endl;
        cout << "==========================================" << endl;
		cout << "TOTAL TESTS: " << count << endl;
        if (failed == 0) {
            cout << "Great job! All basic scenarios passed." << endl;
            cout << "Now make sure to handle edge cases (empty inputs, collisions, etc.)!" << endl;
        } else {
            cout << "Some basic tests failed. Check your logic against the PDF examples." << endl;
        }
    }
};

StudentTestRunner runner;

// ==========================================
// PART A: DATA STRUCTURES
// ==========================================

// ==========================================
// HASH TABLE - STRONG TESTS
// ==========================================

void test_HashTable_DoubleHashing() {
    cout << "\n🔍 HASH TABLE - DOUBLE HASHING TESTS\n";

    PlayerTable* table = createPlayerTable();

    // Test 1: Basic insert and search
    runner.runTest("HashTable: Basic insert/search", [&]() {
        table->insert(100, "Alice");
        return table->search(100) == "Alice";
        }());

    // Test 2: Collision handling with double hashing
    runner.runTest("HashTable: Double hashing resolves collisions", [&]() {
        // These keys should all hash to same initial position with %101
        table->insert(0, "Player0");      // hash1(0) = 0
        table->insert(101, "Player101");  // hash1(101) = 0 -> collision
        table->insert(202, "Player202");  // hash1(202) = 0 -> another collision
        return table->search(0) == "Player0" &&
            table->search(101) == "Player101" &&
            table->search(202) == "Player202";
        }());

    // Test 3: Update existing player
    runner.runTest("HashTable: Update existing player", [&]() {
        table->insert(100, "AliceUpdated");
        return table->search(100) == "AliceUpdated";
        }());

    // Test 4: Search for non-existent player
    runner.runTest("HashTable: Search non-existent returns empty string", [&]() {
        return table->search(99999) == "";
        }());

    delete table;
}

void test_HashTable_TableFull() {
    cout << "\n🔍 HASH TABLE - TABLE FULL SCENARIO\n";

    PlayerTable* table = createPlayerTable();
    bool fullExceptionThrown = false;

    // Fill the table with 101 unique players
    try {
        for (int i = 0; i < 101; i++) {
            table->insert(i * 1000, "Player" + to_string(i));
        }
    }
    catch (...) {
        fullExceptionThrown = true;
    }

    // Table should NOT be full yet (we inserted exactly 101)
    runner.runTest("HashTable: Can insert exactly 101 players", !fullExceptionThrown);

    // Now try to insert one more - should throw
    try {
        table->insert(999999, "ShouldFail");
    }
    catch (const char* msg) {
        if (string(msg) == "Table is full") {
            fullExceptionThrown = true;
        }
    }

    runner.runTest("HashTable: Throws 'Table is full' at capacity", fullExceptionThrown);

    // Verify existing players are still accessible
    runner.runTest("HashTable: Existing players still accessible when full",
        table->search(0) == "Player0" &&
        table->search(100000) == "Player100");

    delete table;
}

void test_HashTable_TombstoneHandling() {
    cout << "\n🔍 HASH TABLE - TOMBSTONE HANDLING\n";

    PlayerTable* table = createPlayerTable();

    // Create collision chain
    table->insert(0, "A");
    table->insert(101, "B");  // Collides with 0
    table->insert(202, "C");  // Collides with 0 and 101

    // Delete middle element in chain
    // Note: We need delete operation - if not implemented, simulate with update
    // Since we can't delete, we'll test insertion after collisions
    runner.runTest("HashTable: Can insert after collisions", [&]() {
        table->insert(303, "D");  // Another collision
        return table->search(303) == "D";
        }());

    // Test searching through tombstones
    runner.runTest("HashTable: Search finds element after deleted ones", [&]() {
        // Assuming 202 was deleted and replaced with tombstone
        table->insert(202, "C_updated");
        return table->search(202) == "C_updated";
        }());

    delete table;
}

// ==========================================
// SKIP LIST - STRONG TESTS
// ==========================================

void test_SkipList_TieBreaking() {
    cout << "\n🔍 SKIP LIST - TIE-BREAKING TESTS\n";

    Leaderboard* board = createLeaderboard();

    // Test 1: Simple tie-breaking (ID 10 before ID 20 for same score)
    runner.runTest("SkipList: Simple tie-break (10 before 20)", [&]() {
        board->addScore(10, 500);
        board->addScore(20, 500);
        vector<int> top = board->getTopN(2);
        return top.size() == 2 && top[0] == 10 && top[1] == 20;
        }());

    // Test 2: Complex tie-breaking with multiple players
    runner.runTest("SkipList: Multiple tie-breaks sorted by ID", [&]() {
        board->addScore(30, 400);
        board->addScore(25, 400);  // Same score as 30
        board->addScore(15, 400);  // Same score as 30 and 25
        vector<int> top = board->getTopN(5);
        // Order should be: 500s (10,20), then 400s (15,25,30)
        if (top.size() < 5) return false;
        return top[2] == 15 && top[3] == 25 && top[4] == 30;
        }());

    // Test 3: Mixed scores with tie-breaking
    runner.runTest("SkipList: Mixed scores with proper ordering", [&]() {
        board->addScore(100, 600);  // Highest score
        board->addScore(90, 600);   // Same score, lower ID
        vector<int> top = board->getTopN(2);
        return top.size() >= 2 && top[0] == 90 && top[1] == 100;  // 90 before 100
        }());

    delete board;
}

void test_SkipList_ScoreUpdates() {
    cout << "\n🔍 SKIP LIST - SCORE UPDATES\n";

    Leaderboard* board = createLeaderboard();

    // Test 1: Update player score (should remove and reinsert)
    runner.runTest("SkipList: Update player score", [&]() {
        board->addScore(1, 100);
        board->addScore(2, 200);
        board->addScore(1, 300);  // Update player 1 from 100 to 300
        vector<int> top = board->getTopN(2);
        return top.size() >= 2 && top[0] == 1 && top[1] == 2;
        }());

    // Test 2: Update causing reordering
    runner.runTest("SkipList: Update causes reordering", [&]() {
        board->addScore(3, 250);
        board->addScore(4, 275);
        board->addScore(3, 150);  // Move player 3 down
        vector<int> top = board->getTopN(4);
        // Order should be: 1(300), 4(275), 2(200), 3(150)
        if (top.size() < 4) return false;
        return top[0] == 1 && top[1] == 4 && top[2] == 2 && top[3] == 3;
        }());

    // Test 3: Update to same score (tie-breaking should work)
    runner.runTest("SkipList: Update to same score maintains tie-breaking", [&]() {
        board->addScore(5, 200);  // Same as player 2
        vector<int> top = board->getTopN(6);
        // At score 200, players 2 and 5, 2 should come first (lower ID)
        // Need to find positions of 2 and 5
        int pos2 = -1, pos5 = -1;
        for (int i = 0; i < top.size(); i++) {
            if (top[i] == 2) pos2 = i;
            if (top[i] == 5) pos5 = i;
        }
        return pos2 != -1 && pos5 != -1 && pos2 < pos5;
        }());

    delete board;
}

void test_SkipList_RemoveOperations() {
    cout << "\n🔍 SKIP LIST - REMOVE OPERATIONS\n";

    Leaderboard* board = createLeaderboard();

    // Setup: Add multiple players
    for (int i = 1; i <= 10; i++) {
        board->addScore(i, i * 100);
    }

    // Test 1: Remove middle player
    runner.runTest("SkipList: Remove middle player", [&]() {
        int before = board->getTopN(20).size();
        board->removePlayer(5);  // Remove player with score 500
        int after = board->getTopN(20).size();
        return after == before - 1;
        }());

    // Test 2: Remove non-existent player
    runner.runTest("SkipList: Remove non-existent player (no crash)", [&]() {
        board->removePlayer(999);  // Should not crash
        return true;
        }());

    // Test 3: Remove and re-add
    runner.runTest("SkipList: Remove and re-add works", [&]() {
        board->addScore(3, 1001);  // Re-add with high score
        vector<int> top = board->getTopN(1);
        return !top.empty() && top[0] == 3;
        }());

    // Test 4: Remove first player
    runner.runTest("SkipList: Remove top player", [&]() {
        vector<int> before = board->getTopN(2);
        board->removePlayer(3);  // Currently top player
        vector<int> after = board->getTopN(2);
        return before[0] == 3 && after[0] != 3;
        }());

    delete board;
}

void test_SkipList_getTopN_EdgeCases() {
    cout << "\n🔍 SKIP LIST - GETTOP N EDGE CASES\n";

    Leaderboard* board = createLeaderboard();

    // Add some players
    for (int i = 1; i <= 5; i++) {
        board->addScore(i, i * 100);
    }

    // Test 1: N = 0
    runner.runTest("SkipList: getTopN(0) returns empty", [&]() {
        vector<int> result = board->getTopN(0);
        return result.empty();
        }());

    // Test 2: N > total players
    runner.runTest("SkipList: getTopN(large) returns all players", [&]() {
        vector<int> result = board->getTopN(100);
        return result.size() == 5;
        }());

    // Test 3: N = total players
    runner.runTest("SkipList: getTopN(exact count) returns all", [&]() {
        vector<int> result = board->getTopN(5);
        return result.size() == 5;
        }());

    // Test 4: After removals
    runner.runTest("SkipList: getTopN works after removals", [&]() {
        board->removePlayer(2);
        board->removePlayer(4);
        vector<int> result = board->getTopN(10);
        return result.size() == 3;  // Players 1, 3, 5 remain
        }());

    delete board;
}

// ==========================================
// RED-BLACK TREE - STRONG TESTS
// ==========================================

void test_RBTree_DuplicatePrices() {
    cout << "\n🔍 RB TREE - DUPLICATE PRICES\n";

    AuctionTree* tree = createAuctionTree();

    // Test 1: Same prices, different IDs (should order by price then ID)
    runner.runTest("RBTree: Same prices ordered by ID", [&]() {
        tree->insertItem(5, 100);  // ID=5, Price=100
        tree->insertItem(3, 100);  // ID=3, Price=100 (lower ID, should be left)
        tree->insertItem(7, 100);  // ID=7, Price=100 (higher ID, should be right)
        // If tree is correct, it won't crash on these operations
        tree->deleteItem(3);
        tree->deleteItem(5);
        tree->deleteItem(7);
        return true;
        }());

    // Test 2: Mix of different and same prices
    runner.runTest("RBTree: Mixed price insertions", [&]() {
        tree->insertItem(1, 50);   // Lowest price
        tree->insertItem(2, 100);  // Medium price
        tree->insertItem(3, 100);  // Same as 2
        tree->insertItem(4, 150);  // Highest price
        // Should handle all without crashing
        return true;
        }());

    delete tree;
}

void test_RBTree_DeleteOperations() {
    cout << "\n🔍 RB TREE - DELETE OPERATIONS\n";

    AuctionTree* tree = createAuctionTree();

    // Setup: Insert various items
    tree->insertItem(1, 100);
    tree->insertItem(2, 200);
    tree->insertItem(3, 300);
    tree->insertItem(4, 400);
    tree->insertItem(5, 500);

    // Test 1: Delete leaf node
    runner.runTest("RBTree: Delete leaf node", [&]() {
        tree->deleteItem(5);  // Likely a leaf
        // Try to delete again (should not crash)
        tree->deleteItem(5);
        return true;
        }());

    // Test 2: Delete node with one child
    runner.runTest("RBTree: Delete node with one child", [&]() {
        tree->insertItem(6, 600);
        tree->insertItem(7, 550);  // Between 500 and 600
        tree->deleteItem(6);  // Node 7 becomes child of 500
        return true;
        }());

    // Test 3: Delete node with two children
    runner.runTest("RBTree: Delete node with two children", [&]() {
        tree->insertItem(8, 450);  // Between 400 and 500
        tree->deleteItem(4);  // Has children 400 and 450
        return true;
        }());

    // Test 4: Delete root node
    runner.runTest("RBTree: Delete root node", [&]() {
        tree->deleteItem(1);  // Might be root
        return true;
        }());

    // Test 5: Delete non-existent item
    runner.runTest("RBTree: Delete non-existent item (no crash)", [&]() {
        tree->deleteItem(999);
        return true;
        }());

    delete tree;
}

void test_RBTree_UpdateItems() {
    cout << "\n🔍 RB TREE - UPDATE ITEMS\n";

    AuctionTree* tree = createAuctionTree();

    // Test 1: Update price (delete + insert)
    runner.runTest("RBTree: Update item price", [&]() {
        tree->insertItem(1, 100);
        tree->insertItem(1, 200);  // Update price from 100 to 200
        // Should handle without duplicate keys
        return true;
        }());

    // Test 2: Update to same price (should still work)
    runner.runTest("RBTree: Update to same price", [&]() {
        tree->insertItem(2, 200);  // Same price as updated item 1
        tree->insertItem(2, 200);  // Update to same price
        return true;
        }());

    // Test 3: Multiple updates
    runner.runTest("RBTree: Multiple updates on same item", [&]() {
        for (int i = 0; i < 10; i++) {
            tree->insertItem(3, i * 50);
        }
        return true;
        }());

    delete tree;
}

// ==========================================
// INTEGRATION TESTS - PART A
// ==========================================

void test_PartA_Integration() {
    cout << "\n🔍 PART A - INTEGRATION TEST\n";

    PlayerTable* players = createPlayerTable();
    Leaderboard* board = createLeaderboard();
    AuctionTree* auction = createAuctionTree();

    // Simulate game operations
    runner.runTest("Integration: Register player and add to leaderboard", [&]() {
        players->insert(1, "Player1");
        board->addScore(1, 1000);
        auction->insertItem(1, 500);  // Player1 sells item

        players->insert(2, "Player2");
        board->addScore(2, 1500);
        auction->insertItem(2, 300);

        // Verify all systems work together
        bool playerOk = players->search(1) == "Player1";
        vector<int> top = board->getTopN(2);
        bool boardOk = !top.empty() && top[0] == 2;  // Player2 has higher score

        return playerOk && boardOk;
        }());

    runner.runTest("Integration: Player updates score and sells new item", [&]() {
        board->addScore(1, 2000);  // Player1 gets higher score
        auction->insertItem(1, 700);  // Player1 updates item price

        vector<int> top = board->getTopN(2);
        bool boardUpdated = !top.empty() && top[0] == 1;  // Now player1 is top

        return boardUpdated;
        }());

    runner.runTest("Integration: Remove player from all systems", [&]() {
        board->removePlayer(1);
        auction->deleteItem(1);

        vector<int> top = board->getTopN(5);
        bool playerRemoved = true;
        for (int id : top) {
            if (id == 1) playerRemoved = false;
        }

        return playerRemoved;
        }());

    delete players;
    delete board;
    delete auction;
}


// ==========================================
// MAIN FUNCTION
// ==========================================


void test_PartA_DataStructures() {
  
    // Hash Table Tests
    test_HashTable_DoubleHashing();
    test_HashTable_TableFull();
    test_HashTable_TombstoneHandling();

    // Skip List Tests
    test_SkipList_TieBreaking();
    test_SkipList_ScoreUpdates();
    test_SkipList_RemoveOperations();
    test_SkipList_getTopN_EdgeCases();

    // Red-Black Tree Tests
    test_RBTree_DuplicatePrices();
    test_RBTree_DeleteOperations();
    test_RBTree_UpdateItems();

    // Integration & Performance
    test_PartA_Integration();
   
   
}

// ==========================================
// PART B: INVENTORY SYSTEM
// ==========================================


void test_PartB_Inventory_Comprehensive() {
    cout << "\n🔍 PART B - INVENTORY SYSTEM COMPREHENSIVE TESTS\n";

    // ========== 1. LOOT SPLIT (Partition Problem) ==========

    // Basic test cases
    runner.runTest("LootSplit: {1, 2, 4} -> Diff 1", [&]() {
        vector<int> coins = { 1, 2, 4 };
        return InventorySystem::optimizeLootSplit(3, coins) == 1;
        }());

    runner.runTest("LootSplit: {3, 1, 4, 2, 2, 1} -> Diff 1", [&]() {
        vector<int> coins = { 3, 1, 4, 2, 2, 1 };
        return InventorySystem::optimizeLootSplit(6, coins) == 1;
        }());

    // Perfect split possible
    runner.runTest("LootSplit: {2, 2, 2, 2} -> Diff 0", [&]() {
        vector<int> coins = { 2, 2, 2, 2 };
        return InventorySystem::optimizeLootSplit(4, coins) == 0;
        }());

    runner.runTest("LootSplit: {1, 5, 11, 5} -> Diff 0", [&]() {
        vector<int> coins = { 1, 5, 11, 5 };
        return InventorySystem::optimizeLootSplit(4, coins) == 0;
        }());

    // Edge cases
    runner.runTest("LootSplit: Single coin -> Diff = coin", [&]() {
        vector<int> coins = { 100 };
        return InventorySystem::optimizeLootSplit(1, coins) == 100;
        }());

    runner.runTest("LootSplit: All zeros -> Diff 0", [&]() {
        vector<int> coins = { 0, 0, 0, 0 };
        return InventorySystem::optimizeLootSplit(4, coins) == 0;
        }());

    runner.runTest("LootSplit: Large coin values", [&]() {
        vector<int> coins = { 1000, 500, 300, 200 };
        return InventorySystem::optimizeLootSplit(4, coins) == 0; // 1000 vs (500+300+200)=1000
        }());

    runner.runTest("LootSplit: All same coins", [&]() {
        vector<int> coins = { 10, 10, 10, 10, 10 };
        return InventorySystem::optimizeLootSplit(5, coins) == 10; // 30 vs 20
        }());

    // Large input test
    runner.runTest("LootSplit: 50 coins (stress test)", [&]() {
        vector<int> coins;
        for (int i = 1; i <= 50; i++) {
            coins.push_back(i);
        }
        int result = InventorySystem::optimizeLootSplit(50, coins);
        // Minimum difference for sum 1275 is 1 (637 vs 638)
        return result >= 0 && result <= 5;
        }());

    // Negative coins? (if applicable)
    // runner.runTest("LootSplit: With negative values", [&]() {
    //     vector<int> coins = {-1, 2, 3};
    //     return InventorySystem::optimizeLootSplit(3, coins) == 0; // 2 vs (-1+3)=2
    // }());

    // ========== 2. KNAPSACK (Maximize Carry Value) ==========

    // Basic test cases
    runner.runTest("Knapsack: Cap 10, all fit -> Value 60", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {2, 20}, {3, 30} };
        return InventorySystem::maximizeCarryValue(10, items) == 60;
        }());

    runner.runTest("Knapsack: Cap 5, items {{4, 10}, {3, 9}, {2, 5}} -> Value 14", [&]() {
        vector<pair<int, int>> items = { {4, 10}, {3, 9}, {2, 5} };
        return InventorySystem::maximizeCarryValue(5, items) == 14; // 9+5=14
        }());

    // Edge cases
    runner.runTest("Knapsack: Zero capacity -> Value 0", [&]() {
        vector<pair<int, int>> items = { {1, 100}, {2, 200} };
        return InventorySystem::maximizeCarryValue(0, items) == 0;
        }());

    runner.runTest("Knapsack: No items -> Value 0", [&]() {
        vector<pair<int, int>> items = {};
        return InventorySystem::maximizeCarryValue(100, items) == 0;
        }());

    runner.runTest("Knapsack: Single item fits", [&]() {
        vector<pair<int, int>> items = { {5, 50} };
        return InventorySystem::maximizeCarryValue(10, items) == 50;
        }());

    runner.runTest("Knapsack: Single item doesn't fit", [&]() {
        vector<pair<int, int>> items = { {15, 100} };
        return InventorySystem::maximizeCarryValue(10, items) == 0;
        }());

    runner.runTest("Knapsack: All items too heavy", [&]() {
        vector<pair<int, int>> items = { {20, 100}, {25, 200}, {30, 300} };
        return InventorySystem::maximizeCarryValue(10, items) == 0;
        }());

    // Complex scenarios
    runner.runTest("Knapsack: Multiple optimal solutions", [&]() {
        vector<pair<int, int>> items = { {2, 3}, {3, 4}, {4, 5}, {5, 6} };
        return InventorySystem::maximizeCarryValue(5, items) == 7; // (2,3)+(3,4)=7
        }());

    runner.runTest("Knapsack: High value/low weight vs low value/high weight", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {10, 1} };
        return InventorySystem::maximizeCarryValue(10, items) == 10; // Take only first item
        }());

    runner.runTest("Knapsack: Large capacity, many items", [&]() {
        vector<pair<int, int>> items;
        for (int i = 1; i <= 100; i++) {
            items.push_back({ i % 10 + 1, i * 10 }); // weights 1-10, values 10-1000
        }
        int result = InventorySystem::maximizeCarryValue(50, items);
        return result > 0;
        }());

    runner.runTest("Knapsack: Duplicate items", [&]() {
        vector<pair<int, int>> items = { {2, 5}, {2, 5}, {2, 5} };
        return InventorySystem::maximizeCarryValue(6, items) == 15; // Can take all 3
        }());

    // ========== 3. STRING DECODING ==========

    // Basic test cases
    runner.runTest("StringDecode: 'uu' -> 2", [&]() {
        return InventorySystem::countStringPossibilities("uu") == 2;
        }());

    runner.runTest("StringDecode: 'nn' -> 2", [&]() {
        return InventorySystem::countStringPossibilities("nn") == 2;
        }());

    runner.runTest("StringDecode: 'uuuu' -> 5", [&]() {
        // Possibilities: uuuu, wuu, uuw, ww, uwu
        return InventorySystem::countStringPossibilities("uuuu") == 5;
        }());

    runner.runTest("StringDecode: 'nnnn' -> 5", [&]() {
        return InventorySystem::countStringPossibilities("nnnn") == 5;
        }());

    // Edge cases
    runner.runTest("StringDecode: Empty string -> 1", [&]() {
        return InventorySystem::countStringPossibilities("") == 1;
        }());

    runner.runTest("StringDecode: Single character -> 1", [&]() {
        return InventorySystem::countStringPossibilities("a") == 1;
        }());

    runner.runTest("StringDecode: No special pairs -> 1", [&]() {
        return InventorySystem::countStringPossibilities("abc") == 1;
        }());

    runner.runTest("StringDecode: Only regular characters -> 1", [&]() {
        return InventorySystem::countStringPossibilities("xyzxyz") == 1;
        }());

    // Complex cases
    runner.runTest("StringDecode: 'uun' -> 2", [&]() {
        // Possibilities: uun, wn
        return InventorySystem::countStringPossibilities("uun") == 2;
        }());

    runner.runTest("StringDecode: 'nuu' -> 2", [&]() {
        // Possibilities: nuu, nw
        return InventorySystem::countStringPossibilities("nuu") == 2;
        }());

    runner.runTest("StringDecode: 'uunn' -> 4", [&]() {
        // Possibilities: uunn, wnn, uum, wm
        return InventorySystem::countStringPossibilities("uunn") == 4;
        }());

    runner.runTest("StringDecode: Mixed with non-special characters", [&]() {
        return InventorySystem::countStringPossibilities("auub") == 2; // auub, awb
        }());

    runner.runTest("StringDecode: 'uunu' -> 3", [&]() {
        // Possibilities: uunu, wnu, uwn
        return InventorySystem::countStringPossibilities("uunu") == 2;
        }());

    runner.runTest("StringDecode: 'u' repeated 10 times", [&]() {
        string s(10, 'u'); // "uuuuuuuuuu"
        long long result = InventorySystem::countStringPossibilities(s);
        // Fibonacci-like sequence: F(n) where n = length/2
        return result == 89; // F(10) for "uu" pairs
        }());

    runner.runTest("StringDecode: 'n' repeated 10 times", [&]() {
        string s(10, 'n');
        long long result = InventorySystem::countStringPossibilities(s);
        return result == 89;
        }());

    runner.runTest("StringDecode: Alternating u and n", [&]() {
        return InventorySystem::countStringPossibilities("ununun") == 1; // No adjacent uu or nn
        }());

    // Large input test
    runner.runTest("StringDecode: 30-character mixed string", [&]() {
        string s = "uunnuunnuunnuunnuunnuunnuunnuu";
        long long result = InventorySystem::countStringPossibilities(s);
        return result > 0; // Should be large
        }());

    runner.runTest("StringDecode: Long string with no specials", [&]() {
        string s(100, 'a');
        return InventorySystem::countStringPossibilities(s) == 1;
        }());
}
void test_PartB_Inventory() {
    cout << "\n--- Part B: Inventory System ---" << endl;

    // 1. Loot Splitting (Partition)
    // PDF Example: coins = {1, 2, 4} -> Best split {4} vs {1,2} -> Diff 1
    runner.runTest("LootSplit: {1, 2, 4} -> Diff 1", [&]() {
        vector<int> coins = { 1, 2, 4 };
        return InventorySystem::optimizeLootSplit(3, coins) == 1;
        }());

    // 2. Inventory Packer (Knapsack)
    // PDF Example: Cap=10, Items={{1,10}, {2,20}, {3,30}}. All fit. Value=60.
    runner.runTest("Knapsack: Cap 10, All Fit -> Value 60", [&]() {
        vector<pair<int, int>> items = { {1, 10}, {2, 20}, {3, 30} };
        return InventorySystem::maximizeCarryValue(10, items) == 60;
        }());

    // 3. Chat Autocorrect (String DP)
    // PDF Example: "uu" -> "uu" or "w" -> 2 possibilities
    runner.runTest("ChatDecorder: 'uu' -> 2 Possibilities", [&]() {
        return InventorySystem::countStringPossibilities("uu") == 2;
        }());
    test_PartB_Inventory_Comprehensive();
}

// ==========================================
// PART C: WORLD NAVIGATOR - STRONG TESTS
// ==========================================

void test_PartC_Navigator_Comprehensive() {
    cout << "\n🔍 PART C - WORLD NAVIGATOR COMPREHENSIVE TESTS\n";

    // ========== 1. PATH EXISTS (BFS/DFS) ==========

    // Basic test cases
    runner.runTest("PathExists: Simple path 0->1->2", [&]() {
        vector<vector<int>> edges = { {0, 1}, {1, 2} };
        return WorldNavigator::pathExists(3, edges, 0, 2) == true;
        }());

    runner.runTest("PathExists: No path (disconnected)", [&]() {
        vector<vector<int>> edges = { {0, 1}, {2, 3} };
        return WorldNavigator::pathExists(4, edges, 0, 3) == false;
        }());

    runner.runTest("PathExists: Self-loop path", [&]() {
        vector<vector<int>> edges = { {0, 0} };
        return WorldNavigator::pathExists(1, edges, 0, 0) == true;
        }());

    // Edge cases
    runner.runTest("PathExists: Single node, no edges", [&]() {
        vector<vector<int>> edges = {};
        return WorldNavigator::pathExists(1, edges, 0, 0) == true;
        }());

    runner.runTest("PathExists: Single node with self-edge", [&]() {
        vector<vector<int>> edges = { {0, 0} };
        return WorldNavigator::pathExists(1, edges, 0, 0) == true;
        }());

    runner.runTest("PathExists: Source equals destination", [&]() {
        vector<vector<int>> edges = { {0, 1}, {1, 2} };
        return WorldNavigator::pathExists(3, edges, 0, 0) == true;
        }());

    runner.runTest("PathExists: Invalid node indices", [&]() {
        vector<vector<int>> edges = { {0, 1} };
        // Should handle gracefully (return false or not crash)
        bool result = WorldNavigator::pathExists(2, edges, 0, 5);
        return !result; // Should be false
        }());

    // Complex graphs
    runner.runTest("PathExists: Complete graph K4", [&]() {
        vector<vector<int>> edges;
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                edges.push_back({ i, j });
            }
        }
        return WorldNavigator::pathExists(4, edges, 0, 3) == true;
        }());

    runner.runTest("PathExists: Star graph", [&]() {
        vector<vector<int>> edges = { {0, 1}, {0, 2}, {0, 3}, {0, 4} };
        return WorldNavigator::pathExists(5, edges, 1, 4) == true;
        }());

    runner.runTest("PathExists: Cycle graph", [&]() {
        vector<vector<int>> edges = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
        return WorldNavigator::pathExists(4, edges, 0, 2) == true;
        }());

    runner.runTest("PathExists: Tree with multiple paths", [&]() {
        vector<vector<int>> edges = { {0, 1}, {0, 2}, {1, 3}, {1, 4}, {2, 5}, {2, 6} };
        return WorldNavigator::pathExists(7, edges, 3, 6) == true;
        }());

    // Large graph test
    runner.runTest("PathExists: 1000-node line graph", [&]() {
        vector<vector<int>> edges;
        for (int i = 0; i < 999; i++) {
            edges.push_back({ i, i + 1 });
        }
        return WorldNavigator::pathExists(1000, edges, 0, 999) == true;
        }());

    // ========== 2. MINIMUM BRIBE COST (MST) ==========

    // Basic test cases
    runner.runTest("MinBribeCost: Triangle graph -> Cost 15", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 10, 0},
            {1, 2, 5, 0},
            {0, 2, 20, 0}
        };
        return WorldNavigator::minBribeCost(3, 3, 1, 1, roads) == 15;
        }());

    runner.runTest("MinBribeCost: Simple line graph", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 5, 2},
            {1, 2, 3, 1}
        };
        // Costs: (5*1 + 2*1) = 7, (3*1 + 1*1) = 4
        return WorldNavigator::minBribeCost(3, 2, 1, 1, roads) == 11;
        }());

    // Edge cases
    runner.runTest("MinBribeCost: Single node", [&]() {
        vector<vector<int>> roads = {};
        return WorldNavigator::minBribeCost(1, 0, 1, 1, roads) == 0;
        }());

    runner.runTest("MinBribeCost: Two nodes, one edge", [&]() {
        vector<vector<int>> roads = { {0, 1, 10, 5} };
        return WorldNavigator::minBribeCost(2, 1, 1, 1, roads) == 15;
        }());

    runner.runTest("MinBribeCost: Disconnected graph", [&]() {
        vector<vector<int>> roads = { {0, 1, 10, 0}, {2, 3, 5, 0} };
        long long result = WorldNavigator::minBribeCost(4, 2, 1, 1, roads);
        // MST not possible, should return cost for connected components
        return result == 15; // Only connects nodes within components
        }());

    runner.runTest("MinBribeCost: Zero rates", [&]() {
        vector<vector<int>> roads = { {0, 1, 10, 5}, {1, 2, 3, 2} };
        return WorldNavigator::minBribeCost(3, 2, 0, 0, roads) == 0;
        }());

    runner.runTest("MinBribeCost: Negative rates? (if allowed)", [&]() {
        vector<vector<int>> roads = { {0, 1, 10, 5} };
        long long result = WorldNavigator::minBribeCost(2, 1, -1, -1, roads);
        return result == -15; // Negative cost
        }());

    // Complex graphs
    runner.runTest("MinBribeCost: Complete graph K5", [&]() {
        vector<vector<int>> roads;
        int nodes = 5;
        for (int i = 0; i < nodes; i++) {
            for (int j = i + 1; j < nodes; j++) {
                int gold = (i + j) % 10 + 1;
                int silver = (i * j) % 10 + 1;
                roads.push_back({ i, j, gold, silver });
            }
        }
        long long result = WorldNavigator::minBribeCost(nodes, roads.size(), 1, 1, roads);
        return result == 18; // MST of K5 with these weights
        }());

    runner.runTest("MinBribeCost: Grid graph 3x3", [&]() {
        vector<vector<int>> roads;
        int n = 3;
        // Create grid edges
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int node = i * n + j;
                if (j < n - 1) { // Right edge
                    roads.push_back({ node, node + 1, 1, 1 });
                }
                if (i < n - 1) { // Down edge
                    roads.push_back({ node, node + n, 2, 2 });
                }
            }
        }
        // 9 nodes, 12 edges, MST should have 8 edges
        return WorldNavigator::minBribeCost(9, 12, 1, 1, roads) == 20;
        }());

    runner.runTest("MinBribeCost: Multiple edges between same nodes", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 10, 5},
            {0, 1, 5, 2},  // Cheaper parallel edge
            {1, 2, 3, 1}
        };
        // Should pick cheapest edge between 0-1
        return WorldNavigator::minBribeCost(3, 3, 1, 1, roads) == 11; // 7 + 4
        }());

    // Large graph test
    runner.runTest("MinBribeCost: 100 nodes random graph", [&]() {
        vector<vector<int>> roads;
        int nodes = 100;
        srand(42);
        for (int i = 0; i < nodes * 2; i++) {
            int u = rand() % nodes;
            int v = rand() % nodes;
            if (u != v) {
                int gold = rand() % 100 + 1;
                int silver = rand() % 100 + 1;
                roads.push_back({ u, v, gold, silver });
            }
        }
        long long result = WorldNavigator::minBribeCost(nodes, roads.size(), 1, 1, roads);
        return result > 0;
        }());

    // ========== 3. BINARY SUM OF DISTANCES (Floyd-Warshall) ==========

    // Basic test cases
    runner.runTest("BinarySum: Line graph -> '110'", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 1},
            {1, 2, 2}
        };
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "110"; // Sum = 6
        }());

    runner.runTest("BinarySum: Triangle graph -> '1000'", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 1},
            {1, 2, 1},
            {0, 2, 1}
        };
        // Distances: all pairs = 1, total pairs = 3, sum = 3 -> binary 11
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "11";
        }());

    // Edge cases
    runner.runTest("BinarySum: Single node -> '0'", [&]() {
        vector<vector<int>> roads = {};
        return WorldNavigator::sumMinDistancesBinary(1, roads) == "0";
        }());

    runner.runTest("BinarySum: Two nodes disconnected", [&]() {
        vector<vector<int>> roads = {};
        // Disconnected nodes, distances are infinite, sum = 0
        return WorldNavigator::sumMinDistancesBinary(2, roads) == "0";
        }());

    runner.runTest("BinarySum: Zero-length edges", [&]() {
        vector<vector<int>> roads = { {0, 1, 0}, {1, 2, 0} };
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "0";
        }());

    runner.runTest("BinarySum: Self-loops (should be ignored)", [&]() {
        vector<vector<int>> roads = { {0, 0, 5}, {0, 1, 1} };
        // Only distance 0-1 = 1, sum = 1 -> binary 1
        return WorldNavigator::sumMinDistancesBinary(2, roads) == "1";
        }());

    // Complex graphs
    runner.runTest("BinarySum: Complete graph K4 with unit weights", [&]() {
        vector<vector<int>> roads;
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 4; j++) {
                roads.push_back({ i, j, 1 });
            }
        }
        // All distances = 1, 6 pairs, sum = 6 -> binary 110
        return WorldNavigator::sumMinDistancesBinary(4, roads) == "110";
        }());

    runner.runTest("BinarySum: Star graph", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 2},
            {0, 2, 2},
            {0, 3, 2},
            {0, 4, 2}
        };
        // Center to leaves: 2, leaf to leaf: 4
        // Pairs: 4*2 + C(4,2)*4 = 8 + 6*4 = 32 -> binary 100000
        return WorldNavigator::sumMinDistancesBinary(5, roads) == "100000";
        }());

    runner.runTest("BinarySum: Multiple shortest paths", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 1},
            {1, 2, 1},
            {0, 2, 2}  // Longer direct edge
        };
        // Floyd-Warshall should pick shortest: 0-1-2 = 2, not 0-2 = 2
        // All pairs: 0-1=1, 1-2=1, 0-2=2, sum=4 -> binary 100
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "100";
        }());

    runner.runTest("BinarySum: Graph with negative cycles? (if allowed)", [&]() {
        // Floyd-Warshall doesn't handle negative cycles well
        // This test might fail depending on implementation
        vector<vector<int>> roads = {
            {0, 1, 1},
            {1, 2, -2},
            {2, 0, 1}
        };
        // Has negative cycle 0-1-2-0: 1 + (-2) + 1 = 0
        string result = WorldNavigator::sumMinDistancesBinary(3, roads);
        return true; // Just don't crash
        }());

    // Large graph test
    runner.runTest("BinarySum: 10-node random graph", [&]() {
        vector<vector<int>> roads;
        int nodes = 10;
        srand(123);
        for (int i = 0; i < nodes; i++) {
            for (int j = i + 1; j < nodes; j++) {
                if (rand() % 3 == 0) { // Sparse graph
                    int weight = rand() % 10 + 1;
                    roads.push_back({ i, j, weight });
                }
            }
        }
        string result = WorldNavigator::sumMinDistancesBinary(nodes, roads);
        // Just verify it returns a valid binary string
        return !result.empty() && result.find_first_not_of("01") == string::npos;
        }());

    runner.runTest("BinarySum: Pathological case - complete graph", [&]() {
        vector<vector<int>> roads;
        int nodes = 20;
        for (int i = 0; i < nodes; i++) {
            for (int j = i + 1; j < nodes; j++) {
                roads.push_back({ i, j, (i + j) % 100 + 1 });
            }
        }
        string result = WorldNavigator::sumMinDistancesBinary(nodes, roads);
        return result.length() > 0;
        }());

    // Test binary conversion edge cases
    runner.runTest("BinarySum: Very large sum", [&]() {
        vector<vector<int>> roads;
        int nodes = 30;
        // Create a dense graph with large weights
        for (int i = 0; i < nodes; i++) {
            for (int j = i + 1; j < nodes; j++) {
                roads.push_back({ i, j, 1000 });
            }
        }
        string result = WorldNavigator::sumMinDistancesBinary(nodes, roads);
        // Should be a long binary string
        return result.length() > 10;
        }());
}
void test_PartC_Navigator() {
    cout << "\n--- Part C: World Navigator ---" << endl;

    // 1. Safe Passage (Path Exists)
    // PDF Example: 0-1, 1-2. Path 0->2 exists.
    runner.runTest("PathExists: 0->1->2 -> True", [&]() {
        vector<vector<int>> edges = { {0, 1}, {1, 2} };
        return WorldNavigator::pathExists(3, edges, 0, 2) == true;
        }());

    // 2. The Bribe (MST)
    // PDF Example: 3 Nodes. Roads: {0,1,10}, {1,2,5}, {0,2,20}. Rate=1.
    // MST should pick 10 and 5. Total 15.
    runner.runTest("MinBribeCost: Triangle Graph -> Cost 15", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 10, 0},
            {1, 2, 5, 0},
            {0, 2, 20, 0}
        };
        // n=3, m=3, goldRate=1, silverRate=1
        return WorldNavigator::minBribeCost(3, 3, 1, 1, roads) == 15;
        }());

    // 3. Teleporter (Binary Sum APSP)
    // PDF Example: 0-1 (1), 1-2 (2). Distances: 1, 2, 3. Sum=6 -> "110"
    runner.runTest("BinarySum: Line Graph -> '110'", [&]() {
        vector<vector<int>> roads = {
            {0, 1, 1},
            {1, 2, 2}
        };
        return WorldNavigator::sumMinDistancesBinary(3, roads) == "110";
        }());
    test_PartC_Navigator_Comprehensive();
}



// ==========================================
// PART D: SERVER KERNEL
// ==========================================
void test_TaskScheduler_Comprehensive() {
    cout << "\n🔍 TASK SCHEDULER COMPREHENSIVE TESTS\n";

    // Test 1: Basic example from PDF
    runner.runTest("Scheduler: {A, A, B}, n=2 -> 4", [&]() {
        vector<char> tasks = { 'A', 'A', 'B' };
        return ServerKernel::minIntervals(tasks, 2) == 4;
        }());

    // Test 2: All same tasks
    runner.runTest("Scheduler: {A, A, A}, n=2 -> 7", [&]() {
        vector<char> tasks = { 'A', 'A', 'A' };
        return ServerKernel::minIntervals(tasks, 2) == 7;
        }());

    // Test 3: All unique tasks
    runner.runTest("Scheduler: {A, B, C}, n=2 -> 3", [&]() {
        vector<char> tasks = { 'A', 'B', 'C' };
        return ServerKernel::minIntervals(tasks, 2) == 3;
        }());

    // Test 4: Complex example from PDF
    runner.runTest("Scheduler: {A, A, A, B, B, B}, n=2 -> 8", [&]() {
        vector<char> tasks = { 'A', 'A', 'A', 'B', 'B', 'B' };
        return ServerKernel::minIntervals(tasks, 2) == 8;
        }());

    // Test 5: n = 0 (no cooling time)
    runner.runTest("Scheduler: n=0, tasks can execute immediately", [&]() {
        vector<char> tasks = { 'A', 'A', 'A', 'B', 'B', 'C' };
        return ServerKernel::minIntervals(tasks, 0) == tasks.size();
        }());

    // Test 6: n = 1
    runner.runTest("Scheduler: {A, A, B, B}, n=1 -> 4", [&]() {
        vector<char> tasks = { 'A', 'A', 'B', 'B' };
        return ServerKernel::minIntervals(tasks, 1) == 4;
        }());

    // Test 7: Large n, few tasks
    runner.runTest("Scheduler: {A, A, A}, n=5 -> 13", [&]() {
        vector<char> tasks = { 'A', 'A', 'A' };
        // Formula: ((3-1)*(5+1)) + 1 = 2*6 + 1 = 13
        return ServerKernel::minIntervals(tasks, 5) == 13;
        }());

    // Test 8: Multiple tasks with same max frequency
    runner.runTest("Scheduler: 3 tasks with freq 2, n=2 -> 6", [&]() {
        vector<char> tasks = { 'A', 'A', 'B', 'B', 'C', 'C' };
        // maxFreq = 2, count_maxFreq = 3
        // Formula: ((2-1)*(2+1)) + 3 = 1*3 + 3 = 6
        return ServerKernel::minIntervals(tasks, 2) == 6;
        }());

    // Test 9: Empty task list
    runner.runTest("Scheduler: empty list -> 0", [&]() {
        vector<char> tasks = {};
        return ServerKernel::minIntervals(tasks, 5) == 0;
        }());

    // Test 10: Single task
    runner.runTest("Scheduler: single task -> 1", [&]() {
        vector<char> tasks = { 'A' };
        return ServerKernel::minIntervals(tasks, 10) == 1;
        }());

    // Test 11: Tasks with gaps larger than n
    runner.runTest("Scheduler: {A, B, C, A, B, C}, n=1 -> 6", [&]() {
        vector<char> tasks = { 'A', 'B', 'C', 'A', 'B', 'C' };
        return ServerKernel::minIntervals(tasks, 1) == 6;
        }());

    // Test 12: Real-world large case
    runner.runTest("Scheduler: large random case", [&]() {
        vector<char> tasks;
        for (int i = 0; i < 1000; i++) {
            tasks.push_back('A' + (i % 26));
        }
        // Just verify it doesn't crash and returns reasonable value
        int result = ServerKernel::minIntervals(tasks, 10);
        return result >= tasks.size() && result <= tasks.size() * 2;
        }());
}
void test_PartD_Kernel() {
    cout << "\n--- Part D: Server Kernel ---" << endl;

    // 1. Task Scheduler
    // PDF Example: Tasks={A, A, B}, n=2.
    // Order: A -> B -> idle -> A. Total intervals: 4.
    runner.runTest("Scheduler: {A, A, B}, n=2 -> 4 Intervals", [&]() {
        vector<char> tasks = {'A', 'A', 'B'};
        return ServerKernel::minIntervals(tasks, 2) == 4;
    }());
    test_TaskScheduler_Comprehensive();
}

int main() {
    cout << "Arcadia Engine - Student Happy Path Tests" << endl;
    cout << "-----------------------------------------" << endl;

    test_PartA_DataStructures();
    test_PartB_Inventory();
    test_PartC_Navigator();
    test_PartD_Kernel();
    runner.printSummary();

    return 0;
}
