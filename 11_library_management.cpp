#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>

using namespace std;

class Book {
private:
    string title;
    string author;
    string isbn;
    bool available;

public:
    Book(string t, string a, string i) : title(t), author(a), isbn(i), available(true) {}

    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getISBN() const { return isbn; }
    bool isAvailable() const { return available; }

    void borrowBook() { available = false; }
    void returnBook() { available = true; }

    void displayInfo() const {
        cout << "Title: " << title << endl;
        cout << "Author: " << author << endl;
        cout << "ISBN: " << isbn << endl;
        cout << "Status: " << (available ? "Available" : "Borrowed") << endl;
    }
};

class Library {
private:
    vector<Book> books;

public:
    void addBook(const Book& book) {
        books.push_back(book);
    }

    void displayAllBooks() const {
        if (books.empty()) {
            cout << "No books in the library." << endl;
            return;
        }

        cout << "\n===== Library Books =====" << endl;
        for (size_t i = 0; i < books.size(); i++) {
            cout << "\nBook " << (i + 1) << ":" << endl;
            books[i].displayInfo();
        }
    }

    bool borrowBook(const string& isbn) {
        for (auto& book : books) {
            if (book.getISBN() == isbn && book.isAvailable()) {
                book.borrowBook();
                cout << "Book with ISBN " << isbn << " has been borrowed successfully." << endl;
                return true;
            }
        }
        cout << "Book with ISBN " << isbn << " is not available or doesn't exist." << endl;
        return false;
    }

    bool returnBook(const string& isbn) {
        for (auto& book : books) {
            if (book.getISBN() == isbn && !book.isAvailable()) {
                book.returnBook();
                cout << "Book with ISBN " << isbn << " has been returned successfully." << endl;
                return true;
            }
        }
        cout << "Book with ISBN " << isbn << " is either already available or doesn't exist." << endl;
        return false;
    }

    void searchByTitle(const string& title) {
        bool found = false;
        cout << "\n===== Search Results =====" << endl;
        
        for (const auto& book : books) {
            if (book.getTitle().find(title) != string::npos) {
                book.displayInfo();
                cout << endl;
                found = true;
            }
        }
        
        if (!found) {
            cout << "No books found with title containing '" << title << "'." << endl;
        }
    }
};

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    Library library;
    int choice;
    string title, author, isbn;

    // Add some sample books
    library.addBook(Book("The Great Gatsby", "F. Scott Fitzgerald", "9780743273565"));
    library.addBook(Book("To Kill a Mockingbird", "Harper Lee", "9780061120084"));
    library.addBook(Book("1984", "George Orwell", "9780451524935"));

    while (true) {
        cout << "\n===== Library Management System =====" << endl;
        cout << "1. Display all books" << endl;
        cout << "2. Add a new book" << endl;
        cout << "3. Borrow a book" << endl;
        cout << "4. Return a book" << endl;
        cout << "5. Search books by title" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number." << endl;
            clearInputBuffer();
            continue;
        }

        clearInputBuffer();

        switch (choice) {
            case 1:
                clearScreen();
                library.displayAllBooks();
                break;
                
            case 2:
                clearScreen();
                cout << "Enter book title: ";
                getline(cin, title);
                
                cout << "Enter author name: ";
                getline(cin, author);
                
                cout << "Enter ISBN: ";
                getline(cin, isbn);
                
                library.addBook(Book(title, author, isbn));
                cout << "Book added successfully!" << endl;
                break;
                
            case 3:
                clearScreen();
                cout << "Enter ISBN of the book to borrow: ";
                getline(cin, isbn);
                library.borrowBook(isbn);
                break;
                
            case 4:
                clearScreen();
                cout << "Enter ISBN of the book to return: ";
                getline(cin, isbn);
                library.returnBook(isbn);
                break;
                
            case 5:
                clearScreen();
                cout << "Enter title to search: ";
                getline(cin, title);
                library.searchByTitle(title);
                break;
                
            case 6:
                cout << "Thank you for using the Library Management System!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
} 