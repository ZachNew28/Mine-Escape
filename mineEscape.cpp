// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F
#include "mineEscape.h"

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iomanip>
#include <iostream>
#include <queue>
#include <vector>

#include <getopt.h>

#include "P2random.h"


int main(int argc, char* argv[]) {
    // Speed up io
    ios_base::sync_with_stdio(false);

    // Set precision for median
    cout << std::fixed << std::setprecision(2);

    MineBoard game;
    game.getOptions(argc, argv);
    game.mine();
    game.output();
}

// Prints a help message if requested that explains all the options and what the program does
// argv[0] is the name of the program
void MineBoard::printHelp(char* argv[]) {
    cout << "Usage: " << argv[0] << " -h | -m | -v | [-s <word>] |\n";
    cout << "This program is used to convert a \"beginning\" word into an \"end\" word\n";
    cout << "while only allowing specific single character conversions.\n\n";
    cout << "-h: Prints a help message the explains the program and options\n";
    cout << "-q: Directs the program to use a search container that behaves like a queue\n";
    cout << "-s: Directs the program to use a search container that behaves like a stack\n";
    cout << "-b <word>: Specifies the beginning word for the search\n";
    cout << "-e <word>: Specifies the end word for the search\n";
    cout << "-o (W|M): Specifies the output format, Word or Modification\n";
    cout << "-c: Allows Letterman to change one letter into another during word morphs\n";
    cout << "-l: Allows Letterman to insert or delete a single letter during word morphs\n";
    cout << "-p: Allows Letterman to swap any two adjacent letters during word morphs" << endl;
}

void MineBoard::getOptions(int argc, char* argv[]) {
    opterr = false;
    int choice;
    int index = 0;

    // List of the options
    option long_options[] = {
        {   "help",       no_argument, nullptr,  'h'},
        {  "stats", required_argument, nullptr,  's'},
        { "median",       no_argument, nullptr,  'm'},
        {"verbose",       no_argument, nullptr,  'v'},
        {  nullptr,                 0, nullptr, '\0'},
    };

    // Read input file
    readInput();

    // Actually get the desired option now and do something with it
    while ((choice = getopt_long(argc, argv, "hmvs:", long_options, &index)) != -1) {
        if (choice != 'h' && choice != 'm' && choice != 'v' && choice != 's') {
            cerr << "Unknown command line option" << endl;
            exit(1);
        }

        switch (choice) {
        case 'h':
            printHelp(argv);
            exit(0);

        case 'm':
            medianMode = true;
            break;

        case 'v':
            verboseMode = true;
            break;

        case 's': {
            int arg { stoi(optarg) };
            statsMode = true;
            statsPrintNum = static_cast<size_t>(arg);
            break;
        }
        }
    }

    // Do any optimizations if necessary

    // Error messages
}

vector<vector<Tile>> MineBoard::readInput() {
    char inputType;
    string junk;
    int rubbleValue;
    stringstream ss;

    cin >> inputType;
    cin >> junk;   // Reads in 'Size: ' from the second line
    cin >> size;
    cin >> junk;   // Reads in 'Start: ' from the third line
    cin >> currRow;
    cin >> currCol;

    // Check that row and column are valid
    if (currRow > size) {
        cerr << "Invalid starting row";
        exit(1);
    }
    if (currCol > size) {
        cerr << "Invalid starting column";
        exit(1);
    }

    // Resize the map
    map2D.resize(static_cast<size_t>(size), vector<Tile>(static_cast<size_t>(size)));

    // Pseudorandom input mode
    if (inputType == 'R') {
        uint32_t seed;
        uint32_t maxRubble;
        uint32_t numTNT;

        cin >> junk;        // Reads in 'Seed: ' from the fourth line
        cin >> seed;        // Must be non-negative
        cin >> junk;        // Reads in 'Max_Rubble: ' from the fifth line
        cin >> maxRubble;   // Must be non-negative
        cin >> junk;        // Reads in 'TNT: ' from the sixth line
        cin >> numTNT;      // Must be non-negative

        P2random::PR_init(ss, static_cast<uint32_t>(size), seed, maxRubble, numTNT);
    } else if (inputType == 'M') {
        // Do nothing
    }
    // Invalid input mode
    else {
        cerr << "Invalid input mode";
        exit(1);
    }

    // Sets the stringstream 'ss' to read just like cin would for map
    istream& inputStream = (inputType == 'M') ? cin : ss;

    // Loop through the grid and read in the rubble value
    for (size_t row = 0; row < size; ++row) {
        for (size_t column = 0; column < size; ++column) {
            inputStream >> rubbleValue;
            map2D[row][column].rubble = rubbleValue;
            map2D[row][column].rowNum = row;
            map2D[row][column].colNum = column;
            if (rubbleValue == -1) {
                map2D[row][column].isTNT = true;
            }
        }
    }

    return map2D;
}

