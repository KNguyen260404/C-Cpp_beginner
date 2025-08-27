#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stack>
#include <queue>
#include <functional>
#include <variant>
#include <regex>
#include <exception>
#include <iomanip>

// Forward declarations
class Token;
class ASTNode;
class Parser;
class Interpreter;
class CodeGenerator;

// Token types for lexical analysis
enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    BOOLEAN,
    IDENTIFIER,
    
    // Operators
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    AND,
    OR,
    NOT,
    
    // Keywords
    IF,
    ELSE,
    WHILE,
    FOR,
    FUNCTION,
    RETURN,
    VAR,
    CONST,
    TRUE,
    FALSE,
    NIL,
    PRINT,
    INPUT,
    
    // Delimiters
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    SEMICOLON,
    DOT,
    
    // Special
    NEWLINE,
    EOF_TOKEN,
    INVALID
};

// Token class
class Token {
public:
    TokenType type;
    std::string lexeme;
    std::variant<double, std::string, bool> literal;
    int line;
    int column;
    
    Token(TokenType t, const std::string& lex, int ln = 1, int col = 1)
        : type(t), lexeme(lex), line(ln), column(col) {}
    
    Token(TokenType t, const std::string& lex, double value, int ln = 1, int col = 1)
        : type(t), lexeme(lex), literal(value), line(ln), column(col) {}
    
    Token(TokenType t, const std::string& lex, const std::string& value, int ln = 1, int col = 1)
        : type(t), lexeme(lex), literal(value), line(ln), column(col) {}
    
    Token(TokenType t, const std::string& lex, bool value, int ln = 1, int col = 1)
        : type(t), lexeme(lex), literal(value), line(ln), column(col) {}
    
    std::string toString() const {
        std::string result = "Token(" + tokenTypeToString(type) + ", '" + lexeme + "'";
        if (std::holds_alternative<double>(literal)) {
            result += ", " + std::to_string(std::get<double>(literal));
        } else if (std::holds_alternative<std::string>(literal)) {
            result += ", \"" + std::get<std::string>(literal) + "\"";
        } else if (std::holds_alternative<bool>(literal)) {
            result += ", " + (std::get<bool>(literal) ? "true" : "false");
        }
        result += ")";
        return result;
    }
    
    static std::string tokenTypeToString(TokenType type) {
        static std::map<TokenType, std::string> typeNames = {
            {TokenType::NUMBER, "NUMBER"},
            {TokenType::STRING, "STRING"},
            {TokenType::BOOLEAN, "BOOLEAN"},
            {TokenType::IDENTIFIER, "IDENTIFIER"},
            {TokenType::PLUS, "PLUS"},
            {TokenType::MINUS, "MINUS"},
            {TokenType::MULTIPLY, "MULTIPLY"},
            {TokenType::DIVIDE, "DIVIDE"},
            {TokenType::ASSIGN, "ASSIGN"},
            {TokenType::EQUAL, "EQUAL"},
            {TokenType::IF, "IF"},
            {TokenType::ELSE, "ELSE"},
            {TokenType::WHILE, "WHILE"},
            {TokenType::FUNCTION, "FUNCTION"},
            {TokenType::LEFT_PAREN, "LEFT_PAREN"},
            {TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
            {TokenType::LEFT_BRACE, "LEFT_BRACE"},
            {TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
            {TokenType::SEMICOLON, "SEMICOLON"},
            {TokenType::EOF_TOKEN, "EOF"},
            {TokenType::INVALID, "INVALID"}
        };
        
        auto it = typeNames.find(type);
        return (it != typeNames.end()) ? it->second : "UNKNOWN";
    }
};

// Lexer/Scanner class
class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;
    size_t start;
    size_t current;
    int line;
    int column;
    
    static std::unordered_map<std::string, TokenType> keywords;
    
public:
    Lexer(const std::string& sourceCode) 
        : source(sourceCode), start(0), current(0), line(1), column(1) {}
    
    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            start = current;
            scanToken();
        }
        
        tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
        return tokens;
    }
    
