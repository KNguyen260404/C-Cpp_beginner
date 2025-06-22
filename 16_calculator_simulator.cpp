#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stack>
#include <cctype>
#include <stdexcept>
#include <limits>

using namespace std;

// Constants
const double PI = 3.14159265358979323846;
const double E = 2.71828182845904523536;

// Class to represent a scientific calculator
class ScientificCalculator {
private:
    double memory;
    double ans;
    bool degreeMode;
    
    // Helper function to check if a character is an operator
    bool isOperator(char c) const {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '%';
    }
    
    // Helper function to get operator precedence
    int getPrecedence(char op) const {
        if (op == '+' || op == '-')
            return 1;
        if (op == '*' || op == '/' || op == '%')
            return 2;
        if (op == '^')
            return 3;
        return 0;
    }
    
    // Helper function to apply an operator
    double applyOperator(double a, double b, char op) const {
        switch (op) {
            case '+': return a + b;
            case '-': return a - b;
            case '*': return a * b;
            case '/': 
                if (b == 0) throw runtime_error("Division by zero");
                return a / b;
            case '^': return pow(a, b);
            case '%': 
                if (b == 0) throw runtime_error("Modulo by zero");
                return fmod(a, b);
            default: throw runtime_error("Unknown operator");
        }
    }
    
    // Convert degrees to radians if in degree mode
    double toRadians(double angle) const {
        return degreeMode ? angle * PI / 180.0 : angle;
    }
    
    // Convert radians to degrees if in degree mode
    double fromRadians(double angle) const {
        return degreeMode ? angle * 180.0 / PI : angle;
    }
    
