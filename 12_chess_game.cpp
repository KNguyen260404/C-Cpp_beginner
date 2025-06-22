#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <cmath>

using namespace std;

// Enum for piece types
enum PieceType {
    EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

// Enum for piece colors
enum PieceColor {
    NONE, WHITE, BLACK
};

// Structure to represent a chess piece
struct Piece {
    PieceType type;
    PieceColor color;
    
    Piece() : type(EMPTY), color(NONE) {}
    Piece(PieceType t, PieceColor c) : type(t), color(c) {}
};

// Structure to represent a position on the board
struct Position {
    int row;
    int col;
    
    Position() : row(0), col(0) {}
    Position(int r, int c) : row(r), col(c) {}
    
    bool isValid() const {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }
    
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

// Class to represent the chess board and game logic
class ChessBoard {
private:
    vector<vector<Piece>> board;
    PieceColor currentPlayer;
    
public:
    ChessBoard() : board(8, vector<Piece>(8)), currentPlayer(WHITE) {
        initializeBoard();
    }
    
    void initializeBoard() {
        // Initialize empty board
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                board[i][j] = Piece();
            }
        }
        
        // Set up pawns
        for (int i = 0; i < 8; i++) {
            board[1][i] = Piece(PAWN, BLACK);
            board[6][i] = Piece(PAWN, WHITE);
        }
        
        // Set up other black pieces
        board[0][0] = Piece(ROOK, BLACK);
        board[0][1] = Piece(KNIGHT, BLACK);
        board[0][2] = Piece(BISHOP, BLACK);
        board[0][3] = Piece(QUEEN, BLACK);
        board[0][4] = Piece(KING, BLACK);
        board[0][5] = Piece(BISHOP, BLACK);
        board[0][6] = Piece(KNIGHT, BLACK);
        board[0][7] = Piece(ROOK, BLACK);
        
        // Set up other white pieces
        board[7][0] = Piece(ROOK, WHITE);
        board[7][1] = Piece(KNIGHT, WHITE);
        board[7][2] = Piece(BISHOP, WHITE);
        board[7][3] = Piece(QUEEN, WHITE);
        board[7][4] = Piece(KING, WHITE);
        board[7][5] = Piece(BISHOP, WHITE);
        board[7][6] = Piece(KNIGHT, WHITE);
        board[7][7] = Piece(ROOK, WHITE);
    }
    
    void displayBoard() const {
        cout << "\n  a b c d e f g h" << endl;
        cout << " +-+-+-+-+-+-+-+-+" << endl;
        
        for (int i = 0; i < 8; i++) {
            cout << 8 - i << "|";
            
            for (int j = 0; j < 8; j++) {
                char pieceChar = getPieceChar(board[i][j]);
                cout << pieceChar << "|";
            }
            
            cout << 8 - i << endl;
            cout << " +-+-+-+-+-+-+-+-+" << endl;
        }
        
        cout << "  a b c d e f g h" << endl;
        
        cout << "\nCurrent player: " << (currentPlayer == WHITE ? "White" : "Black") << endl;
    }
    
    char getPieceChar(const Piece& piece) const {
        if (piece.type == EMPTY) {
            return ' ';
        }
        
        char c;
        switch (piece.type) {
            case PAWN: c = 'p'; break;
            case KNIGHT: c = 'n'; break;
            case BISHOP: c = 'b'; break;
            case ROOK: c = 'r'; break;
            case QUEEN: c = 'q'; break;
            case KING: c = 'k'; break;
            default: c = '?'; break;
        }
        
        return (piece.color == WHITE) ? toupper(c) : c;
    }
    
    bool makeMove(const string& moveStr) {
        if (moveStr.length() != 4) {
            cout << "Invalid move format. Use format 'e2e4'." << endl;
            return false;
        }
        
        Position from = stringToPosition(moveStr.substr(0, 2));
        Position to = stringToPosition(moveStr.substr(2, 2));
        
        if (!from.isValid() || !to.isValid()) {
            cout << "Invalid position." << endl;
            return false;
        }
        
        Piece& pieceFrom = board[from.row][from.col];
        
        // Check if there is a piece at the starting position
        if (pieceFrom.type == EMPTY) {
            cout << "No piece at the starting position." << endl;
            return false;
        }
        
        // Check if the piece belongs to the current player
        if (pieceFrom.color != currentPlayer) {
            cout << "That's not your piece." << endl;
            return false;
        }
        
        // Check if the move is valid for the piece
        if (!isValidMove(from, to)) {
            cout << "Invalid move for this piece." << endl;
            return false;
        }
        
        // Make the move
        board[to.row][to.col] = pieceFrom;
        board[from.row][from.col] = Piece();
        
        // Switch player
        currentPlayer = (currentPlayer == WHITE) ? BLACK : WHITE;
        
        return true;
    }
    
