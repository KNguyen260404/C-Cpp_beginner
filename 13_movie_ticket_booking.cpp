#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <ctime>

using namespace std;

class Movie {
private:
    string title;
    string genre;
    int duration; // in minutes
    string rating;

public:
    Movie(string t, string g, int d, string r)
        : title(t), genre(g), duration(d), rating(r) {}

    string getTitle() const { return title; }
    string getGenre() const { return genre; }
    int getDuration() const { return duration; }
    string getRating() const { return rating; }

    void displayInfo() const {
        cout << "Title: " << title << endl;
        cout << "Genre: " << genre << endl;
        cout << "Duration: " << duration << " minutes" << endl;
        cout << "Rating: " << rating << endl;
    }
};

class ShowTime {
private:
    Movie movie;
    string time;
    string date;
    double price;

public:
    ShowTime(Movie m, string t, string d, double p)
        : movie(m), time(t), date(d), price(p) {}

    Movie getMovie() const { return movie; }
    string getTime() const { return time; }
    string getDate() const { return date; }
    double getPrice() const { return price; }

    void displayInfo() const {
        cout << "Movie: " << movie.getTitle() << endl;
        cout << "Date: " << date << endl;
        cout << "Time: " << time << endl;
        cout << "Price: $" << fixed << setprecision(2) << price << endl;
    }
};

class Seat {
private:
    int row;
    int column;
    bool isBooked;

public:
    Seat(int r, int c) : row(r), column(c), isBooked(false) {}

    int getRow() const { return row; }
    int getColumn() const { return column; }
    bool getIsBooked() const { return isBooked; }
    void book() { isBooked = true; }
    void unbook() { isBooked = false; }

    string getSeatId() const {
        return string(1, 'A' + row) + to_string(column + 1);
    }
};

class Theater {
private:
    string name;
    vector<vector<Seat>> seats;
    int rows;
    int columns;

public:
    Theater(string n, int r, int c) : name(n), rows(r), columns(c) {
        // Initialize seats
        seats.resize(rows);
        for (int i = 0; i < rows; i++) {
            seats[i].resize(columns);
            for (int j = 0; j < columns; j++) {
                seats[i][j] = Seat(i, j);
            }
        }
    }

    string getName() const { return name; }
    int getRows() const { return rows; }
    int getColumns() const { return columns; }

    bool bookSeat(int row, int column) {
        if (row < 0 || row >= rows || column < 0 || column >= columns) {
            return false;
        }
        
        if (seats[row][column].getIsBooked()) {
            return false;
        }
        
        seats[row][column].book();
        return true;
    }

    void displaySeatingPlan() const {
        cout << "\n  Seating Plan for " << name << "\n" << endl;
        
        // Display column numbers
        cout << "    ";
        for (int j = 0; j < columns; j++) {
            cout << setw(3) << j + 1;
        }
        cout << endl;

        // Display seats
        for (int i = 0; i < rows; i++) {
            cout << " " << char('A' + i) << "  ";
            
            for (int j = 0; j < columns; j++) {
                if (seats[i][j].getIsBooked()) {
                    cout << setw(3) << "X";
                } else {
                    cout << setw(3) << "O";
                }
            }
            
            cout << endl;
        }
        
        cout << "\nLegend: O - Available, X - Booked" << endl;
    }

    Seat* getSeat(int row, int column) {
        if (row < 0 || row >= rows || column < 0 || column >= columns) {
            return nullptr;
        }
        return &seats[row][column];
    }
};

class Ticket {
private:
    static int nextId;
    int id;
    ShowTime showtime;
    Seat seat;
    string customerName;
    string purchaseDate;

public:
    Ticket(ShowTime st, Seat s, string name)
        : id(++nextId), showtime(st), seat(s), customerName(name) {
        // Get current date for purchase date
        time_t now = time(0);
        tm* ltm = localtime(&now);
        purchaseDate = to_string(1900 + ltm->tm_year) + "-" +
                      to_string(1 + ltm->tm_mon) + "-" +
                      to_string(ltm->tm_mday);
    }

    int getId() const { return id; }
    ShowTime getShowtime() const { return showtime; }
    Seat getSeat() const { return seat; }
    string getCustomerName() const { return customerName; }
    string getPurchaseDate() const { return purchaseDate; }

    void displayTicket() const {
        cout << "\n===================================" << endl;
        cout << "           MOVIE TICKET            " << endl;
        cout << "===================================" << endl;
        cout << "Ticket ID: " << id << endl;
        cout << "Movie: " << showtime.getMovie().getTitle() << endl;
        cout << "Date: " << showtime.getDate() << endl;
        cout << "Time: " << showtime.getTime() << endl;
        cout << "Theater: Main Theater" << endl;
        cout << "Seat: " << seat.getSeatId() << endl;
        cout << "Customer: " << customerName << endl;
        cout << "Price: $" << fixed << setprecision(2) << showtime.getPrice() << endl;
        cout << "Purchase Date: " << purchaseDate << endl;
        cout << "===================================" << endl;
    }
};

int Ticket::nextId = 1000;

class BookingSystem {
private:
    vector<Movie> movies;
    vector<ShowTime> showtimes;
    vector<Theater> theaters;
    vector<Ticket> tickets;

public:
    BookingSystem() {
        // Initialize with some sample data
        initSampleData();
    }

