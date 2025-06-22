#include <iostream>
#include <vector>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <windows.h>

using namespace std;

// Constants
const int WIDTH = 40;
const int HEIGHT = 20;
const char SNAKE_BODY = 'o';
const char SNAKE_HEAD = 'O';
const char FOOD = '*';
const char EMPTY = ' ';
const char WALL = '#';
const int INITIAL_SNAKE_LENGTH = 3;
const int GAME_SPEED = 100; // milliseconds

// Direction enum
enum Direction {
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Position struct
struct Position {
    int x;
    int y;
    
    Position(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Game class
class SnakeGame {
private:
    bool gameOver;
    bool won;
    int score;
    vector<vector<char>> board;
    deque<Position> snake;
    Position food;
    Direction dir;
    
    // Initialize the game
    void init() {
        gameOver = false;
        won = false;
        score = 0;
        dir = STOP;
        
        // Initialize board
        board = vector<vector<char>>(HEIGHT, vector<char>(WIDTH, EMPTY));
        
        // Create walls
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1) {
                    board[i][j] = WALL;
                }
            }
        }
        
        // Initialize snake in the middle of the board
        int startX = WIDTH / 2;
        int startY = HEIGHT / 2;
        
        for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
            snake.push_front(Position(startX - i, startY));
        }
        
        // Generate initial food
        generateFood();
    }
    
    // Generate food at a random empty position
    void generateFood() {
        int x, y;
        do {
            x = rand() % (WIDTH - 2) + 1; // Avoid walls
            y = rand() % (HEIGHT - 2) + 1; // Avoid walls
        } while (!isEmpty(Position(x, y)));
        
        food = Position(x, y);
    }
    
    // Check if a position is empty (not snake or wall)
    bool isEmpty(const Position& pos) const {
        // Check if the position is within bounds
        if (pos.x <= 0 || pos.x >= WIDTH - 1 || pos.y <= 0 || pos.y >= HEIGHT - 1) {
            return false;
        }
        
        // Check if the position is occupied by the snake
        for (const auto& segment : snake) {
            if (segment == pos) {
                return false;
            }
        }
        
        return true;
    }
    
    // Update the board with current snake and food positions
    void updateBoard() {
        // Clear the board (except walls)
        for (int i = 1; i < HEIGHT - 1; i++) {
            for (int j = 1; j < WIDTH - 1; j++) {
                board[i][j] = EMPTY;
            }
        }
        
        // Place food
        board[food.y][food.x] = FOOD;
        
        // Place snake
        for (size_t i = 0; i < snake.size(); i++) {
            const Position& pos = snake[i];
            board[pos.y][pos.x] = (i == 0) ? SNAKE_HEAD : SNAKE_BODY;
        }
    }
    
public:
    SnakeGame() {
        srand(static_cast<unsigned int>(time(nullptr)));
        init();
    }
    
    // Process user input
    void input() {
        if (_kbhit()) {
            switch (_getch()) {
                case 'a':
                case 'A':
                case 75: // Left arrow
                    if (dir != RIGHT) dir = LEFT;
                    break;
                case 'd':
                case 'D':
                case 77: // Right arrow
                    if (dir != LEFT) dir = RIGHT;
                    break;
                case 'w':
                case 'W':
                case 72: // Up arrow
                    if (dir != DOWN) dir = UP;
                    break;
                case 's':
                case 'S':
                case 80: // Down arrow
                    if (dir != UP) dir = DOWN;
                    break;
                case 'x':
                case 'X':
                case 27: // ESC
                    gameOver = true;
                    break;
            }
        }
    }
    
    // Update game state
    void update() {
        if (gameOver) return;
        
        // Move the snake according to the direction
        Position newHead = snake.front();
        
        switch (dir) {
            case LEFT:
                newHead.x--;
                break;
            case RIGHT:
                newHead.x++;
                break;
            case UP:
                newHead.y--;
                break;
            case DOWN:
                newHead.y++;
                break;
            default:
                return; // If direction is STOP, don't move
        }
        
        // Check for collisions
        if (newHead.x <= 0 || newHead.x >= WIDTH - 1 || 
            newHead.y <= 0 || newHead.y >= HEIGHT - 1) {
            gameOver = true;
            return;
        }
        
        // Check for self-collision
        for (const auto& segment : snake) {
            if (newHead == segment) {
                gameOver = true;
                return;
            }
        }
        
        // Move the snake
        snake.push_front(newHead);
        
        // Check if food was eaten
        if (newHead == food) {
            score++;
            generateFood();
            
            // Check if the snake has filled the board (win condition)
            if (snake.size() == (WIDTH - 2) * (HEIGHT - 2) - 1) {
                gameOver = true;
                won = true;
            }
        } else {
            // If no food was eaten, remove the tail
            snake.pop_back();
        }
        
        // Update the board
        updateBoard();
    }
    
    // Render the game
    void render() const {
        // Clear the console
        system("cls");
        
        // Display the board
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                cout << board[i][j];
            }
            cout << endl;
        }
        
        // Display score
        cout << "Score: " << score << endl;
        
        // Display game over message
        if (gameOver) {
            if (won) {
                cout << "You won! Congratulations!" << endl;
            } else {
                cout << "Game Over!" << endl;
            }
        }
        
        // Display controls
        cout << "Controls: WASD or Arrow Keys to move, X or ESC to quit" << endl;
    }
    
    // Check if the game is over
    bool isGameOver() const {
        return gameOver;
    }
    
    // Get the current score
    int getScore() const {
        return score;
    }
    
    // Check if the player won
    bool hasWon() const {
        return won;
    }
};

// Function to set console cursor visibility
void setCursorVisibility(bool visible) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

// Main function
int main() {
    // Hide the cursor
    setCursorVisibility(false);
    
    // Display welcome message
    cout << "Welcome to Snake Game!" << endl;
    cout << "Controls: WASD or Arrow Keys to move, X or ESC to quit" << endl;
    cout << "Press any key to start..." << endl;
    _getch();
    
    // Create and run the game
    SnakeGame game;
    
    while (!game.isGameOver()) {
        game.input();
        game.update();
        game.render();
        Sleep(GAME_SPEED); // Control game speed
    }
    
    // Display final score
    cout << "Final Score: " << game.getScore() << endl;
    
    // Wait for user input before exiting
    cout << "Press any key to exit..." << endl;
    _getch();
    
    // Show the cursor again
    setCursorVisibility(true);
    
    return 0;
} 