    Position stringToPosition(const string& posStr) const {
        if (posStr.length() != 2) {
            return Position(-1, -1);
        }
        
        int col = tolower(posStr[0]) - 'a';
        int row = 8 - (posStr[1] - '0');
        
        return Position(row, col);
    }
    
    bool isValidMove(const Position& from, const Position& to) const {
        // Simple implementation - doesn't check all chess rules
        // Just checks if the destination is empty or has an opponent's piece
        
        const Piece& pieceFrom = board[from.row][from.col];
        const Piece& pieceTo = board[to.row][to.col];
        
        // Can't move to a square occupied by own piece
        if (pieceTo.type != EMPTY && pieceTo.color == pieceFrom.color) {
            return false;
        }
        
        // Simple movement checks for each piece type
        switch (pieceFrom.type) {
            case PAWN: {
                int direction = (pieceFrom.color == WHITE) ? -1 : 1;
                int startRow = (pieceFrom.color == WHITE) ? 6 : 1;
                
                // Move forward one square
                if (to.col == from.col && to.row == from.row + direction && pieceTo.type == EMPTY) {
                    return true;
                }
                
                // Move forward two squares from starting position
                if (to.col == from.col && from.row == startRow && 
                    to.row == from.row + 2 * direction && 
                    pieceTo.type == EMPTY && 
                    board[from.row + direction][from.col].type == EMPTY) {
                    return true;
                }
                
                // Capture diagonally
                if (abs(to.col - from.col) == 1 && to.row == from.row + direction && 
                    pieceTo.type != EMPTY && pieceTo.color != pieceFrom.color) {
                    return true;
                }
                
                return false;
            }
            
            case KNIGHT:
                // Knight moves in an L-shape: 2 squares in one direction and 1 square perpendicular
                return (abs(to.row - from.row) == 2 && abs(to.col - from.col) == 1) ||
                       (abs(to.row - from.row) == 1 && abs(to.col - from.col) == 2);
                
            case BISHOP:
                // Bishop moves diagonally
                if (abs(to.row - from.row) != abs(to.col - from.col)) {
                    return false;
                }
                // Check if path is clear (simplified)
                return true;
                
            case ROOK:
                // Rook moves horizontally or vertically
                if (to.row != from.row && to.col != from.col) {
                    return false;
                }
                // Check if path is clear (simplified)
                return true;
                
            case QUEEN:
                // Queen moves like a rook or bishop
                if ((to.row != from.row && to.col != from.col) && 
                    (abs(to.row - from.row) != abs(to.col - from.col))) {
                    return false;
                }
                // Check if path is clear (simplified)
                return true;
                
            case KING:
                // King moves one square in any direction
                return abs(to.row - from.row) <= 1 && abs(to.col - from.col) <= 1;
                
            default:
                return false;
        }
    }
    
    bool isGameOver() const {
        // Simplified check - just see if a king is missing
        bool whiteKingFound = false;
        bool blackKingFound = false;
        
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (board[i][j].type == KING) {
                    if (board[i][j].color == WHITE) {
                        whiteKingFound = true;
                    } else {
                        blackKingFound = true;
                    }
                }
            }
        }
        
        return !whiteKingFound || !blackKingFound;
    }
    
    PieceColor getWinner() const {
        bool whiteKingFound = false;
        bool blackKingFound = false;
        
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (board[i][j].type == KING) {
                    if (board[i][j].color == WHITE) {
                        whiteKingFound = true;
                    } else {
                        blackKingFound = true;
                    }
                }
            }
        }
        
        if (!whiteKingFound) return BLACK;
        if (!blackKingFound) return WHITE;
        return NONE;
    }
};

int main() {
    ChessBoard board;
    string move;
    
    cout << "Welcome to Simple Chess!" << endl;
    cout << "Enter moves in the format 'e2e4' (from-to positions)." << endl;
    cout << "Type 'quit' to exit the game." << endl;
    
    while (true) {
        board.displayBoard();
        
        if (board.isGameOver()) {
            PieceColor winner = board.getWinner();
            cout << "Game over! " << (winner == WHITE ? "White" : "Black") << " wins!" << endl;
            break;
        }
        
        cout << "Enter move: ";
        cin >> move;
        
        if (move == "quit") {
            cout << "Thanks for playing!" << endl;
            break;
        }
        
        if (!board.makeMove(move)) {
            cout << "Invalid move. Try again." << endl;
        }
    }
    
    return 0;
} 