    // Parse and evaluate a mathematical expression
    double evaluateExpression(const string& expression) {
        vector<string> tokens = tokenize(expression);
        
        stack<double> values;
        stack<char> operators;
        
        for (size_t i = 0; i < tokens.size(); i++) {
            const string& token = tokens[i];
            
            // Skip empty tokens
            if (token.empty()) continue;
            
            // Check for functions and constants
            if (isalpha(token[0])) {
                if (token == "pi" || token == "PI") {
                    values.push(PI);
                } else if (token == "e" || token == "E") {
                    values.push(E);
                } else if (token == "ans") {
                    values.push(ans);
                } else if (token == "sin") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('s'); // Use 's' to represent sin
                    } else {
                        throw runtime_error("Expected '(' after 'sin'");
                    }
                } else if (token == "cos") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('c'); // Use 'c' to represent cos
                    } else {
                        throw runtime_error("Expected '(' after 'cos'");
                    }
                } else if (token == "tan") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('t'); // Use 't' to represent tan
                    } else {
                        throw runtime_error("Expected '(' after 'tan'");
                    }
                } else if (token == "log") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('l'); // Use 'l' to represent log
                    } else {
                        throw runtime_error("Expected '(' after 'log'");
                    }
                } else if (token == "ln") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('n'); // Use 'n' to represent ln
                    } else {
                        throw runtime_error("Expected '(' after 'ln'");
                    }
                } else if (token == "sqrt") {
                    if (i + 1 < tokens.size() && tokens[i+1] == "(") {
                        operators.push('q'); // Use 'q' to represent sqrt
                    } else {
                        throw runtime_error("Expected '(' after 'sqrt'");
                    }
                } else {
                    throw runtime_error("Unknown function or constant: " + token);
                }
                continue;
            }
            
            // Check for numbers
            if (isdigit(token[0]) || (token[0] == '-' && token.length() > 1 && isdigit(token[1]))) {
                values.push(stod(token));
                continue;
            }
            
            // Check for operators and parentheses
            if (token.length() == 1) {
                char c = token[0];
                
                if (c == '(') {
                    operators.push(c);
                } else if (c == ')') {
                    while (!operators.empty() && operators.top() != '(') {
                        processOperator(values, operators);
                    }
                    
                    if (operators.empty()) {
                        throw runtime_error("Mismatched parentheses");
                    }
                    
                    operators.pop(); // Remove the '('
                    
                    // Check if this is closing a function call
                    if (!operators.empty() && (operators.top() == 's' || operators.top() == 'c' || 
                                              operators.top() == 't' || operators.top() == 'l' || 
                                              operators.top() == 'n' || operators.top() == 'q')) {
                        char func = operators.top();
                        operators.pop();
                        
                        double val = values.top();
                        values.pop();
                        
                        switch (func) {
                            case 's': values.push(sin(toRadians(val))); break;
                            case 'c': values.push(cos(toRadians(val))); break;
                            case 't': values.push(tan(toRadians(val))); break;
                            case 'l': 
                                if (val <= 0) throw runtime_error("Log of non-positive number");
                                values.push(log10(val)); 
                                break;
                            case 'n': 
                                if (val <= 0) throw runtime_error("Ln of non-positive number");
                                values.push(log(val)); 
                                break;
                            case 'q': 
                                if (val < 0) throw runtime_error("Square root of negative number");
                                values.push(sqrt(val)); 
                                break;
                        }
                    }
                } else if (isOperator(c)) {
                    while (!operators.empty() && isOperator(operators.top()) && 
                           getPrecedence(operators.top()) >= getPrecedence(c)) {
                        processOperator(values, operators);
                    }
                    operators.push(c);
                }
            }
        }
        
        // Process any remaining operators
        while (!operators.empty()) {
            if (operators.top() == '(') {
                throw runtime_error("Mismatched parentheses");
            }
            processOperator(values, operators);
        }
        
        if (values.size() != 1) {
            throw runtime_error("Invalid expression");
        }
        
        return values.top();
    }
    
    // Process an operator from the stack
    void processOperator(stack<double>& values, stack<char>& operators) {
        char op = operators.top();
        operators.pop();
        
        if (!isOperator(op)) {
            throw runtime_error("Invalid operator");
        }
        
        if (values.size() < 2) {
            throw runtime_error("Not enough operands for operator");
        }
        
        double b = values.top();
        values.pop();
        
        double a = values.top();
        values.pop();
        
        values.push(applyOperator(a, b, op));
    }
    
    // Tokenize an expression into individual components
    vector<string> tokenize(const string& expression) {
        vector<string> tokens;
        string current;
        
        for (size_t i = 0; i < expression.length(); i++) {
            char c = expression[i];
            
            if (isspace(c)) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else if (c == '(' || c == ')' || isOperator(c)) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.push_back(string(1, c));
            } else {
                current += c;
            }
        }
        
        if (!current.empty()) {
            tokens.push_back(current);
        }
        
        return tokens;
    }
    
public:
    ScientificCalculator() : memory(0), ans(0), degreeMode(true) {}
    
    // Calculate the result of an expression
    double calculate(const string& expression) {
        try {
            ans = evaluateExpression(expression);
            return ans;
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
            return ans; // Return previous result on error
        }
    }
    
    // Store a value in memory
    void storeInMemory(double value) {
        memory = value;
    }
    
    // Recall the value from memory
    double recallMemory() const {
        return memory;
    }
    
    // Clear the memory
    void clearMemory() {
        memory = 0;
    }
    
    // Add to memory
    void addToMemory(double value) {
        memory += value;
    }
    
    // Subtract from memory
    void subtractFromMemory(double value) {
        memory -= value;
    }
    
    // Get the last answer
    double getLastAnswer() const {
        return ans;
    }
    
    // Toggle between degree and radian mode
    void toggleAngleMode() {
        degreeMode = !degreeMode;
    }
    
    // Check if in degree mode
    bool isInDegreeMode() const {
        return degreeMode;
    }
};

// Function to display the calculator menu
void displayMenu() {
    cout << "\n===== Scientific Calculator =====" << endl;
    cout << "1. Calculate Expression" << endl;
    cout << "2. Store in Memory (M+)" << endl;
    cout << "3. Recall Memory (MR)" << endl;
    cout << "4. Clear Memory (MC)" << endl;
    cout << "5. Add to Memory (M+)" << endl;
    cout << "6. Subtract from Memory (M-)" << endl;
    cout << "7. Toggle Angle Mode (DEG/RAD)" << endl;
    cout << "8. Exit" << endl;
    cout << "===============================" << endl;
}

