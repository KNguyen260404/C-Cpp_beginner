#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <iomanip>

using namespace std;

// Class to represent a Sudoku board
class SudokuBoard {
private:
    // The board is represented as a 9x9 grid
    vector<vector<int>> board;
    
    // Check if a number can be placed in a specific position
    bool isValid(int row, int col, int num) const {
        // Check row
        for (int i = 0; i < 9; i++) {
            if (board[row][i] == num) {
                return false;
            }
        }
        
        // Check column
        for (int i = 0; i < 9; i++) {
            if (board[i][col] == num) {
                return false;
            }
        }
        
        // Check 3x3 box
        int boxRow = row - row % 3;
        int boxCol = col - col % 3;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[boxRow + i][boxCol + j] == num) {
                    return false;
                }
            }
        }
        
        return true;
    }
    
    // Find an empty cell
    bool findEmptyCell(int& row, int& col) const {
        for (row = 0; row < 9; row++) {
            for (col = 0; col < 9; col++) {
                if (board[row][col] == 0) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Count the number of solutions for the current board
    int countSolutions() const {
        // Create a copy of the board
        SudokuBoard copy = *this;
        return copy.countSolutionsRecursive();
    }
    
    // Recursive function to count solutions
    int countSolutionsRecursive() {
        int row, col;
        
        // If no empty cell is found, we have a solution
        if (!findEmptyCell(row, col)) {
            return 1;
        }
        
        int count = 0;
        
        // Try each number from 1 to 9
        for (int num = 1; num <= 9; num++) {
            if (isValid(row, col, num)) {
                // Place the number
                board[row][col] = num;
                
                // Recursively count solutions
                count += countSolutionsRecursive();
                
                // If we have found more than one solution, we can stop
                if (count > 1) {
                    return count;
                }
                
                // Backtrack
                board[row][col] = 0;
            }
        }
        
        return count;
    }
    
public:
    // Constructor
    SudokuBoard() {
        // Initialize the board with zeros (empty cells)
        board.resize(9, vector<int>(9, 0));
    }
    
    // Get the value at a specific position
    int getValue(int row, int col) const {
        if (row >= 0 && row < 9 && col >= 0 && col < 9) {
            return board[row][col];
        }
        return 0;
    }
    
    // Set the value at a specific position
    bool setValue(int row, int col, int value) {
        if (row >= 0 && row < 9 && col >= 0 && col < 9 && value >= 0 && value <= 9) {
            // Check if the value is valid (if not 0)
            if (value != 0 && !isValid(row, col, value)) {
                return false;
            }
            
            board[row][col] = value;
            return true;
        }
        return false;
    }
    
    // Clear the board
    void clear() {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                board[i][j] = 0;
            }
        }
    }
    
    // Check if the board is valid
    bool isValidBoard() const {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                if (board[i][j] != 0) {
                    // Check if the current value is valid
                    int value = board[i][j];
                    
                    // Temporarily set the cell to 0
                    const_cast<SudokuBoard*>(this)->board[i][j] = 0;
                    
                    // Check if the value is valid in this position
                    bool valid = isValid(i, j, value);
                    
                    // Restore the value
                    const_cast<SudokuBoard*>(this)->board[i][j] = value;
                    
                    if (!valid) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    // Solve the Sudoku puzzle
    bool solve() {
        int row, col;
        
        // If no empty cell is found, the puzzle is solved
        if (!findEmptyCell(row, col)) {
            return true;
        }
        
        // Try each number from 1 to 9
        for (int num = 1; num <= 9; num++) {
            if (isValid(row, col, num)) {
                // Place the number
                board[row][col] = num;
                
                // Recursively solve the rest of the puzzle
                if (solve()) {
                    return true;
                }
                
                // If placing the number doesn't lead to a solution, backtrack
                board[row][col] = 0;
            }
        }
        
        // No solution found
        return false;
    }
    
    // Check if the puzzle has a unique solution
    bool hasUniqueSolution() const {
        return countSolutions() == 1;
    }
    
    // Generate a new Sudoku puzzle
    void generate(int difficulty) {
        // Start with a solved puzzle
        clear();
        solve();
        
        // Create a list of all cells
        vector<pair<int, int>> cells;
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                cells.push_back(make_pair(i, j));
            }
        }
        
        // Shuffle the cells
        random_shuffle(cells.begin(), cells.end());
        
        // Determine how many cells to remove based on difficulty
        int cellsToRemove;
        switch (difficulty) {
            case 1: // Easy
                cellsToRemove = 40;
                break;
            case 2: // Medium
                cellsToRemove = 50;
                break;
            case 3: // Hard
                cellsToRemove = 60;
                break;
            default:
                cellsToRemove = 45;
        }
        
        // Remove cells while ensuring the puzzle still has a unique solution
        for (const auto& cell : cells) {
            int row = cell.first;
            int col = cell.second;
            int value = board[row][col];
            
            // Try removing the cell
            board[row][col] = 0;
            
            // If the puzzle no longer has a unique solution, restore the cell
            if (!hasUniqueSolution()) {
                board[row][col] = value;
            } else {
                cellsToRemove--;
                if (cellsToRemove <= 0) {
                    break;
                }
            }
        }
    }
    
    // Load a Sudoku puzzle from a file
    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            return false;
        }
        
        clear();
        
        string line;
        int row = 0;
        
        while (getline(file, line) && row < 9) {
            // Remove whitespace
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            
            if (line.length() < 9) {
                continue; // Skip incomplete lines
            }
            
            for (int col = 0; col < 9; col++) {
                char c = line[col];
                if (c >= '0' && c <= '9') {
                    board[row][col] = c - '0';
                } else {
                    board[row][col] = 0;
                }
            }
            
            row++;
        }
        
        file.close();
        return row == 9;
    }
    
    // Save a Sudoku puzzle to a file
    bool saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file) {
            return false;
        }
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                file << board[i][j];
                if (j < 8) {
                    file << " ";
                }
            }
            file << endl;
        }
        
        file.close();
        return true;
    }
    
    // Display the Sudoku board
    void display() const {
        cout << "   1 2 3   4 5 6   7 8 9" << endl;
        cout << " +-------+-------+-------+" << endl;
        
        for (int i = 0; i < 9; i++) {
            cout << i + 1 << "| ";
            
            for (int j = 0; j < 9; j++) {
                if (board[i][j] == 0) {
                    cout << ". ";
                } else {
                    cout << board[i][j] << " ";
                }
                
                if (j % 3 == 2 && j < 8) {
                    cout << "| ";
                }
            }
            
            cout << "|" << endl;
            
            if (i % 3 == 2 && i < 8) {
                cout << " +-------+-------+-------+" << endl;
            }
        }
        
        cout << " +-------+-------+-------+" << endl;
    }
};

