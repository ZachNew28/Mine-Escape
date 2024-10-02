// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F
#include <cstddef>
#include <queue>
#include <string>

#include "getopt.h"
using namespace std;


struct Tile {
    size_t rowNum;
    size_t colNum;
    int rubble;
    bool isDiscovered = false;
    bool isDetonated = false;
    bool isTNT = false;
    bool sortBool = false;
};

struct TileCompare {
    // Should sort with smallest on top
    bool operator()(const Tile* a, const Tile* b) const {
        if (a->rubble != b->rubble) {
            return a->rubble > b->rubble;   // Smaller rubble values come first
        }
        if (a->colNum != b->colNum) {
            return a->colNum > b->colNum;   // If rubble is equal, smaller colNum comes first
        }
        return a->rowNum > b->rowNum;   // If both rubble and colNum are equal, smaller rowNum comes first
    }
};

struct StatsEasyCompare {
    // Should sort from least rubble to most rubble
    bool operator()(Tile const& a, Tile const& b) {
        if (a.rubble != b.rubble) {
            return a.rubble < b.rubble;   // Smaller rubble values come first
        }
        if (a.colNum != b.colNum) {
            return a.colNum < b.colNum;   // If rubble is equal, smaller colNum comes first
        }
        return a.rowNum < b.rowNum;
    }
} EasyCompare;

class MineBoard {
private:
    vector<vector<Tile>> map2D;
    vector<Tile> statsTiles;
    vector<int> rubbleValues;
    size_t currRow = 0;
    size_t currCol = 0;
    size_t size = 0;
    size_t statsPrintNum = 0;
    int debugLineNum = 1;
    int tilesCleared = 0;
    int rubbleCleared = 0;
    bool verboseMode = false;
    bool medianMode = false;
    bool statsMode = false;

public:
    MineBoard() = default;
    void printHelp(char* argv[]);
    void getOptions(int argc, char* argv[]);
    vector<vector<Tile>> readInput();
    void output();
    void mine();
    void detonate(priority_queue<Tile*, vector<Tile*>, TileCompare>& tntPQ, vector<Tile*>& detonatedTiles);
    double getMedian();
    void sortQueue(priority_queue<Tile*, vector<Tile*>, TileCompare>& primaryPQ, size_t tileRowNum, size_t tileColNum);
};