private:
    bool isAtEnd() {
        return current >= source.length();
    }
    
    char advance() {
        column++;
        return source[current++];
    }
    
    char peek() {
        if (isAtEnd()) return '\0';
        return source[current];
    }
    
    char peekNext() {
        if (current + 1 >= source.length()) return '\0';
        return source[current + 1];
    }
    
    bool match(char expected) {
        if (isAtEnd()) return false;
        if (source[current] != expected) return false;
        
        current++;
        column++;
        return true;
    }
    
    void scanToken() {
        char c = advance();
        
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace
                break;
            case '\n':
                tokens.emplace_back(TokenType::NEWLINE, "\\n", line, column - 1);
                line++;
                column = 1;
                break;
            case '(':
                addToken(TokenType::LEFT_PAREN);
                break;
            case ')':
                addToken(TokenType::RIGHT_PAREN);
                break;
            case '{':
                addToken(TokenType::LEFT_BRACE);
                break;
            case '}':
                addToken(TokenType::RIGHT_BRACE);
                break;
            case '[':
                addToken(TokenType::LEFT_BRACKET);
                break;
            case ']':
                addToken(TokenType::RIGHT_BRACKET);
                break;
            case ',':
                addToken(TokenType::COMMA);
                break;
            case '.':
                addToken(TokenType::DOT);
                break;
            case '-':
                addToken(TokenType::MINUS);
                break;
            case '+':
                addToken(TokenType::PLUS);
                break;
            case ';':
                addToken(TokenType::SEMICOLON);
                break;
            case '*':
                addToken(TokenType::MULTIPLY);
                break;
            case '%':
                addToken(TokenType::MODULO);
                break;
            case '!':
                addToken(match('=') ? TokenType::NOT_EQUAL : TokenType::NOT);
                break;
            case '=':
                addToken(match('=') ? TokenType::EQUAL : TokenType::ASSIGN);
                break;
            case '<':
                addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
                break;
            case '>':
                addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
                break;
            case '&':
                if (match('&')) {
                    addToken(TokenType::AND);
                } else {
                    error("Unexpected character: &");
                }
                break;
            case '|':
                if (match('|')) {
                    addToken(TokenType::OR);
                } else {
                    error("Unexpected character: |");
                }
                break;
            case '/':
                if (match('/')) {
                    // Line comment
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else if (match('*')) {
                    // Block comment
                    blockComment();
                } else {
                    addToken(TokenType::DIVIDE);
                }
                break;
            case '"':
                string();
                break;
            default:
                if (isDigit(c)) {
                    number();
                } else if (isAlpha(c)) {
                    identifier();
                } else {
                    error("Unexpected character: " + std::string(1, c));
                }
                break;
        }
    }
    
    void blockComment() {
        while (peek() != '*' || peekNext() != '/') {
            if (peek() == '\n') {
                line++;
                column = 0;
            }
            if (isAtEnd()) {
                error("Unterminated block comment");
                return;
            }
            advance();
        }
        
        // Consume the closing */
        advance(); // *
        advance(); // /
    }
    
    void string() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') {
                line++;
                column = 0;
            }
            advance();
        }
        
        if (isAtEnd()) {
            error("Unterminated string");
            return;
        }
        
        // Consume closing "
        advance();
        
        // Extract string value (without quotes)
        std::string value = source.substr(start + 1, current - start - 2);
        addToken(TokenType::STRING, value);
    }
    
    void number() {
        while (isDigit(peek())) advance();
        
        // Look for decimal part
        if (peek() == '.' && isDigit(peekNext())) {
            advance(); // Consume .
            while (isDigit(peek())) advance();
        }
        
        double value = std::stod(source.substr(start, current - start));
        addToken(TokenType::NUMBER, value);
    }
    
    void identifier() {
        while (isAlphaNumeric(peek())) advance();
        
        std::string text = source.substr(start, current - start);
        TokenType type = TokenType::IDENTIFIER;
        
        auto it = keywords.find(text);
        if (it != keywords.end()) {
            type = it->second;
        }
        
        if (type == TokenType::TRUE) {
            addToken(type, true);
        } else if (type == TokenType::FALSE) {
            addToken(type, false);
        } else {
            addToken(type);
        }
    }
    
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
    
    bool isAlpha(char c) {
        return (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               c == '_';
    }
    
    bool isAlphaNumeric(char c) {
        return isAlpha(c) || isDigit(c);
    }
    
    void addToken(TokenType type) {
        std::string text = source.substr(start, current - start);
        tokens.emplace_back(type, text, line, column - text.length());
    }
    
    void addToken(TokenType type, double literal) {
        std::string text = source.substr(start, current - start);
        tokens.emplace_back(type, text, literal, line, column - text.length());
    }
    
    void addToken(TokenType type, const std::string& literal) {
        std::string text = source.substr(start, current - start);
        tokens.emplace_back(type, text, literal, line, column - text.length());
    }
    
    void addToken(TokenType type, bool literal) {
        std::string text = source.substr(start, current - start);
        tokens.emplace_back(type, text, literal, line, column - text.length());
    }
    
    void error(const std::string& message) {
        std::cerr << "Lexer error at line " << line << ", column " << column 
                  << ": " << message << std::endl;
        tokens.emplace_back(TokenType::INVALID, "", line, column);
    }
};

// Initialize keywords map
std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"and", TokenType::AND},
    {"or", TokenType::OR},
    {"not", TokenType::NOT},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"function", TokenType::FUNCTION},
    {"return", TokenType::RETURN},
    {"var", TokenType::VAR},
    {"const", TokenType::CONST},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"nil", TokenType::NIL},
    {"print", TokenType::PRINT},
    {"input", TokenType::INPUT}
};

// AST Node types
enum class ASTNodeType {
    PROGRAM,
    LITERAL,
    IDENTIFIER,
    BINARY_OP,
    UNARY_OP,
    ASSIGNMENT,
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    FUNCTION_CALL,
    IF_STATEMENT,
    WHILE_STATEMENT,
    FOR_STATEMENT,
    RETURN_STATEMENT,
    PRINT_STATEMENT,
    BLOCK,
    EXPRESSION_STATEMENT
};

