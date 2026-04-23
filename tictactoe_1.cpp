/*
 =====================================================================
   TIC TAC TOE GAME - C++ Backend
   Subject  : Object Oriented Design & Programming (OODP)
   Units covered:
     Unit 1 - Classes, Objects, Constructors, Destructors
     Unit 2 - Inheritance, Polymorphism
     Unit 3 - Operator Overloading, Friend Functions
     Unit 4 - Templates, Exception Handling, File I/O
     Unit 5 - STL: vector, map, array, iterators, algorithms
 =====================================================================
*/

#include <iostream>
#include <vector>       // Unit 5 - STL Sequence Container
#include <array>        // Unit 5 - STL Array
#include <map>          // Unit 5 - STL Associative Container
#include <algorithm>    // Unit 5 - STL Algorithms: find, count, for_each
#include <stdexcept>    // Unit 4 - Exception Handling
#include <fstream>      // Unit 4 - File I/O
#include <string>

using namespace std;


// =====================================================================
// UNIT 5 - STL: Type alias using array for the board (3x3 = 9 cells)
// =====================================================================
using Board = array<char, 9>;


// =====================================================================
// UNIT 1 - Class: Player
//          Encapsulates player name and symbol (X or O)
// =====================================================================
class Player {
protected:
    string name;
    char symbol;
    int score;

public:
    // Unit 1 - Constructor
    Player(string playerName, char playerSymbol)
        : name(playerName), symbol(playerSymbol), score(0) {}

    // Unit 1 - Destructor
    virtual ~Player() {}

    // Unit 1 - Member functions (getters)
    string getName()  const { return name; }
    char   getSymbol() const { return symbol; }
    int    getScore()  const { return score; }

    // Unit 1 - Member function
    void incrementScore() { score++; }

    // Unit 2 - Virtual function for polymorphism (overridden in HumanPlayer)
    virtual void displayInfo() const {
        cout << "Player: " << name << "  Symbol: " << symbol
             << "  Score: " << score << endl;
    }
};


// =====================================================================
// UNIT 2 - Inheritance: HumanPlayer inherits from Player
// =====================================================================
class HumanPlayer : public Player {
public:
    // Calling base class constructor using initializer list
    HumanPlayer(string playerName, char playerSymbol)
        : Player(playerName, playerSymbol) {}

    // Unit 2 - Method Overriding (Runtime Polymorphism)
    void displayInfo() const override {
        cout << "[Human] " << name << " (" << symbol << ") - Score: " << score << endl;
    }
};


// =====================================================================
// UNIT 3 - Operator Overloading inside the Board class wrapper
//          Friend Function to display the board
// =====================================================================
class GameBoard {
private:
    Board cells;    // Unit 5 - STL array<char,9>

public:
    // Unit 1 - Constructor: initializes all cells to empty
    GameBoard() {
        cells.fill('-');
    }

    // Unit 1 - Member function: place a symbol on the board
    bool placeSymbol(int position, char symbol) {
        // Unit 4 - Exception Handling
        if (position < 0 || position >= 9) {
            throw out_of_range("Position must be between 0 and 8.");
        }
        if (cells[position] != '-') {
            throw invalid_argument("Cell already occupied!");
        }
        cells[position] = symbol;
        return true;
    }

    // Unit 1 - Member function: reset the board
    void reset() {
        cells.fill('-');
    }

    // Unit 1 - Getter for a cell
    char getCell(int index) const {
        return cells[index];
    }

    // Unit 3 - Operator Overloading: == to compare two boards
    bool operator==(const GameBoard& other) const {
        return cells == other.cells;
    }

    // Unit 3 - Friend Function: overload << to print the board
    friend ostream& operator<<(ostream& os, const GameBoard& gb);

    // Getter for internal cells (used in win check)
    const Board& getCells() const { return cells; }
};

// Unit 3 - Friend function definition (outside the class)
ostream& operator<<(ostream& os, const GameBoard& gb) {
    os << "\n";
    for (int i = 0; i < 9; i++) {
        os << " " << gb.cells[i];
        if ((i + 1) % 3 == 0)
            os << "\n";
        else
            os << " |";
    }
    os << "\n";
    return os;
}