// Function to display the main menu
void displayMenu() {
    cout << "\n===== Sudoku Solver =====" << endl;
    cout << "1. Generate a new puzzle" << endl;
    cout << "2. Enter a puzzle manually" << endl;
    cout << "3. Load a puzzle from file" << endl;
    cout << "4. Solve the puzzle" << endl;
    cout << "5. Check if the puzzle is valid" << endl;
    cout << "6. Save the puzzle to file" << endl;
    cout << "7. Clear the board" << endl;
    cout << "0. Exit" << endl;
    cout << "=========================" << endl;
    cout << "Enter your choice: ";
}

// Function to display the difficulty menu
void displayDifficultyMenu() {
    cout << "\n===== Difficulty Level =====" << endl;
    cout << "1. Easy" << endl;
    cout << "2. Medium" << endl;
    cout << "3. Hard" << endl;
    cout << "===========================" << endl;
    cout << "Enter your choice: ";
}

// Main function
int main() {
    SudokuBoard board;
    int choice;
    string filename;
    bool puzzleLoaded = false;
    
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    cout << "Welcome to Sudoku Solver!" << endl;
    
    while (true) {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: { // Generate a new puzzle
                displayDifficultyMenu();
                
                int difficulty;
                if (!(cin >> difficulty) || difficulty < 1 || difficulty > 3) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid difficulty level. Using medium difficulty." << endl;
                    difficulty = 2;
                } else {
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                
                cout << "Generating puzzle..." << endl;
                board.generate(difficulty);
                puzzleLoaded = true;
                
                cout << "Puzzle generated successfully." << endl;
                board.display();
                break;
            }
                
            case 2: { // Enter a puzzle manually
                board.clear();
                
                cout << "Enter the Sudoku puzzle (9x9 grid):" << endl;
                cout << "Use 0 or . for empty cells, spaces are ignored." << endl;
                cout << "Example: 5 3 0 0 7 0 0 0 0" << endl;
                
                for (int i = 0; i < 9; i++) {
                    cout << "Row " << i + 1 << ": ";
                    string line;
                    getline(cin, line);
                    
                    // Parse the line
                    istringstream iss(line);
                    char c;
                    int col = 0;
                    
                    while (iss >> c && col < 9) {
                        if (c >= '0' && c <= '9') {
                            board.setValue(i, col, c - '0');
                            col++;
                        } else if (c == '.') {
                            board.setValue(i, col, 0);
                            col++;
                        }
                    }
                    
                    // If the row is incomplete, fill with zeros
                    while (col < 9) {
                        board.setValue(i, col, 0);
                        col++;
                    }
                }
                
                puzzleLoaded = true;
                cout << "Puzzle entered successfully." << endl;
                board.display();
                break;
            }
                
            case 3: { // Load a puzzle from file
                cout << "Enter the filename to load: ";
                getline(cin, filename);
                
                if (board.loadFromFile(filename)) {
                    puzzleLoaded = true;
                    cout << "Puzzle loaded successfully." << endl;
                    board.display();
                } else {
                    cout << "Failed to load puzzle from file." << endl;
                }
                break;
            }
                
            case 4: { // Solve the puzzle
                if (!puzzleLoaded) {
                    cout << "No puzzle loaded. Please load or generate a puzzle first." << endl;
                    break;
                }
                
                // Check if the puzzle is valid
                if (!board.isValidBoard()) {
                    cout << "The puzzle is invalid. Cannot solve." << endl;
                    break;
                }
                
                cout << "Solving puzzle..." << endl;
                
                // Solve the puzzle
                if (board.solve()) {
                    cout << "Puzzle solved successfully!" << endl;
                    board.display();
                } else {
                    cout << "The puzzle has no solution." << endl;
                }
                break;
            }
                
            case 5: { // Check if the puzzle is valid
                if (!puzzleLoaded) {
                    cout << "No puzzle loaded. Please load or generate a puzzle first." << endl;
                    break;
                }
                
                if (board.isValidBoard()) {
                    cout << "The puzzle is valid." << endl;
                    
                    // Check if the puzzle has a unique solution
                    if (board.hasUniqueSolution()) {
                        cout << "The puzzle has a unique solution." << endl;
                    } else {
                        cout << "The puzzle has multiple solutions or no solution." << endl;
                    }
                } else {
                    cout << "The puzzle is invalid." << endl;
                }
                break;
            }
                
            case 6: { // Save the puzzle to file
                if (!puzzleLoaded) {
                    cout << "No puzzle loaded. Please load or generate a puzzle first." << endl;
                    break;
                }
                
                cout << "Enter the filename to save: ";
                getline(cin, filename);
                
                if (board.saveToFile(filename)) {
                    cout << "Puzzle saved successfully." << endl;
                } else {
                    cout << "Failed to save puzzle to file." << endl;
                }
                break;
            }
                
            case 7: { // Clear the board
                board.clear();
                puzzleLoaded = false;
                cout << "Board cleared." << endl;
                break;
            }
                
            case 0: // Exit
                cout << "Thank you for using Sudoku Solver!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 