// AST Node base class
class ASTNode {
public:
    ASTNodeType type;
    int line;
    int column;
    
    ASTNode(ASTNodeType t, int ln = 0, int col = 0) 
        : type(t), line(ln), column(col) {}
    
    virtual ~ASTNode() = default;
    virtual std::string toString(int indent = 0) const = 0;
    
protected:
    std::string getIndent(int level) const {
        return std::string(level * 2, ' ');
    }
};

// Specific AST Node types
class LiteralNode : public ASTNode {
public:
    std::variant<double, std::string, bool> value;
    
    LiteralNode(double val, int ln = 0, int col = 0)
        : ASTNode(ASTNodeType::LITERAL, ln, col), value(val) {}
    
    LiteralNode(const std::string& val, int ln = 0, int col = 0)
        : ASTNode(ASTNodeType::LITERAL, ln, col), value(val) {}
    
    LiteralNode(bool val, int ln = 0, int col = 0)
        : ASTNode(ASTNodeType::LITERAL, ln, col), value(val) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Literal(";
        if (std::holds_alternative<double>(value)) {
            result += std::to_string(std::get<double>(value));
        } else if (std::holds_alternative<std::string>(value)) {
            result += "\"" + std::get<std::string>(value) + "\"";
        } else if (std::holds_alternative<bool>(value)) {
            result += std::get<bool>(value) ? "true" : "false";
        }
        result += ")";
        return result;
    }
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    
    IdentifierNode(const std::string& n, int ln = 0, int col = 0)
        : ASTNode(ASTNodeType::IDENTIFIER, ln, col), name(n) {}
    
    std::string toString(int indent = 0) const override {
        return getIndent(indent) + "Identifier(" + name + ")";
    }
};

class BinaryOpNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    TokenType operator_;
    std::unique_ptr<ASTNode> right;
    
    BinaryOpNode(std::unique_ptr<ASTNode> l, TokenType op, std::unique_ptr<ASTNode> r)
        : ASTNode(ASTNodeType::BINARY_OP), left(std::move(l)), operator_(op), right(std::move(r)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "BinaryOp(" + Token::tokenTypeToString(operator_) + ")\n";
        result += left->toString(indent + 1) + "\n";
        result += right->toString(indent + 1);
        return result;
    }
};

class UnaryOpNode : public ASTNode {
public:
    TokenType operator_;
    std::unique_ptr<ASTNode> operand;
    
    UnaryOpNode(TokenType op, std::unique_ptr<ASTNode> operand_)
        : ASTNode(ASTNodeType::UNARY_OP), operator_(op), operand(std::move(operand_)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "UnaryOp(" + Token::tokenTypeToString(operator_) + ")\n";
        result += operand->toString(indent + 1);
        return result;
    }
};

class AssignmentNode : public ASTNode {
public:
    std::string variable;
    std::unique_ptr<ASTNode> value;
    
    AssignmentNode(const std::string& var, std::unique_ptr<ASTNode> val)
        : ASTNode(ASTNodeType::ASSIGNMENT), variable(var), value(std::move(val)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Assignment(" + variable + ")\n";
        result += value->toString(indent + 1);
        return result;
    }
};

class VariableDeclarationNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> initializer;
    bool isConstant;
    
    VariableDeclarationNode(const std::string& n, std::unique_ptr<ASTNode> init, bool constant = false)
        : ASTNode(ASTNodeType::VARIABLE_DECLARATION), name(n), initializer(std::move(init)), isConstant(constant) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + (isConstant ? "Const(" : "Var(") + name + ")";
        if (initializer) {
            result += "\n" + initializer->toString(indent + 1);
        }
        return result;
    }
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    BlockNode() : ASTNode(ASTNodeType::BLOCK) {}
    
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Block";
        for (const auto& stmt : statements) {
            result += "\n" + stmt->toString(indent + 1);
        }
        return result;
    }
};

class FunctionDeclarationNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockNode> body;
    
    FunctionDeclarationNode(const std::string& n, std::vector<std::string> params, std::unique_ptr<BlockNode> b)
        : ASTNode(ASTNodeType::FUNCTION_DECLARATION), name(n), parameters(std::move(params)), body(std::move(b)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Function(" + name + ", params: [";
        for (size_t i = 0; i < parameters.size(); i++) {
            if (i > 0) result += ", ";
            result += parameters[i];
        }
        result += "])\n";
        result += body->toString(indent + 1);
        return result;
    }
};

class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    
    FunctionCallNode(const std::string& n) : ASTNode(ASTNodeType::FUNCTION_CALL), name(n) {}
    
    void addArgument(std::unique_ptr<ASTNode> arg) {
        arguments.push_back(std::move(arg));
    }
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "FunctionCall(" + name + ")";
        for (const auto& arg : arguments) {
            result += "\n" + arg->toString(indent + 1);
        }
        return result;
    }
};

class IfStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBranch;
    std::unique_ptr<ASTNode> elseBranch;
    
    IfStatementNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> then_, std::unique_ptr<ASTNode> else_ = nullptr)
        : ASTNode(ASTNodeType::IF_STATEMENT), condition(std::move(cond)), thenBranch(std::move(then_)), elseBranch(std::move(else_)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "If\n";
        result += getIndent(indent + 1) + "Condition:\n" + condition->toString(indent + 2) + "\n";
        result += getIndent(indent + 1) + "Then:\n" + thenBranch->toString(indent + 2);
        if (elseBranch) {
            result += "\n" + getIndent(indent + 1) + "Else:\n" + elseBranch->toString(indent + 2);
        }
        return result;
    }
};

class WhileStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;
    
    WhileStatementNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> b)
        : ASTNode(ASTNodeType::WHILE_STATEMENT), condition(std::move(cond)), body(std::move(b)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "While\n";
        result += getIndent(indent + 1) + "Condition:\n" + condition->toString(indent + 2) + "\n";
        result += getIndent(indent + 1) + "Body:\n" + body->toString(indent + 2);
        return result;
    }
};

class PrintStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    
    PrintStatementNode(std::unique_ptr<ASTNode> expr)
        : ASTNode(ASTNodeType::PRINT_STATEMENT), expression(std::move(expr)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Print\n";
        result += expression->toString(indent + 1);
        return result;
    }
};

class ReturnStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    
    ReturnStatementNode(std::unique_ptr<ASTNode> expr = nullptr)
        : ASTNode(ASTNodeType::RETURN_STATEMENT), expression(std::move(expr)) {}
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Return";
        if (expression) {
            result += "\n" + expression->toString(indent + 1);
        }
        return result;
    }
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}
    
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    std::string toString(int indent = 0) const override {
        std::string result = getIndent(indent) + "Program";
        for (const auto& stmt : statements) {
            result += "\n" + stmt->toString(indent + 1);
        }
        return result;
    }
};

// Parser exception
class ParseException : public std::exception {
private:
    std::string message;
    
public:
    ParseException(const std::string& msg) : message(msg) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
};

// Recursive descent parser
class Parser {
private:
    std::vector<Token> tokens;
    size_t current;
    
public:
    Parser(const std::vector<Token>& tokenList) : tokens(tokenList), current(0) {}
    
    std::unique_ptr<ProgramNode> parse() {
        auto program = std::make_unique<ProgramNode>();
        
        while (!isAtEnd()) {
            if (match(TokenType::NEWLINE)) continue;
            
            try {
                auto stmt = statement();
                if (stmt) {
                    program->addStatement(std::move(stmt));
                }
            } catch (const ParseException& e) {
                std::cerr << "Parse error: " << e.what() << std::endl;
                synchronize();
            }
        }
        
        return program;
    }
    
private:
    bool isAtEnd() {
        return peek().type == TokenType::EOF_TOKEN;
    }
    
    Token peek() {
        return tokens[current];
    }
    
    Token previous() {
        return tokens[current - 1];
    }
    