// =====================================================================
// UNIT 4 - Template Function
//          A generic function to display any container's contents
// =====================================================================
template <typename T>
void displayContainer(const T& container, const string& label) {
    cout << label << ": ";
    // Unit 5 - Iterator usage
    for (auto it = container.begin(); it != container.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}


// =====================================================================
// UNIT 1 - Class: Game (main controller)
//          Uses STL containers, algorithms, file I/O, exceptions
// =====================================================================
class Game {
private:
    GameBoard board;
    HumanPlayer* player1;    // Unit 2 - Pointer to base type (Polymorphism)
    HumanPlayer* player2;
    HumanPlayer* currentPlayer;
    int moveCount;
    int draws;

    // Unit 5 - STL: map to store win counts by player symbol
    map<char, int> winRecord;

    // Unit 5 - STL: vector to store move history as strings
    vector<string> moveHistory;

    // Unit 5 - STL: array of winning combinations (index triplets)
    array<array<int,3>, 8> winCombos = {{
        {0,1,2}, {3,4,5}, {6,7,8},   // rows
        {0,3,6}, {1,4,7}, {2,5,8},   // columns
        {0,4,8}, {2,4,6}             // diagonals
    }};

public:
    // Unit 1 - Constructor
    Game(string name1, string name2)
        : moveCount(0), draws(0) {
        player1      = new HumanPlayer(name1, 'X');
        player2      = new HumanPlayer(name2, 'O');
        currentPlayer = player1;

        // Unit 5 - map initialization
        winRecord['X'] = 0;
        winRecord['O'] = 0;
    }

    // Unit 1 - Destructor: release dynamically allocated memory
    ~Game() {
        delete player1;
        delete player2;
    }

    // ---------------------------------------------------------------
    // Switch turn between players
    // ---------------------------------------------------------------
    void switchPlayer() {
        currentPlayer = (currentPlayer == player1) ? player2 : player1;
    }

    // ---------------------------------------------------------------
    // Unit 5 - Algorithm: check win using find() style iteration
    // ---------------------------------------------------------------
    bool checkWin(char symbol) {
        const Board& cells = board.getCells();

        // Iterate over all 8 winning combinations
        for (auto& combo : winCombos) {
            if (cells[combo[0]] == symbol &&
                cells[combo[1]] == symbol &&
                cells[combo[2]] == symbol) {
                return true;
            }
        }
        return false;
    }

    // ---------------------------------------------------------------
    // Unit 5 - Algorithm: count() to check if board is full (draw)
    // ---------------------------------------------------------------
    bool checkDraw() {
        const Board& cells = board.getCells();
        // count how many cells are still empty
        int emptyCells = count(cells.begin(), cells.end(), '-');
        return emptyCells == 0;
    }

    // ---------------------------------------------------------------
    // Unit 4 - File I/O: save game result to a file
    // ---------------------------------------------------------------
    void saveResult(const string& result) {
        ofstream outFile("game_log.txt", ios::app);  // append mode
        if (!outFile) {
            throw runtime_error("Unable to open game_log.txt for writing.");
        }
        outFile << result << endl;
        outFile.close();
        cout << "Result saved to game_log.txt" << endl;
    }

    // ---------------------------------------------------------------
    // Unit 4 - File I/O: load and display previous game logs
    // ---------------------------------------------------------------
    void loadHistory() {
        ifstream inFile("game_log.txt");
        if (!inFile) {
            cout << "No previous game history found.\n";
            return;
        }
        cout << "\n--- Game History ---\n";
        string line;
        while (getline(inFile, line)) {
            cout << line << "\n";
        }
        inFile.close();
    }

    // ---------------------------------------------------------------
    // Unit 5 - for_each() algorithm to display move history
    // ---------------------------------------------------------------
    void displayMoveHistory() {
        cout << "\n--- Move History ---\n";
        for_each(moveHistory.begin(), moveHistory.end(), [](const string& move) {
            cout << move << "\n";
        });
    }

    // ---------------------------------------------------------------
    // Unit 5 - STL map: display scoreboard
    // ---------------------------------------------------------------
    void displayScoreboard() {
        cout << "\n====== SCOREBOARD ======\n";
        // Unit 2 - Runtime Polymorphism via virtual displayInfo()
        player1->displayInfo();
        player2->displayInfo();
        cout << "Draws : " << draws << "\n";
        cout << "========================\n";
    }

    // ---------------------------------------------------------------
    // Main play function: one full round
    // ---------------------------------------------------------------
    void playRound() {
        board.reset();
        moveCount = 0;
        currentPlayer = player1;

        cout << "\nPosition guide (enter 1-9):\n";
        cout << " 1 | 2 | 3\n 4 | 5 | 6\n 7 | 8 | 9\n";

        while (true) {
            cout << board;
            cout << currentPlayer->getName()
                 << " (" << currentPlayer->getSymbol() << "), enter position (1-9): ";

            int pos;
            cin >> pos;

            // Unit 4 - Exception Handling with try-catch
            try {
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(100, '\n');
                    throw invalid_argument("Please enter a number.");
                }
                board.placeSymbol(pos - 1, currentPlayer->getSymbol());
                moveCount++;

                // Unit 5 - push_back to vector (move history)
                moveHistory.push_back(
                    currentPlayer->getName() + " -> position " + to_string(pos)
                );

            } catch (const out_of_range& e) {
                cout << "Error: " << e.what() << " Try again.\n";
                continue;
            } catch (const invalid_argument& e) {
                cout << "Error: " << e.what() << " Try again.\n";
                continue;
            }

            // Check for win
            if (checkWin(currentPlayer->getSymbol())) {
                cout << board;
                cout << "\n*** " << currentPlayer->getName() << " WINS! ***\n";
                currentPlayer->incrementScore();
                winRecord[currentPlayer->getSymbol()]++;

                // Unit 4 - File I/O
                saveResult(currentPlayer->getName() + " won in " +
                           to_string(moveCount) + " moves.");
                return;
            }

            // Check for draw
            if (checkDraw()) {
                cout << board;
                cout << "\n*** It's a DRAW! ***\n";
                draws++;
                saveResult("Draw after " + to_string(moveCount) + " moves.");
                return;
            }

            switchPlayer();
        }
    }

    // ---------------------------------------------------------------
    // Main game loop
    // ---------------------------------------------------------------
    void start() {
        cout << "==============================\n";
        cout << "    TIC TAC TOE - C++ STL    \n";
        cout << "==============================\n";

        loadHistory();

        char choice;
        do {
            playRound();
            displayScoreboard();
            displayMoveHistory();

            cout << "\nPlay again? (y/n): ";
            cin >> choice;

        } while (choice == 'y' || choice == 'Y');

        cout << "\nThanks for playing!\n";
    }
};


// =====================================================================
// Unit 4 - Template function usage example
// =====================================================================
void demoSTL() {
    // Unit 5 - vector demo
    vector<string> log = {"Game started", "Player X joined", "Player O joined"};
    displayContainer(log, "Event Log");

    // Unit 5 - map demo (symbol -> win count)
    map<char, int> symbolWins;
    symbolWins['X'] = 0;
    symbolWins['O'] = 0;
    cout << "Initial win counts:\n";
    for (auto it = symbolWins.begin(); it != symbolWins.end(); ++it) {
        cout << "  Player " << it->first << " : " << it->second << " wins\n";
    }
}


// =====================================================================
// MAIN - Entry point
// =====================================================================
int main() {
    // Unit 4 - Template demo
    demoSTL();

    string p1, p2;
    cout << "\nEnter Player 1 name (plays as X): ";
    cin >> p1;
    cout << "Enter Player 2 name (plays as O): ";
    cin >> p2;

    // Unit 1 - Object creation, Unit 4 - Exception Handling at top level
    try {
        Game game(p1, p2);
        game.start();
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