void MineBoard::output() {
    // Summary message
    cout << "Cleared " << tilesCleared << " tiles containing " << rubbleCleared << " rubble and escaped." << endl;

    if (statsMode) {
        size_t vectorSize = statsTiles.size();
        cout << "First tiles cleared:" << endl;

        // If N is greater than size of vector then only loop through size
        if (statsPrintNum > vectorSize) {
            bool loopStop = true;
            // Print first tiles
            for (size_t i = 0; i < vectorSize; ++i) {
                // TNT
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                }
                // Normal tile
                else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
            }

            cout << "Last tiles cleared:" << endl;

            // Print last tiles
            if (vectorSize > 0) {
                for (size_t i = vectorSize - 1; loopStop; --i) {
                    if (statsTiles[i].isTNT) {
                        cout << "TNT";
                    }
                    // Normal tile
                    else {
                        cout << statsTiles[i].rubble;
                    }
                    cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
                    if (i == 0) {
                        loopStop = false;
                    }
                }
            }

            cout << "Easiest tiles cleared:" << endl;

            // Sort and print in order of easiest tiles
            if (vectorSize > 0) {
                std::sort(statsTiles.begin(), statsTiles.end(), EasyCompare);
            }


            for (size_t i = 0; i < vectorSize; ++i) {
                // TNT
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                }
                // Normal tile
                else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
            }

            cout << "Hardest tiles cleared:" << endl;

            // Print in order of hardest tiles
            if (vectorSize > 0) {
                loopStop = true;
                for (size_t i = vectorSize - 1; loopStop; --i) {
                    if (statsTiles[i].isTNT) {
                        cout << "TNT";
                    }
                    // Normal tile
                    else {
                        cout << statsTiles[i].rubble;
                    }
                    cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
                    if (i == 0) {
                        loopStop = false;
                    }
                }
            }
        } else {
            // Print first tiles
            for (size_t i = 0; i < statsPrintNum; ++i) {
                // TNT
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                }
                // Normal tile
                else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
            }

            cout << "Last tiles cleared:" << endl;

            // Print last tiles
            size_t start = vectorSize - 1;   // Starting index
            //???
            size_t end
              = vectorSize >= statsPrintNum ? vectorSize - statsPrintNum : 0;   // Calculate end to avoid underflow

            for (size_t i = start;; --i) {
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                } else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;

                if (i == 0 || i == end) {   // Check if we've reached the beginning or the end of the desired range
                    break;
                }
            }

            cout << "Easiest tiles cleared:" << endl;

            // Sort and print in order of easiest tiles
            std::sort(statsTiles.begin(), statsTiles.end(), EasyCompare);

            for (size_t i = 0; i < statsPrintNum; ++i) {
                // TNT
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                }
                // Normal tile
                else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;
            }

            cout << "Hardest tiles cleared:" << endl;

            // Print in order of hardest tiles
            for (size_t i = start;; --i) {
                // TNT
                if (statsTiles[i].isTNT) {
                    cout << "TNT";
                }
                // Normal tile
                else {
                    cout << statsTiles[i].rubble;
                }
                cout << " at [" << statsTiles[i].rowNum << "," << statsTiles[i].colNum << "]" << endl;

                if (i == 0 || i == end) {   // Check if we've reached the beginning or the end of the desired range
                    break;
                }
            }
        }
    }
}