    bool check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }
    
    Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }
    
    bool match(std::initializer_list<TokenType> types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }
    
    Token consume(TokenType type, const std::string& message) {
        if (check(type)) return advance();
        
        throw ParseException(message + " at line " + std::to_string(peek().line));
    }
    
    void synchronize() {
        advance();
        
        while (!isAtEnd()) {
            if (previous().type == TokenType::SEMICOLON) return;
            
            switch (peek().type) {
                case TokenType::FUNCTION:
                case TokenType::VAR:
                case TokenType::FOR:
                case TokenType::IF:
                case TokenType::WHILE:
                case TokenType::PRINT:
                case TokenType::RETURN:
                    return;
                default:
                    break;
            }
            
            advance();
        }
    }
    
    std::unique_ptr<ASTNode> statement() {
        if (match({TokenType::VAR})) return variableDeclaration(false);
        if (match({TokenType::CONST})) return variableDeclaration(true);
        if (match({TokenType::FUNCTION})) return functionDeclaration();
        if (match({TokenType::IF})) return ifStatement();
        if (match({TokenType::WHILE})) return whileStatement();
        if (match({TokenType::PRINT})) return printStatement();
        if (match({TokenType::RETURN})) return returnStatement();
        if (match({TokenType::LEFT_BRACE})) return block();
        
        return expressionStatement();
    }
    
    std::unique_ptr<ASTNode> variableDeclaration(bool isConstant) {
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name");
        
        std::unique_ptr<ASTNode> initializer = nullptr;
        if (match({TokenType::ASSIGN})) {
            initializer = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
        return std::make_unique<VariableDeclarationNode>(name.lexeme, std::move(initializer), isConstant);
    }
    
    std::unique_ptr<ASTNode> functionDeclaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expected function name");
        
        consume(TokenType::LEFT_PAREN, "Expected '(' after function name");
        
        std::vector<std::string> parameters;
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                Token param = consume(TokenType::IDENTIFIER, "Expected parameter name");
                parameters.push_back(param.lexeme);
            } while (match({TokenType::COMMA}));
        }
        
        consume(TokenType::RIGHT_PAREN, "Expected ')' after parameters");
        consume(TokenType::LEFT_BRACE, "Expected '{' before function body");
        
        auto body = std::unique_ptr<BlockNode>(dynamic_cast<BlockNode*>(block().release()));
        
        return std::make_unique<FunctionDeclarationNode>(name.lexeme, std::move(parameters), std::move(body));
    }
    
    std::unique_ptr<ASTNode> ifStatement() {
        consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition");
        
        auto thenBranch = statement();
        std::unique_ptr<ASTNode> elseBranch = nullptr;
        
        if (match({TokenType::ELSE})) {
            elseBranch = statement();
        }
        
        return std::make_unique<IfStatementNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }
    
    std::unique_ptr<ASTNode> whileStatement() {
        consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'");
        auto condition = expression();
        consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition");
        
        auto body = statement();
        
        return std::make_unique<WhileStatementNode>(std::move(condition), std::move(body));
    }
    
    std::unique_ptr<ASTNode> printStatement() {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after print expression");
        return std::make_unique<PrintStatementNode>(std::move(expr));
    }
    
    std::unique_ptr<ASTNode> returnStatement() {
        std::unique_ptr<ASTNode> expr = nullptr;
        if (!check(TokenType::SEMICOLON)) {
            expr = expression();
        }
        
        consume(TokenType::SEMICOLON, "Expected ';' after return statement");
        return std::make_unique<ReturnStatementNode>(std::move(expr));
    }
    
    std::unique_ptr<ASTNode> block() {
        auto blockNode = std::make_unique<BlockNode>();
        
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            if (match({TokenType::NEWLINE})) continue;
            
            auto stmt = statement();
            if (stmt) {
                blockNode->addStatement(std::move(stmt));
            }
        }
        
        consume(TokenType::RIGHT_BRACE, "Expected '}' after block");
        return std::move(blockNode);
    }
    
    std::unique_ptr<ASTNode> expressionStatement() {
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        return expr;
    }
    
    std::unique_ptr<ASTNode> expression() {
        return assignment();
    }
    
    std::unique_ptr<ASTNode> assignment() {
        auto expr = logicalOr();
        
        if (match({TokenType::ASSIGN})) {
            Token equals = previous();
            auto value = assignment();
            
            if (auto identifier = dynamic_cast<IdentifierNode*>(expr.get())) {
                std::string name = identifier->name;
                expr.release(); // Release ownership since we're creating a new node
                return std::make_unique<AssignmentNode>(name, std::move(value));
            }
            
            throw ParseException("Invalid assignment target at line " + std::to_string(equals.line));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> logicalOr() {
        auto expr = logicalAnd();
        
        while (match({TokenType::OR})) {
            TokenType operator_ = previous().type;
            auto right = logicalAnd();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> logicalAnd() {
        auto expr = equality();
        
        while (match({TokenType::AND})) {
            TokenType operator_ = previous().type;
            auto right = equality();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> equality() {
        auto expr = comparison();
        
        while (match({TokenType::NOT_EQUAL, TokenType::EQUAL})) {
            TokenType operator_ = previous().type;
            auto right = comparison();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> comparison() {
        auto expr = term();
        
        while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
            TokenType operator_ = previous().type;
            auto right = term();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> term() {
        auto expr = factor();
        
        while (match({TokenType::MINUS, TokenType::PLUS})) {
            TokenType operator_ = previous().type;
            auto right = factor();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> factor() {
        auto expr = unary();
        
        while (match({TokenType::DIVIDE, TokenType::MULTIPLY, TokenType::MODULO})) {
            TokenType operator_ = previous().type;
            auto right = unary();
            expr = std::make_unique<BinaryOpNode>(std::move(expr), operator_, std::move(right));
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> unary() {
        if (match({TokenType::NOT, TokenType::MINUS})) {
            TokenType operator_ = previous().type;
            auto right = unary();
            return std::make_unique<UnaryOpNode>(operator_, std::move(right));
        }
        
        return call();
    }
    
    std::unique_ptr<ASTNode> call() {
        auto expr = primary();
        
        while (true) {
            if (match({TokenType::LEFT_PAREN})) {
                expr = finishCall(std::move(expr));
            } else {
                break;
            }
        }
        
        return expr;
    }
    
    std::unique_ptr<ASTNode> finishCall(std::unique_ptr<ASTNode> callee) {
        auto identifier = dynamic_cast<IdentifierNode*>(callee.get());
        if (!identifier) {
            throw ParseException("Invalid function call");
        }
        
        std::string name = identifier->name;
        auto call = std::make_unique<FunctionCallNode>(name);
        
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                call->addArgument(expression());
            } while (match({TokenType::COMMA}));
        }
        
        consume(TokenType::RIGHT_PAREN, "Expected ')' after arguments");
        return std::move(call);
    }
    
    std::unique_ptr<ASTNode> primary() {
        if (match({TokenType::TRUE})) {
            return std::make_unique<LiteralNode>(true);
        }
        
        if (match({TokenType::FALSE})) {
            return std::make_unique<LiteralNode>(false);
        }
        
        if (match({TokenType::NIL})) {
            return std::make_unique<LiteralNode>(0.0); // Represent nil as 0
        }
        
        if (match({TokenType::NUMBER})) {
            return std::make_unique<LiteralNode>(std::get<double>(previous().literal));
        }
        
        if (match({TokenType::STRING})) {
            return std::make_unique<LiteralNode>(std::get<std::string>(previous().literal));
        }
        
        if (match({TokenType::IDENTIFIER})) {
            return std::make_unique<IdentifierNode>(previous().lexeme);
        }
        
        if (match({TokenType::LEFT_PAREN})) {
            auto expr = expression();
            consume(TokenType::RIGHT_PAREN, "Expected ')' after expression");
            return expr;
        }
        
        throw ParseException("Expected expression at line " + std::to_string(peek().line));
    }
};

// Runtime value type
using RuntimeValue = std::variant<double, std::string, bool>;

// Environment for variable storage
class Environment {
private:
    std::unordered_map<std::string, RuntimeValue> variables;
    std::shared_ptr<Environment> enclosing;
    
public:
    Environment() : enclosing(nullptr) {}
    Environment(std::shared_ptr<Environment> enc) : enclosing(enc) {}
    
    void define(const std::string& name, const RuntimeValue& value) {
        variables[name] = value;
    }
    
    RuntimeValue get(const std::string& name) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        
        if (enclosing) {
            return enclosing->get(name);
        }
        
        throw std::runtime_error("Undefined variable '" + name + "'");
    }
    
    void assign(const std::string& name, const RuntimeValue& value) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            it->second = value;
            return;
        }
        
        if (enclosing) {
            enclosing->assign(name, value);
            return;
        }
        
        throw std::runtime_error("Undefined variable '" + name + "'");
    }
    
    bool isDefined(const std::string& name) {
        if (variables.find(name) != variables.end()) {
            return true;
        }
        
        if (enclosing) {
            return enclosing->isDefined(name);
        }
        
        return false;
    }
};

// Function object
class Function {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockNode> body;
    std::shared_ptr<Environment> closure;
    
    Function(const std::string& n, std::vector<std::string> params, 
             std::unique_ptr<BlockNode> b, std::shared_ptr<Environment> env)
        : name(n), parameters(std::move(params)), closure(env) {
        // Deep copy the body
        body = std::unique_ptr<BlockNode>(dynamic_cast<BlockNode*>(b.release()));
    }
};

// Return exception for control flow
class ReturnException : public std::exception {
public:
    RuntimeValue value;
    
    ReturnException(const RuntimeValue& val) : value(val) {}
};

// Interpreter class
class Interpreter {
private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    std::unordered_map<std::string, Function> functions;
    
public:
    Interpreter() {
        globals = std::make_shared<Environment>();
        environment = globals;
    }
    
    void interpret(const std::unique_ptr<ProgramNode>& program) {
        try {
            for (const auto& statement : program->statements) {
                execute(statement.get());
            }
        } catch (const std::exception& e) {
            std::cerr << "Runtime error: " << e.what() << std::endl;
        }
    }
    
private:
    void execute(ASTNode* node) {
        if (!node) return;
        
        switch (node->type) {
            case ASTNodeType::VARIABLE_DECLARATION:
                executeVariableDeclaration(dynamic_cast<VariableDeclarationNode*>(node));
                break;
            case ASTNodeType::FUNCTION_DECLARATION:
                executeFunctionDeclaration(dynamic_cast<FunctionDeclarationNode*>(node));
                break;
            case ASTNodeType::IF_STATEMENT:
                executeIfStatement(dynamic_cast<IfStatementNode*>(node));
                break;
            case ASTNodeType::WHILE_STATEMENT:
                executeWhileStatement(dynamic_cast<WhileStatementNode*>(node));
                break;
            case ASTNodeType::PRINT_STATEMENT:
                executePrintStatement(dynamic_cast<PrintStatementNode*>(node));
                break;
            case ASTNodeType::RETURN_STATEMENT:
                executeReturnStatement(dynamic_cast<ReturnStatementNode*>(node));
                break;
            case ASTNodeType::BLOCK:
                executeBlock(dynamic_cast<BlockNode*>(node));
                break;
            case ASTNodeType::ASSIGNMENT:
                executeAssignment(dynamic_cast<AssignmentNode*>(node));
                break;
            default:
                // Expression statement
                evaluate(node);
                break;
        }
    }
    
    void executeVariableDeclaration(VariableDeclarationNode* node) {
        RuntimeValue value = 0.0; // Default value
        
        if (node->initializer) {
            value = evaluate(node->initializer.get());
        }
        
        environment->define(node->name, value);
    }
    
    void executeFunctionDeclaration(FunctionDeclarationNode* node) {
        Function function(node->name, node->parameters, 
                         std::unique_ptr<BlockNode>(dynamic_cast<BlockNode*>(node->body.release())), 
                         environment);
        functions[node->name] = std::move(function);
    }
    
    void executeIfStatement(IfStatementNode* node) {
        RuntimeValue condition = evaluate(node->condition.get());
        
        if (isTruthy(condition)) {
            execute(node->thenBranch.get());
        } else if (node->elseBranch) {
            execute(node->elseBranch.get());
        }
    }
    
    void executeWhileStatement(WhileStatementNode* node) {
        while (isTruthy(evaluate(node->condition.get()))) {
            execute(node->body.get());
        }
    }
    
    void executePrintStatement(PrintStatementNode* node) {
        RuntimeValue value = evaluate(node->expression.get());
        std::cout << valueToString(value) << std::endl;
    }
    
    void executeReturnStatement(ReturnStatementNode* node) {
        RuntimeValue value = 0.0;
        
        if (node->expression) {
            value = evaluate(node->expression.get());
        }
        
        throw ReturnException(value);
    }
    
    void executeBlock(BlockNode* node) {
        executeBlock(node, std::make_shared<Environment>(environment));
    }
    
    void executeBlock(BlockNode* node, std::shared_ptr<Environment> env) {
        auto previous = environment;
        
        try {
            environment = env;
            
            for (const auto& statement : node->statements) {
                execute(statement.get());
            }
        } catch (...) {
            environment = previous;
            throw;
        }
        
        environment = previous;
    }
    
    void executeAssignment(AssignmentNode* node) {
        RuntimeValue value = evaluate(node->value.get());
        environment->assign(node->variable, value);
    }
    
    RuntimeValue evaluate(ASTNode* node) {
        if (!node) return 0.0;
        
        switch (node->type) {
            case ASTNodeType::LITERAL:
                return evaluateLiteral(dynamic_cast<LiteralNode*>(node));
            case ASTNodeType::IDENTIFIER:
                return evaluateIdentifier(dynamic_cast<IdentifierNode*>(node));
            case ASTNodeType::BINARY_OP:
                return evaluateBinaryOp(dynamic_cast<BinaryOpNode*>(node));
            case ASTNodeType::UNARY_OP:
                return evaluateUnaryOp(dynamic_cast<UnaryOpNode*>(node));
            case ASTNodeType::FUNCTION_CALL:
                return evaluateFunctionCall(dynamic_cast<FunctionCallNode*>(node));
            case ASTNodeType::ASSIGNMENT:
                executeAssignment(dynamic_cast<AssignmentNode*>(node));
                return environment->get(dynamic_cast<AssignmentNode*>(node)->variable);
            default:
                throw std::runtime_error("Unknown expression type");
        }
    }
    
    RuntimeValue evaluateLiteral(LiteralNode* node) {
        return node->value;
    }
    
    RuntimeValue evaluateIdentifier(IdentifierNode* node) {
        return environment->get(node->name);
    }
    
    RuntimeValue evaluateBinaryOp(BinaryOpNode* node) {
        RuntimeValue left = evaluate(node->left.get());
        RuntimeValue right = evaluate(node->right.get());
        
        switch (node->operator_) {
            case TokenType::PLUS:
                if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                    return std::get<double>(left) + std::get<double>(right);
                }
                if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
                    return valueToString(left) + valueToString(right);
                }
                break;
            case TokenType::MINUS:
                checkNumberOperands(left, right);
                return std::get<double>(left) - std::get<double>(right);
            case TokenType::MULTIPLY:
                checkNumberOperands(left, right);
                return std::get<double>(left) * std::get<double>(right);
            case TokenType::DIVIDE:
                checkNumberOperands(left, right);
                if (std::get<double>(right) == 0) {
                    throw std::runtime_error("Division by zero");
                }
                return std::get<double>(left) / std::get<double>(right);
            case TokenType::MODULO:
                checkNumberOperands(left, right);
                return fmod(std::get<double>(left), std::get<double>(right));
            case TokenType::GREATER:
                checkNumberOperands(left, right);
                return std::get<double>(left) > std::get<double>(right);
            case TokenType::GREATER_EQUAL:
                checkNumberOperands(left, right);
                return std::get<double>(left) >= std::get<double>(right);
            case TokenType::LESS:
                checkNumberOperands(left, right);
                return std::get<double>(left) < std::get<double>(right);
            case TokenType::LESS_EQUAL:
                checkNumberOperands(left, right);
                return std::get<double>(left) <= std::get<double>(right);
            case TokenType::EQUAL:
                return isEqual(left, right);
            case TokenType::NOT_EQUAL:
                return !isEqual(left, right);
            case TokenType::AND:
                if (!isTruthy(left)) return left;
                return right;
            case TokenType::OR:
                if (isTruthy(left)) return left;
                return right;
            default:
                break;
        }
        
        throw std::runtime_error("Unknown binary operator");
    }
    
    RuntimeValue evaluateUnaryOp(UnaryOpNode* node) {
        RuntimeValue operand = evaluate(node->operand.get());
        
        switch (node->operator_) {
            case TokenType::MINUS:
                checkNumberOperand(operand);
                return -std::get<double>(operand);
            case TokenType::NOT:
                return !isTruthy(operand);
            default:
                break;
        }
        
        throw std::runtime_error("Unknown unary operator");
    }
    
    RuntimeValue evaluateFunctionCall(FunctionCallNode* node) {
        auto it = functions.find(node->name);
        if (it == functions.end()) {
            throw std::runtime_error("Undefined function '" + node->name + "'");
        }
        
        Function& function = it->second;
        
        if (node->arguments.size() != function.parameters.size()) {
            throw std::runtime_error("Expected " + std::to_string(function.parameters.size()) + 
                                   " arguments but got " + std::to_string(node->arguments.size()));
        }
        
        // Create new environment for function execution
        auto funcEnv = std::make_shared<Environment>(function.closure);
        
        // Bind parameters
        for (size_t i = 0; i < function.parameters.size(); i++) {
            RuntimeValue arg = evaluate(node->arguments[i].get());
            funcEnv->define(function.parameters[i], arg);
        }
        
        try {
            executeBlock(function.body.get(), funcEnv);
        } catch (const ReturnException& returnValue) {
            return returnValue.value;
        }
        
        return 0.0; // Default return value
    }
    
    bool isTruthy(const RuntimeValue& value) {
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value) != 0.0;
        }
        if (std::holds_alternative<std::string>(value)) {
            return !std::get<std::string>(value).empty();
        }
        return false;
    }
    
    bool isEqual(const RuntimeValue& a, const RuntimeValue& b) {
        if (a.index() != b.index()) return false;
        
        if (std::holds_alternative<double>(a)) {
            return std::get<double>(a) == std::get<double>(b);
        }
        if (std::holds_alternative<std::string>(a)) {
            return std::get<std::string>(a) == std::get<std::string>(b);
        }
        if (std::holds_alternative<bool>(a)) {
            return std::get<bool>(a) == std::get<bool>(b);
        }
        
        return false;
    }
    
    void checkNumberOperand(const RuntimeValue& operand) {
        if (!std::holds_alternative<double>(operand)) {
            throw std::runtime_error("Operand must be a number");
        }
    }
    
    void checkNumberOperands(const RuntimeValue& left, const RuntimeValue& right) {
        if (!std::holds_alternative<double>(left) || !std::holds_alternative<double>(right)) {
            throw std::runtime_error("Operands must be numbers");
        }
    }
    
    std::string valueToString(const RuntimeValue& value) {
        if (std::holds_alternative<double>(value)) {
            double d = std::get<double>(value);
            if (d == (int)d) {
                return std::to_string((int)d);
            }
            return std::to_string(d);
        }
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value) ? "true" : "false";
        }
        return "nil";
    }
};

// Main compiler/interpreter class
class MiniLanguage {
private:
    Lexer lexer;
    Parser parser;
    Interpreter interpreter;
    
public:
    MiniLanguage() : lexer(""), parser({}) {}
    
    void runFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << std::endl;
            return;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        
        run(source);
    }
    
    void runREPL() {
        std::cout << "Mini Language REPL v1.0" << std::endl;
        std::cout << "Type 'exit' to quit" << std::endl;
        
        std::string line;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, line);
            
            if (line == "exit") break;
            if (line.empty()) continue;
            
            run(line);
        }
    }
    
    void run(const std::string& source) {
        try {
            // Lexical analysis
            Lexer lexer(source);
            std::vector<Token> tokens = lexer.scanTokens();
            
            std::cout << "=== TOKENS ===" << std::endl;
            for (const auto& token : tokens) {
                if (token.type != TokenType::EOF_TOKEN && token.type != TokenType::NEWLINE) {
                    std::cout << token.toString() << std::endl;
                }
            }
            
            // Parsing
            Parser parser(tokens);
            auto ast = parser.parse();
            
            std::cout << "\n=== AST ===" << std::endl;
            std::cout << ast->toString() << std::endl;
            
            // Interpretation
            std::cout << "\n=== EXECUTION ===" << std::endl;
            Interpreter interpreter;
            interpreter.interpret(ast);
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void runDemo() {
        std::cout << "=== MINI LANGUAGE COMPILER/INTERPRETER DEMO ===" << std::endl;
        
        // Demo program
        std::string program = R"(
            // Variable declarations
            var x = 10;
            var y = 20;
            const PI = 3.14159;
            
            // Function definition
            function add(a, b) {
                return a + b;
            }
            
            function factorial(n) {
                if (n <= 1) {
                    return 1;
                } else {
                    return n * factorial(n - 1);
                }
            }
            
            // Main program
            print "Hello, World!";
            print "x + y = " + add(x, y);
            print "PI = " + PI;
            
            // Conditional statement
            if (x > y) {
                print "x is greater than y";
            } else {
                print "y is greater than or equal to x";
            }
            
            // Loop
            var i = 1;
            while (i <= 5) {
                print "Factorial of " + i + " = " + factorial(i);
                i = i + 1;
            }
            
            // String operations
            var name = "Mini Language";
            print "Welcome to " + name + "!";
        )";
        
        std::cout << "Running demo program:" << std::endl;
        std::cout << program << std::endl;
        std::cout << "\n" << std::string(50, '=') << std::endl;
        
        run(program);
    }
};

// Demo function
void runCompilerDemo() {
    MiniLanguage language;
    language.runDemo();
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Starting REPL (type 'exit' to quit):" << std::endl;
    
    // Uncomment to enable REPL
    // language.runREPL();
}

int main() {
    try {
        runCompilerDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