// Function to display help information
void displayHelp() {
    cout << "\n===== Calculator Help =====" << endl;
    cout << "Available operations:" << endl;
    cout << "  + : Addition" << endl;
    cout << "  - : Subtraction" << endl;
    cout << "  * : Multiplication" << endl;
    cout << "  / : Division" << endl;
    cout << "  ^ : Exponentiation (e.g., 2^3 = 8)" << endl;
    cout << "  % : Modulo (remainder)" << endl;
    cout << "\nAvailable functions:" << endl;
    cout << "  sin(x) : Sine function" << endl;
    cout << "  cos(x) : Cosine function" << endl;
    cout << "  tan(x) : Tangent function" << endl;
    cout << "  log(x) : Base-10 logarithm" << endl;
    cout << "  ln(x)  : Natural logarithm" << endl;
    cout << "  sqrt(x): Square root" << endl;
    cout << "\nConstants:" << endl;
    cout << "  pi : 3.14159..." << endl;
    cout << "  e  : 2.71828..." << endl;
    cout << "  ans: Last calculated result" << endl;
    cout << "\nExample expressions:" << endl;
    cout << "  2 + 3 * 4" << endl;
    cout << "  sin(45) + cos(30)" << endl;
    cout << "  log(100) + ln(e^2)" << endl;
    cout << "  2^3 + sqrt(16)" << endl;
    cout << "=============================" << endl;
}

int main() {
    ScientificCalculator calculator;
    int choice;
    string expression;
    double value;
    
    cout << "Welcome to Scientific Calculator Simulator!" << endl;
    cout << "Type 'help' when prompted for an expression to see available operations." << endl;
    
    while (true) {
        displayMenu();
        cout << "Current mode: " << (calculator.isInDegreeMode() ? "DEG" : "RAD") << endl;
        cout << "Last answer: " << calculator.getLastAnswer() << endl;
        cout << "Memory: " << calculator.recallMemory() << endl;
        cout << "Enter your choice: ";
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer
        
        switch (choice) {
            case 1: // Calculate Expression
                cout << "Enter expression: ";
                getline(cin, expression);
                
                if (expression == "help") {
                    displayHelp();
                } else {
                    double result = calculator.calculate(expression);
                    cout << "Result: " << result << endl;
                }
                break;
                
            case 2: // Store in Memory
                cout << "Enter value to store in memory: ";
                if (cin >> value) {
                    calculator.storeInMemory(value);
                    cout << "Value stored in memory." << endl;
                } else {
                    cin.clear();
                    cout << "Invalid input." << endl;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
                
            case 3: // Recall Memory
                cout << "Memory value: " << calculator.recallMemory() << endl;
                break;
                
            case 4: // Clear Memory
                calculator.clearMemory();
                cout << "Memory cleared." << endl;
                break;
                
            case 5: // Add to Memory
                cout << "Enter value to add to memory: ";
                if (cin >> value) {
                    calculator.addToMemory(value);
                    cout << "Value added to memory." << endl;
                } else {
                    cin.clear();
                    cout << "Invalid input." << endl;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
                
            case 6: // Subtract from Memory
                cout << "Enter value to subtract from memory: ";
                if (cin >> value) {
                    calculator.subtractFromMemory(value);
                    cout << "Value subtracted from memory." << endl;
                } else {
                    cin.clear();
                    cout << "Invalid input." << endl;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
                
            case 7: // Toggle Angle Mode
                calculator.toggleAngleMode();
                cout << "Angle mode toggled to " << (calculator.isInDegreeMode() ? "DEG" : "RAD") << endl;
                break;
                
            case 8: // Exit
                cout << "Thank you for using Scientific Calculator Simulator!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 