void MineBoard::mine() {
    priority_queue<Tile*, vector<Tile*>, TileCompare> primaryPQ;
    priority_queue<Tile*, vector<Tile*>, TileCompare> tntPQ;
    // Vector of tile pointers so that I can shove them into the PQ afterward
    vector<Tile*> detonatedTiles;
    Tile junkTile;
    junkTile.rubble = -1;
    junkTile.rowNum = 0;
    junkTile.colNum = 0;

    // Add the starting tile to the queue
    map2D[currRow][currCol].isDiscovered = true;
    primaryPQ.push(&map2D[currRow][currCol]);
    if (map2D[currRow][currCol].rubble > 0) {
        if (verboseMode) {
            cout << "Cleared: " << map2D[currRow][currCol].rubble << " at [" << currRow << "," << currCol << "]"
                 << endl;
        }
        rubbleValues.push_back(map2D[currRow][currCol].rubble);
        rubbleCleared += map2D[currRow][currCol].rubble;
        if (statsMode) {
            statsTiles.push_back(map2D[currRow][currCol]);
        }
        map2D[currRow][currCol].rubble = 0;
        tilesCleared++;
        if (medianMode) {
            cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
        }
        debugLineNum++;
    }
    // Starting tile is TNT
    else if (map2D[currRow][currCol].rubble == -1) {
        // Loop until the next tile is not TNT
        while (map2D[currRow][currCol].rubble == -1) {
            detonate(tntPQ, detonatedTiles);
        }

        // Clear out the tnt PQ for when it needs to be used again and set all those tiles to zero rubble
        while (!tntPQ.empty()) {
            // Only clear it if rubble isn't zero
            if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble != 0) {
                if (verboseMode) {
                    cout << "Cleared by TNT: " << map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble << " at ["
                         << tntPQ.top()->rowNum << "," << tntPQ.top()->colNum << "]" << endl;
                }
                rubbleCleared += map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble;
                rubbleValues.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble);
                if (statsMode) {
                    statsTiles.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum]);
                }
                map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble = 0;
                if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool) {
                    sortQueue(primaryPQ, tntPQ.top()->rowNum, tntPQ.top()->colNum);
                    map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool = false;
                }
                tilesCleared++;
                if (medianMode) {
                    cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
                }
                debugLineNum++;
            }

            tntPQ.pop();
        }

        // Add all the detonated tiles to the primaryPQ
        primaryPQ.push(&junkTile);
        for (size_t i = 0; i < detonatedTiles.size(); ++i) {
            primaryPQ.push(detonatedTiles[i]);
        }
    }

    // See where the miner can go, loop will end once the miner
    while (currRow != 0 && currRow != size - 1 && currCol != 0 && currCol != size - 1) {
        // Tile is going to be investigated (cleared) so remove it from PQ
        primaryPQ.pop();

        // Add any undiscovered tiles to the primary queue
        if (!map2D[currRow - 1][currCol].isDiscovered) {   // Up
            primaryPQ.push(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow + 1][currCol].isDiscovered) {   // Down
            primaryPQ.push(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow][currCol - 1].isDiscovered) {   // Left
            primaryPQ.push(&map2D[currRow][currCol - 1]);
            map2D[currRow][currCol - 1].isDiscovered = true;
        }
        if (!map2D[currRow][currCol + 1].isDiscovered) {   // Right
            primaryPQ.push(&map2D[currRow][currCol + 1]);
            map2D[currRow][currCol + 1].isDiscovered = true;
        }

        // Set the new tile to be whatever is at the top of the queue
        currRow = primaryPQ.top()->rowNum;
        currCol = primaryPQ.top()->colNum;

        // If it is the final tile, break loop and deal with it differently
        if (currRow == 0 || currRow == size - 1 || currCol == 0 || currCol == size - 1) {
            break;
        }

        if (map2D[currRow][currCol].rubble == -1) {
            // Clear the vector of previously detonated tiles
            detonatedTiles.clear();

            // Loop until the next tile is not TNT
            while (map2D[currRow][currCol].rubble == -1) {
                detonate(tntPQ, detonatedTiles);
            }

            // Clear out the tnt PQ for when it needs to be used again and set all those tiles to zero rubble
            while (!tntPQ.empty()) {
                // Only clear it if rubble isn't zero
                if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble != 0) {
                    if (verboseMode) {
                        cout << "Cleared by TNT: " << map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble << " at ["
                             << tntPQ.top()->rowNum << "," << tntPQ.top()->colNum << "]" << endl;
                    }
                    rubbleCleared += map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble;
                    rubbleValues.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble);
                    if (statsMode) {
                        statsTiles.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum]);
                    }
                    map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble = 0;
                    // Reinsert it into PQ
                    if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool) {
                        sortQueue(primaryPQ, tntPQ.top()->rowNum, tntPQ.top()->colNum);
                        map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool = false;
                    }
                    tilesCleared++;
                    if (medianMode) {
                        cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
                    }
                    debugLineNum++;
                }

                tntPQ.pop();
            }
            primaryPQ.push(&junkTile);
            for (size_t i = 0; i < detonatedTiles.size(); ++i) {
                primaryPQ.push(detonatedTiles[i]);
            }
        }
        // Just clear the tile normally if it is not TNT
        else {
            // Only clear it if rubble isn't zero
            if (map2D[currRow][currCol].rubble > 0) {
                if (verboseMode) {
                    cout << "Cleared: " << map2D[currRow][currCol].rubble << " at [" << currRow << "," << currCol << "]"
                         << endl;
                }
                rubbleCleared += map2D[currRow][currCol].rubble;
                rubbleValues.push_back(map2D[currRow][currCol].rubble);
                if (statsMode) {
                    statsTiles.push_back(map2D[currRow][currCol]);
                }
                map2D[currRow][currCol].rubble = 0;
                tilesCleared++;
                if (medianMode) {
                    cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
                }
                debugLineNum++;
            }
        }
    }

    // Miner has escaped, maybe output goes here but could also go in main
    if (map2D[currRow][currCol].rubble == -1) {
        // The final tile is tnt
        detonate(tntPQ, detonatedTiles);
        while (!tntPQ.empty()) {
            // Only clear it if rubble isn't zero
            if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble != 0) {
                if (verboseMode) {
                    cout << "Cleared by TNT: " << map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble << " at ["
                         << tntPQ.top()->rowNum << "," << tntPQ.top()->colNum << "]" << endl;
                }
                rubbleCleared += map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble;
                rubbleValues.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble);
                if (statsMode) {
                    statsTiles.push_back(map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum]);
                }
                map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].rubble = 0;
                // Reinsert it into PQ
                if (map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool) {
                    sortQueue(primaryPQ, tntPQ.top()->rowNum, tntPQ.top()->colNum);
                    map2D[tntPQ.top()->rowNum][tntPQ.top()->colNum].sortBool = false;
                }
                tilesCleared++;
                if (medianMode) {
                    cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
                }
                debugLineNum++;
            }

            tntPQ.pop();
        }
    }
    // Tile does not need to be cleared
    else if (map2D[currRow][currCol].rubble == 0) {
        // Miner has simply escaped
    }
    // Tile has rubble
    else {
        // Clear tile
        if (verboseMode) {
            cout << "Cleared: " << map2D[currRow][currCol].rubble << " at [" << currRow << "," << currCol << "]"
                 << endl;
        }
        rubbleCleared += map2D[currRow][currCol].rubble;
        rubbleValues.push_back(map2D[currRow][currCol].rubble);
        if (statsMode) {
            statsTiles.push_back(map2D[currRow][currCol]);
        }
        map2D[currRow][currCol].rubble = 0;
        tilesCleared++;
        if (medianMode) {
            cout << "Median difficulty of clearing rubble is: " << getMedian() << endl;
        }
        debugLineNum++;
    }
}