    void initSampleData() {
        // Add sample movies
        movies.push_back(Movie("Avengers: Endgame", "Action/Adventure", 181, "PG-13"));
        movies.push_back(Movie("The Shawshank Redemption", "Drama", 142, "R"));
        movies.push_back(Movie("Toy Story 4", "Animation/Adventure", 100, "G"));
        
        // Add sample theaters
        theaters.push_back(Theater("Main Theater", 8, 10));
        
        // Add sample showtimes
        showtimes.push_back(ShowTime(movies[0], "18:00", "2023-07-15", 12.99));
        showtimes.push_back(ShowTime(movies[0], "21:00", "2023-07-15", 12.99));
        showtimes.push_back(ShowTime(movies[1], "19:00", "2023-07-15", 10.99));
        showtimes.push_back(ShowTime(movies[2], "17:00", "2023-07-15", 9.99));
    }

    void displayMovies() const {
        cout << "\n===== Available Movies =====" << endl;
        for (size_t i = 0; i < movies.size(); i++) {
            cout << i + 1 << ". " << movies[i].getTitle() << " (" << movies[i].getDuration() 
                 << " min, " << movies[i].getRating() << ")" << endl;
        }
    }

    void displayShowtimes() const {
        cout << "\n===== Available Showtimes =====" << endl;
        for (size_t i = 0; i < showtimes.size(); i++) {
            cout << i + 1 << ". " << showtimes[i].getMovie().getTitle() 
                 << " - " << showtimes[i].getDate() 
                 << " at " << showtimes[i].getTime() 
                 << " ($" << fixed << setprecision(2) << showtimes[i].getPrice() << ")" << endl;
        }
    }

    bool bookTicket() {
        displayShowtimes();
        
        int showtimeChoice;
        cout << "\nSelect a showtime (1-" << showtimes.size() << "): ";
        cin >> showtimeChoice;
        
        if (showtimeChoice < 1 || showtimeChoice > static_cast<int>(showtimes.size())) {
            cout << "Invalid showtime selection." << endl;
            return false;
        }
        
        ShowTime selectedShowtime = showtimes[showtimeChoice - 1];
        
        // For simplicity, we'll use the first theater for all showtimes
        Theater& theater = theaters[0];
        theater.displaySeatingPlan();
        
        char rowChar;
        int column;
        
        cout << "\nSelect a seat (e.g., A5): ";
        cin >> rowChar >> column;
        
        int row = toupper(rowChar) - 'A';
        column--; // Convert to 0-based indexing
        
        if (!theater.bookSeat(row, column)) {
            cout << "Invalid seat selection or seat already booked." << endl;
            return false;
        }
        
        string customerName;
        cout << "Enter your name: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, customerName);
        
        Seat* seat = theater.getSeat(row, column);
        if (!seat) {
            cout << "Error retrieving seat information." << endl;
            return false;
        }
        
        Ticket ticket(selectedShowtime, *seat, customerName);
        tickets.push_back(ticket);
        
        cout << "\nTicket booked successfully!" << endl;
        ticket.displayTicket();
        
        return true;
    }

    void displayBookingHistory() const {
        if (tickets.empty()) {
            cout << "\nNo booking history available." << endl;
            return;
        }
        
        cout << "\n===== Booking History =====" << endl;
        for (const auto& ticket : tickets) {
            cout << "Ticket ID: " << ticket.getId() 
                 << " | Movie: " << ticket.getShowtime().getMovie().getTitle()
                 << " | Date: " << ticket.getShowtime().getDate()
                 << " | Time: " << ticket.getShowtime().getTime()
                 << " | Seat: " << ticket.getSeat().getSeatId()
                 << " | Customer: " << ticket.getCustomerName() << endl;
        }
    }

    void displayMovieDetails() const {
        displayMovies();
        
        int movieChoice;
        cout << "\nSelect a movie to view details (1-" << movies.size() << "): ";
        cin >> movieChoice;
        
        if (movieChoice < 1 || movieChoice > static_cast<int>(movies.size())) {
            cout << "Invalid movie selection." << endl;
            return;
        }
        
        cout << "\n===== Movie Details =====" << endl;
        movies[movieChoice - 1].displayInfo();
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
    BookingSystem bookingSystem;
    int choice;
    
    cout << "Welcome to Movie Ticket Booking System" << endl;
    
    while (true) {
        cout << "\n===== Main Menu =====" << endl;
        cout << "1. View Available Movies" << endl;
        cout << "2. View Movie Details" << endl;
        cout << "3. View Showtimes" << endl;
        cout << "4. Book a Ticket" << endl;
        cout << "5. View Booking History" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number." << endl;
            clearInputBuffer();
            continue;
        }
        
        switch (choice) {
            case 1:
                clearScreen();
                bookingSystem.displayMovies();
                break;
                
            case 2:
                clearScreen();
                bookingSystem.displayMovieDetails();
                break;
                
            case 3:
                clearScreen();
                bookingSystem.displayShowtimes();
                break;
                
            case 4:
                clearScreen();
                bookingSystem.bookTicket();
                break;
                
            case 5:
                clearScreen();
                bookingSystem.displayBookingHistory();
                break;
                
            case 6:
                cout << "Thank you for using the Movie Ticket Booking System!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 