void MineBoard::detonate(priority_queue<Tile*, vector<Tile*>, TileCompare>& tntPQ, vector<Tile*>& detonatedTiles) {
    // Add all adjacent tiles to the TNT priority queue if they haven't been added already
    // Tile is on top row
    if (currRow == 0) {
        if (!map2D[currRow][currCol].isDetonated) {
            map2D[currRow][currCol].isDetonated = true;
        }
        if (!map2D[currRow + 1][currCol].isDetonated) {
            if (map2D[currRow + 1][currCol].isDiscovered) {
                map2D[currRow + 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDetonated = true;
        }
        // If not the top left corner, add the tile to the left
        if (currCol != 0) {
            if (!map2D[currRow][currCol - 1].isDetonated) {
                if (map2D[currRow][currCol - 1].isDiscovered) {
                    map2D[currRow][currCol - 1].sortBool = true;
                }
                tntPQ.push(&map2D[currRow][currCol - 1]);
                map2D[currRow][currCol - 1].isDetonated = true;
            }
        }
        // If not the top right corner, add the tile to the right
        if (currCol != size - 1) {
            if (!map2D[currRow][currCol + 1].isDetonated) {
                if (map2D[currRow][currCol + 1].isDiscovered) {
                    map2D[currRow][currCol + 1].sortBool = true;
                }
                tntPQ.push(&map2D[currRow][currCol + 1]);
                map2D[currRow][currCol + 1].isDetonated = true;
            }
        }

        // Add any undiscovered tiles to the main PQ
        if (!map2D[currRow + 1][currCol].isDiscovered) {   // Down
            detonatedTiles.push_back(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDiscovered = true;
        }
        // Check for top left corner
        if (currCol != 0) {
            if (!map2D[currRow][currCol - 1].isDiscovered) {
                detonatedTiles.push_back(&map2D[currRow][currCol - 1]);
                map2D[currRow][currCol - 1].isDiscovered = true;
            }
        }
        // Check for top right corner
        if (currCol != size - 1) {
            if (!map2D[currRow][currCol + 1].isDiscovered) {   // Right
                detonatedTiles.push_back(&map2D[currRow][currCol + 1]);
                map2D[currRow][currCol + 1].isDiscovered = true;
            }
        }
    }
    // Tile is on bottom row
    else if (currRow == size - 1) {
        if (!map2D[currRow][currCol].isDetonated) {
            map2D[currRow][currCol].isDetonated = true;
        }
        if (!map2D[currRow - 1][currCol].isDetonated) {
            if (map2D[currRow - 1][currCol].isDiscovered) {
                map2D[currRow - 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDetonated = true;
        }
        // Check bottom left corner
        if (currCol != 0) {
            if (!map2D[currRow][currCol - 1].isDetonated) {
                if (map2D[currRow][currCol - 1].isDiscovered) {
                    map2D[currRow][currCol - 1].sortBool = true;
                }
                tntPQ.push(&map2D[currRow][currCol - 1]);
                map2D[currRow][currCol - 1].isDetonated = true;
            }
        }
        // Check bottom right corner
        if (currCol != size - 1) {
            if (!map2D[currRow][currCol + 1].isDetonated) {
                if (map2D[currRow][currCol + 1].isDiscovered) {
                    map2D[currRow][currCol + 1].sortBool = true;
                }
                tntPQ.push(&map2D[currRow][currCol + 1]);
                map2D[currRow][currCol + 1].isDetonated = true;
            }
        }

        // Add any undiscovered tiles to the main PQ
        if (!map2D[currRow - 1][currCol].isDiscovered) {   // Up
            detonatedTiles.push_back(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDiscovered = true;
        }
        // Check for bottom left corner
        if (currCol != 0) {
            if (!map2D[currRow][currCol - 1].isDiscovered) {   // Left
                detonatedTiles.push_back(&map2D[currRow][currCol - 1]);
                map2D[currRow][currCol - 1].isDiscovered = true;
            }
        }
        // Check for bottom right corner
        if (currCol != size - 1) {
            if (!map2D[currRow][currCol + 1].isDiscovered) {   // Right
                detonatedTiles.push_back(&map2D[currRow][currCol + 1]);
                map2D[currRow][currCol + 1].isDiscovered = true;
            }
        }
    }
    // Tile is on leftmost column and not a corner
    if (currCol == 0 && currRow != 0 && currRow != size - 1) {
        if (!map2D[currRow][currCol].isDetonated) {
            map2D[currRow][currCol].isDetonated = true;
        }
        if (!map2D[currRow - 1][currCol].isDetonated) {
            if (map2D[currRow - 1][currCol].isDiscovered) {
                map2D[currRow - 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow + 1][currCol].isDetonated) {
            if (map2D[currRow + 1][currCol].isDiscovered) {
                map2D[currRow + 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow][currCol + 1].isDetonated) {
            if (map2D[currRow][currCol + 1].isDiscovered) {
                map2D[currRow][currCol + 1].sortBool = true;
            }
            tntPQ.push(&map2D[currRow][currCol + 1]);
            map2D[currRow][currCol + 1].isDetonated = true;
        }

        // Add any undiscovered tiles to the main PQ
        if (!map2D[currRow - 1][currCol].isDiscovered) {   // Up
            detonatedTiles.push_back(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow + 1][currCol].isDiscovered) {   // Down
            detonatedTiles.push_back(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow][currCol + 1].isDiscovered) {   // Right
            detonatedTiles.push_back(&map2D[currRow][currCol + 1]);
            map2D[currRow][currCol + 1].isDiscovered = true;
        }
    }
    // Tile is in rightmost column and not a corner
    else if (currCol == size - 1 && currRow != 0 && currRow != size - 1) {
        if (!map2D[currRow][currCol].isDetonated) {
            map2D[currRow][currCol].isDetonated = true;
        }
        if (!map2D[currRow - 1][currCol].isDetonated) {
            if (map2D[currRow - 1][currCol].isDiscovered) {
                map2D[currRow - 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow + 1][currCol].isDetonated) {
            if (map2D[currRow + 1][currCol].isDiscovered) {
                map2D[currRow + 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow][currCol - 1].isDetonated) {
            if (map2D[currRow][currCol - 1].isDiscovered) {
                map2D[currRow][currCol - 1].sortBool = true;
            }
            tntPQ.push(&map2D[currRow][currCol - 1]);
            map2D[currRow][currCol - 1].isDetonated = true;
        }

        // Add any undiscovered tiles to the main PQ
        if (!map2D[currRow - 1][currCol].isDiscovered) {   // Up
            detonatedTiles.push_back(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow + 1][currCol].isDiscovered) {   // Down
            detonatedTiles.push_back(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow][currCol - 1].isDiscovered) {   // Left
            detonatedTiles.push_back(&map2D[currRow][currCol - 1]);
            map2D[currRow][currCol - 1].isDiscovered = true;
        }
    }

    // Tile is not on the edge
    if (currRow != 0 && currRow != size - 1 && currCol != 0 && currCol != size - 1) {
        if (!map2D[currRow][currCol].isDetonated) {
            map2D[currRow][currCol].isDetonated = true;
        }
        if (!map2D[currRow - 1][currCol].isDetonated) {
            if (map2D[currRow - 1][currCol].isDiscovered) {
                map2D[currRow - 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow + 1][currCol].isDetonated) {
            if (map2D[currRow + 1][currCol].isDiscovered) {
                map2D[currRow + 1][currCol].sortBool = true;
            }
            tntPQ.push(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDetonated = true;
        }
        if (!map2D[currRow][currCol - 1].isDetonated) {
            if (map2D[currRow][currCol - 1].isDiscovered) {
                map2D[currRow][currCol - 1].sortBool = true;
            }
            tntPQ.push(&map2D[currRow][currCol - 1]);
            map2D[currRow][currCol - 1].isDetonated = true;
        }
        if (!map2D[currRow][currCol + 1].isDetonated) {
            if (map2D[currRow][currCol + 1].isDiscovered) {
                map2D[currRow][currCol + 1].sortBool = true;
            }
            tntPQ.push(&map2D[currRow][currCol + 1]);
            map2D[currRow][currCol + 1].isDetonated = true;
        }

        // Add any undiscovered tiles to the main PQ
        if (!map2D[currRow - 1][currCol].isDiscovered) {   // Up
            detonatedTiles.push_back(&map2D[currRow - 1][currCol]);
            map2D[currRow - 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow + 1][currCol].isDiscovered) {   // Down
            detonatedTiles.push_back(&map2D[currRow + 1][currCol]);
            map2D[currRow + 1][currCol].isDiscovered = true;
        }
        if (!map2D[currRow][currCol - 1].isDiscovered) {   // Left
            detonatedTiles.push_back(&map2D[currRow][currCol - 1]);
            map2D[currRow][currCol - 1].isDiscovered = true;
        }
        if (!map2D[currRow][currCol + 1].isDiscovered) {   // Right
            detonatedTiles.push_back(&map2D[currRow][currCol + 1]);
            map2D[currRow][currCol + 1].isDiscovered = true;
        }
    }

    // Set current tnt tile to zero rubble because it has officially exploded
    if (verboseMode) {
        cout << "TNT explosion at [" << currRow << "," << currCol << "]!" << endl;
        debugLineNum++;
    }
    statsTiles.push_back(map2D[currRow][currCol]);
    map2D[currRow][currCol].rubble = 0;

    if (tntPQ.empty()) {
        return;
    }
    // If the next tile to blow up is tnt, call the function again
    if (tntPQ.top()->rubble == -1) {
        // Change the current tile to whichever has highest priority tnt
        currRow = tntPQ.top()->rowNum;
        currCol = tntPQ.top()->colNum;

        // Take the current tile out of the tnt PQ
        tntPQ.pop();

        detonate(tntPQ, detonatedTiles);
    }
    // All the tnt that could detonate did so just return
    else {
        return;
    }
}

double MineBoard::getMedian() {
    double median;
    std::sort(rubbleValues.begin(), rubbleValues.end());

    // Even num of tiles cleared
    if (tilesCleared % 2 == 0) {
        median = (rubbleValues[static_cast<size_t>(tilesCleared / 2)]
                  + rubbleValues[static_cast<size_t>((tilesCleared / 2) - 1)])
               / 2.0;
    }
    // Odd num of tiles cleared
    else {
        median = rubbleValues[static_cast<size_t>(tilesCleared / 2)];
    }

    return median;
}

void MineBoard::sortQueue(priority_queue<Tile*, vector<Tile*>, TileCompare>& primaryPQ, size_t tileRowNum,
                          size_t tileColNum) {
    std::vector<Tile*> temp;

    while (!primaryPQ.empty()) {
        if (primaryPQ.top()->rowNum == tileRowNum && primaryPQ.top()->colNum == tileColNum) {
            temp.push_back(primaryPQ.top());
            primaryPQ.pop();
            break;
        }
        temp.push_back(primaryPQ.top());
        primaryPQ.pop();
    }

    // Re-insert elements into the priority queue
    for (Tile* tile : temp) {
        primaryPQ.push(tile);
    }
}
