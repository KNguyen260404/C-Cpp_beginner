#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <limits>
#include <fstream>
#include <map>

using namespace std;

// Forward declarations
class MenuItem;
class Order;
class Table;
class Restaurant;
class OrderItem;

// Class to represent a menu item
class MenuItem {
private:
    int id;
    string name;
    string category;
    double price;
    bool available;

public:
    MenuItem(int i, string n, string c, double p)
        : id(i), name(n), category(c), price(p), available(true) {}

    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    string getCategory() const { return category; }
    double getPrice() const { return price; }
    bool isAvailable() const { return available; }

    // Setters
    void setPrice(double p) { price = p; }
    void setAvailable(bool a) { available = a; }

    // Display the menu item
    void display() const {
        cout << left << setw(4) << id
             << setw(25) << name
             << setw(15) << category
             << right << setw(8) << fixed << setprecision(2) << price
             << setw(12) << (available ? "Available" : "Unavailable") << endl;
    }
};

// Class to represent an order item (menu item + quantity)
class OrderItem {
private:
    MenuItem* menuItem;
    int quantity;

public:
    OrderItem(MenuItem* item, int qty) : menuItem(item), quantity(qty) {}

    // Getters
    MenuItem* getMenuItem() const { return menuItem; }
    int getQuantity() const { return quantity; }

    // Setters
    void setQuantity(int qty) { quantity = qty; }

    // Calculate the total price for this order item
    double getTotalPrice() const {
        return menuItem->getPrice() * quantity;
    }

    // Display the order item
    void display() const {
        cout << left << setw(25) << menuItem->getName()
             << right << setw(8) << quantity
             << setw(10) << fixed << setprecision(2) << menuItem->getPrice()
             << setw(10) << fixed << setprecision(2) << getTotalPrice() << endl;
    }
};

// Class to represent an order
class Order {
private:
    static int nextOrderId;
    int id;
    vector<OrderItem> items;
    int tableNumber;
    string status; // "Pending", "Preparing", "Ready", "Delivered", "Paid"
    time_t orderTime;
    time_t deliveryTime;

public:
    Order(int tableNum) : tableNumber(tableNum), status("Pending") {
        id = nextOrderId++;
        orderTime = time(nullptr);
        deliveryTime = 0;
    }

    // Copy constructor
    Order(const Order& other) : id(other.id), tableNumber(other.tableNumber), 
                               status(other.status), orderTime(other.orderTime),
                               deliveryTime(other.deliveryTime) {
        for (const auto& item : other.items) {
            items.push_back(item);
        }
    }

    // Getters
    int getId() const { return id; }
    int getTableNumber() const { return tableNumber; }
    string getStatus() const { return status; }
    time_t getOrderTime() const { return orderTime; }
    time_t getDeliveryTime() const { return deliveryTime; }
    const vector<OrderItem>& getItems() const { return items; }

    // Setters
    void setStatus(const string& s) { 
        status = s; 
        if (s == "Delivered") {
            deliveryTime = time(nullptr);
        }
    }

    // Add an item to the order
    void addItem(MenuItem* menuItem, int quantity) {
        // Check if the item already exists in the order
        for (auto& item : items) {
            if (item.getMenuItem()->getId() == menuItem->getId()) {
                item.setQuantity(item.getQuantity() + quantity);
                return;
            }
        }
        
        // If not, add a new order item
        items.push_back(OrderItem(menuItem, quantity));
    }

    // Remove an item from the order
    bool removeItem(int menuItemId) {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->getMenuItem()->getId() == menuItemId) {
                items.erase(it);
                return true;
            }
        }
        return false;
    }

    // Update the quantity of an item in the order
    bool updateItemQuantity(int menuItemId, int newQuantity) {
        if (newQuantity <= 0) {
            return removeItem(menuItemId);
        }

        for (auto& item : items) {
            if (item.getMenuItem()->getId() == menuItemId) {
                item.setQuantity(newQuantity);
                return true;
            }
        }
        return false;
    }

    // Calculate the total price of the order
    double getTotalPrice() const {
        double total = 0.0;
        for (const auto& item : items) {
            total += item.getTotalPrice();
        }
        return total;
    }

    // Check if the order is empty
    bool isEmpty() const {
        return items.empty();
    }

    // Get the number of items in the order
    size_t getItemCount() const {
        return items.size();
    }

    // Display the order
    void display() const {
        cout << "Order #" << id << " - Table " << tableNumber << endl;
        cout << "Status: " << status << endl;
        
        // Format and display order time
        char orderTimeStr[26];
        strcpy(orderTimeStr, ctime(&orderTime));
        cout << "Order Time: " << orderTimeStr;
        
        // Format and display delivery time if applicable
        if (deliveryTime != 0) {
            char deliveryTimeStr[26];
            strcpy(deliveryTimeStr, ctime(&deliveryTime));
            cout << "Delivery Time: " << deliveryTimeStr;
        }
        
        cout << "Items:" << endl;
        cout << left << setw(25) << "Name"
             << right << setw(8) << "Quantity"
             << setw(10) << "Price"
             << setw(10) << "Total" << endl;
        cout << string(53, '-') << endl;
        
        for (const auto& item : items) {
            item.display();
        }
        
        cout << string(53, '-') << endl;
        cout << "Total: $" << fixed << setprecision(2) << getTotalPrice() << endl;
    }
};

// Initialize static member
int Order::nextOrderId = 1001;

// Class to represent a table in the restaurant
class Table {
private:
    int number;
    int capacity;
    bool occupied;
    Order* currentOrder;

public:
    Table(int num, int cap) : number(num), capacity(cap), occupied(false), currentOrder(nullptr) {}

    // Getters
    int getNumber() const { return number; }
    int getCapacity() const { return capacity; }
    bool isOccupied() const { return occupied; }
    Order* getCurrentOrder() const { return currentOrder; }

    // Occupy the table
    void occupy() {
        occupied = true;
        if (!currentOrder) {
            currentOrder = new Order(number);
        }
    }

    // Free the table
    void free() {
        occupied = false;
        delete currentOrder;
        currentOrder = nullptr;
    }

    // Display table information
    void display() const {
        cout << "Table " << number 
             << " (Capacity: " << capacity << "): " 
             << (occupied ? "Occupied" : "Free");
        
        if (occupied && currentOrder) {
            cout << " - Order #" << currentOrder->getId()
                 << " - Items: " << currentOrder->getItemCount()
                 << " - Status: " << currentOrder->getStatus();
        }
        
        cout << endl;
    }

    // Destructor
    ~Table() {
        delete currentOrder;
    }
};

// Class to represent the restaurant
class Restaurant {
private:
    string name;
    vector<MenuItem> menu;
    vector<Table> tables;
    vector<Order*> completedOrders;

    // Find a menu item by ID
    MenuItem* findMenuItem(int id) {
        for (auto& item : menu) {
            if (item.getId() == id) {
                return &item;
            }
        }
        return nullptr;
    }

    // Find a table by number
    Table* findTable(int number) {
        for (auto& table : tables) {
            if (table.getNumber() == number) {
                return &table;
            }
        }
        return nullptr;
    }

public:
    Restaurant(const string& n) : name(n) {}

    // Initialize the restaurant with sample data
    void initialize() {
        // Add sample menu items
        menu.push_back(MenuItem(1, "Hamburger", "Main Course", 8.99));
        menu.push_back(MenuItem(2, "Cheeseburger", "Main Course", 9.99));
        menu.push_back(MenuItem(3, "French Fries", "Side", 3.99));
        menu.push_back(MenuItem(4, "Onion Rings", "Side", 4.99));
        menu.push_back(MenuItem(5, "Cola", "Beverage", 1.99));
        menu.push_back(MenuItem(6, "Iced Tea", "Beverage", 1.99));
        menu.push_back(MenuItem(7, "Chocolate Cake", "Dessert", 5.99));
        menu.push_back(MenuItem(8, "Ice Cream", "Dessert", 4.99));
        menu.push_back(MenuItem(9, "Caesar Salad", "Appetizer", 6.99));
        menu.push_back(MenuItem(10, "Chicken Wings", "Appetizer", 7.99));

        // Add tables
        for (int i = 1; i <= 10; ++i) {
            int capacity = (i <= 5) ? 4 : ((i <= 8) ? 6 : 8); // Tables 1-5: 4 seats, 6-8: 6 seats, 9-10: 8 seats
            tables.push_back(Table(i, capacity));
        }
    }

    // Display the menu
    void displayMenu() const {
        cout << "\n===== " << name << " Menu =====" << endl;
        cout << left << setw(4) << "ID"
             << setw(25) << "Name"
             << setw(15) << "Category"
             << right << setw(8) << "Price"
             << setw(12) << "Status" << endl;
        cout << string(64, '-') << endl;
        
        for (const auto& item : menu) {
            item.display();
        }
    }

    // Display all tables
    void displayTables() const {
        cout << "\n===== Tables =====" << endl;
        for (const auto& table : tables) {
            table.display();
        }
    }

    // Occupy a table
    bool occupyTable(int tableNumber) {
        Table* table = findTable(tableNumber);
        if (!table) {
            cout << "Table " << tableNumber << " does not exist." << endl;
            return false;
        }
        
        if (table->isOccupied()) {
            cout << "Table " << tableNumber << " is already occupied." << endl;
            return false;
        }
        
        table->occupy();
        cout << "Table " << tableNumber << " is now occupied." << endl;
        return true;
    }

    // Free a table and save the order
    bool freeTable(int tableNumber) {
        Table* table = findTable(tableNumber);
        if (!table) {
            cout << "Table " << tableNumber << " does not exist." << endl;
            return false;
        }
        
        if (!table->isOccupied()) {
            cout << "Table " << tableNumber << " is not occupied." << endl;
            return false;
        }
        
        Order* order = table->getCurrentOrder();
        if (order && !order->isEmpty() && order->getStatus() == "Paid") {
            // Save a copy of the completed order
            Order* completedOrder = new Order(*order);
            completedOrders.push_back(completedOrder);
        }
        
        table->free();
        cout << "Table " << tableNumber << " is now free." << endl;
        return true;
    }

    // Take an order for a table
    bool takeOrder(int tableNumber) {
        Table* table = findTable(tableNumber);
        if (!table) {
            cout << "Table " << tableNumber << " does not exist." << endl;
            return false;
        }
        
        if (!table->isOccupied()) {
            cout << "Table " << tableNumber << " is not occupied." << endl;
            return false;
        }
        
        Order* order = table->getCurrentOrder();
        if (!order) {
            cout << "Error: No order found for table " << tableNumber << endl;
            return false;
        }
        
        displayMenu();
        
        while (true) {
            int menuItemId;
            int quantity;
            
            cout << "\nEnter menu item ID (0 to finish): ";
            cin >> menuItemId;
            
            if (menuItemId == 0) {
                break;
            }
            
            MenuItem* menuItem = findMenuItem(menuItemId);
            if (!menuItem) {
                cout << "Invalid menu item ID." << endl;
                continue;
            }
            
            if (!menuItem->isAvailable()) {
                cout << "Sorry, " << menuItem->getName() << " is currently unavailable." << endl;
                continue;
            }
            
            cout << "Enter quantity: ";
            cin >> quantity;
            
            if (quantity <= 0) {
                cout << "Quantity must be positive." << endl;
                continue;
            }
            
            order->addItem(menuItem, quantity);
            cout << quantity << " " << menuItem->getName() << "(s) added to the order." << endl;
        }
        
        if (order->isEmpty()) {
            cout << "No items added to the order." << endl;
            return false;
        }
        
        order->setStatus("Pending");
        cout << "Order placed successfully." << endl;
        return true;
    }

    // Update the status of an order
    bool updateOrderStatus(int tableNumber, const string& newStatus) {
        Table* table = findTable(tableNumber);
        if (!table || !table->isOccupied() || !table->getCurrentOrder()) {
            cout << "No active order found for table " << tableNumber << endl;
            return false;
        }
        
        Order* order = table->getCurrentOrder();
        order->setStatus(newStatus);
        cout << "Order status updated to: " << newStatus << endl;
        return true;
    }

    // Display an order for a specific table
    bool displayOrder(int tableNumber) {
        Table* table = findTable(tableNumber);
        if (!table || !table->isOccupied() || !table->getCurrentOrder()) {
            cout << "No active order found for table " << tableNumber << endl;
            return false;
        }
        
        Order* order = table->getCurrentOrder();
        if (order->isEmpty()) {
            cout << "Order is empty." << endl;
            return false;
        }
        
        cout << "\n===== Order Details =====" << endl;
        order->display();
        return true;
    }

    // Process payment for an order
    bool processPayment(int tableNumber) {
        Table* table = findTable(tableNumber);
        if (!table || !table->isOccupied() || !table->getCurrentOrder()) {
            cout << "No active order found for table " << tableNumber << endl;
            return false;
        }
        
        Order* order = table->getCurrentOrder();
        if (order->isEmpty()) {
            cout << "Order is empty." << endl;
            return false;
        }
        
        cout << "\n===== Payment Processing =====" << endl;
        order->display();
        
        double totalAmount = order->getTotalPrice();
        double payment;
        
        cout << "Total amount to pay: $" << fixed << setprecision(2) << totalAmount << endl;
        cout << "Enter payment amount: $";
        cin >> payment;
        
        if (payment < totalAmount) {
            cout << "Insufficient payment." << endl;
            return false;
        }
        
        double change = payment - totalAmount;
        cout << "Payment received: $" << fixed << setprecision(2) << payment << endl;
        cout << "Change: $" << fixed << setprecision(2) << change << endl;
        
        order->setStatus("Paid");
        cout << "Payment processed successfully." << endl;
        return true;
    }

    // Update a menu item
    bool updateMenuItem(int menuItemId) {
        MenuItem* item = findMenuItem(menuItemId);
        if (!item) {
            cout << "Invalid menu item ID." << endl;
            return false;
        }
        
        cout << "\nCurrent details:" << endl;
        item->display();
        
        int choice;
        cout << "\n1. Update price" << endl;
        cout << "2. Update availability" << endl;
        cout << "Enter choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1: {
                double newPrice;
                cout << "Enter new price: $";
                cin >> newPrice;
                
                if (newPrice <= 0) {
                    cout << "Price must be positive." << endl;
                    return false;
                }
                
                item->setPrice(newPrice);
                cout << "Price updated successfully." << endl;
                break;
            }
            case 2: {
                char availability;
                cout << "Is the item available? (y/n): ";
                cin >> availability;
                
                item->setAvailable(tolower(availability) == 'y');
                cout << "Availability updated successfully." << endl;
                break;
            }
            default:
                cout << "Invalid choice." << endl;
                return false;
        }
        
        return true;
    }

    // Generate a sales report
    void generateSalesReport() const {
        cout << "\n===== Sales Report =====" << endl;
        
        if (completedOrders.empty()) {
            cout << "No completed orders to report." << endl;
            return;
        }
        
        double totalSales = 0.0;
        map<int, int> itemSales; // Menu item ID -> quantity sold
        
        for (const auto& order : completedOrders) {
            if (order->getStatus() == "Paid") {
                totalSales += order->getTotalPrice();
                
                // Count items sold
                for (const auto& item : order->getItems()) {
                    int menuItemId = item.getMenuItem()->getId();
                    itemSales[menuItemId] += item.getQuantity();
                }
            }
        }
        
        cout << "Total Sales: $" << fixed << setprecision(2) << totalSales << endl;
        cout << "Number of Completed Orders: " << completedOrders.size() << endl;
        
        cout << "\nTop Selling Items:" << endl;
        cout << left << setw(25) << "Item"
             << right << setw(10) << "Quantity"
             << setw(10) << "Revenue" << endl;
        cout << string(45, '-') << endl;
        
        // Sort items by quantity sold
        vector<pair<int, int>> sortedItems(itemSales.begin(), itemSales.end());
        sort(sortedItems.begin(), sortedItems.end(), 
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 return a.second > b.second;
             });
        
        // Display top items
        for (const auto& item : sortedItems) {
            int menuItemId = item.first;
            int quantitySold = item.second;
            
            const MenuItem* menuItem = nullptr;
            for (const auto& mi : menu) {
                if (mi.getId() == menuItemId) {
                    menuItem = &mi;
                    break;
                }
            }
            
            if (menuItem) {
                double revenue = menuItem->getPrice() * quantitySold;
                cout << left << setw(25) << menuItem->getName()
                     << right << setw(10) << quantitySold
                     << setw(10) << fixed << setprecision(2) << revenue << endl;
            }
        }
    }

    // Save data to file
    bool saveToFile(const string& filename) const {
        ofstream file(filename);
        if (!file) {
            cout << "Error opening file for writing." << endl;
            return false;
        }
        
        // Save menu items
        file << menu.size() << endl;
        for (const auto& item : menu) {
            file << item.getId() << "|"
                 << item.getName() << "|"
                 << item.getCategory() << "|"
                 << item.getPrice() << "|"
                 << item.isAvailable() << endl;
        }
        
        // Save tables
        file << tables.size() << endl;
        for (const auto& table : tables) {
            file << table.getNumber() << "|"
                 << table.getCapacity() << "|"
                 << table.isOccupied() << endl;
        }
        
        file.close();
        cout << "Data saved to " << filename << endl;
        return true;
    }

    // Load data from file
    bool loadFromFile(const string& filename) {
        ifstream file(filename);
        if (!file) {
            cout << "File not found. Using default initialization." << endl;
            return false;
        }
        
        // Clear existing data
        menu.clear();
        tables.clear();
        
        // Load menu items
        size_t menuSize;
        file >> menuSize;
        file.ignore(); // Ignore newline
        
        for (size_t i = 0; i < menuSize; ++i) {
            string line;
            getline(file, line);
            
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            size_t pos3 = line.find('|', pos2 + 1);
            size_t pos4 = line.find('|', pos3 + 1);
            
            if (pos1 == string::npos || pos2 == string::npos || 
                pos3 == string::npos || pos4 == string::npos) {
                continue; // Skip invalid line
            }
            
            int id = stoi(line.substr(0, pos1));
            string name = line.substr(pos1 + 1, pos2 - pos1 - 1);
            string category = line.substr(pos2 + 1, pos3 - pos2 - 1);
            double price = stod(line.substr(pos3 + 1, pos4 - pos3 - 1));
            bool available = (line.substr(pos4 + 1) == "1");
            
            MenuItem item(id, name, category, price);
            item.setAvailable(available);
            menu.push_back(item);
        }
        
        // Load tables
        size_t tableSize;
        file >> tableSize;
        file.ignore(); // Ignore newline
        
        for (size_t i = 0; i < tableSize; ++i) {
            string line;
            getline(file, line);
            
            size_t pos1 = line.find('|');
            size_t pos2 = line.find('|', pos1 + 1);
            
            if (pos1 == string::npos || pos2 == string::npos) {
                continue; // Skip invalid line
            }
            
            int number = stoi(line.substr(0, pos1));
            int capacity = stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
            bool occupied = (line.substr(pos2 + 1) == "1");
            
            Table table(number, capacity);
            if (occupied) {
                table.occupy();
            }
            tables.push_back(table);
        }
        
        file.close();
        cout << "Data loaded from " << filename << endl;
        return true;
    }

    // Destructor
    ~Restaurant() {
        // Clean up completed orders
        for (auto order : completedOrders) {
            delete order;
        }
    }
};

// Function to clear the console screen
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Function to display the main menu
void displayMainMenu() {
    cout << "\n===== Restaurant Management System =====" << endl;
    cout << "1. View Tables" << endl;
    cout << "2. View Menu" << endl;
    cout << "3. Occupy Table" << endl;
    cout << "4. Free Table" << endl;
    cout << "5. Take Order" << endl;
    cout << "6. View Order" << endl;
    cout << "7. Update Order Status" << endl;
    cout << "8. Process Payment" << endl;
    cout << "9. Update Menu Item" << endl;
    cout << "10. Generate Sales Report" << endl;
    cout << "11. Save Data" << endl;
    cout << "12. Load Data" << endl;
    cout << "0. Exit" << endl;
    cout << "=======================================" << endl;
    cout << "Enter your choice: ";
}

// Main function
int main() {
    Restaurant restaurant("Delicious Bites");
    restaurant.initialize();
    
    int choice;
    
    cout << "Welcome to Restaurant Management System!" << endl;
    
    while (true) {
        displayMainMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: // View Tables
                clearScreen();
                restaurant.displayTables();
                break;
                
            case 2: // View Menu
                clearScreen();
                restaurant.displayMenu();
                break;
                
            case 3: { // Occupy Table
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number to occupy: ";
                cin >> tableNumber;
                restaurant.occupyTable(tableNumber);
                break;
            }
                
            case 4: { // Free Table
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number to free: ";
                cin >> tableNumber;
                restaurant.freeTable(tableNumber);
                break;
            }
                
            case 5: { // Take Order
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number to take order: ";
                cin >> tableNumber;
                restaurant.takeOrder(tableNumber);
                break;
            }
                
            case 6: { // View Order
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number to view order: ";
                cin >> tableNumber;
                restaurant.displayOrder(tableNumber);
                break;
            }
                
            case 7: { // Update Order Status
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number: ";
                cin >> tableNumber;
                
                cout << "Select new status:" << endl;
                cout << "1. Pending" << endl;
                cout << "2. Preparing" << endl;
                cout << "3. Ready" << endl;
                cout << "4. Delivered" << endl;
                
                int statusChoice;
                cout << "Enter choice: ";
                cin >> statusChoice;
                
                string newStatus;
                switch (statusChoice) {
                    case 1: newStatus = "Pending"; break;
                    case 2: newStatus = "Preparing"; break;
                    case 3: newStatus = "Ready"; break;
                    case 4: newStatus = "Delivered"; break;
                    default: 
                        cout << "Invalid status choice." << endl;
                        continue;
                }
                
                restaurant.updateOrderStatus(tableNumber, newStatus);
                break;
            }
                
            case 8: { // Process Payment
                clearScreen();
                restaurant.displayTables();
                
                int tableNumber;
                cout << "\nEnter table number to process payment: ";
                cin >> tableNumber;
                restaurant.processPayment(tableNumber);
                break;
            }
                
            case 9: { // Update Menu Item
                clearScreen();
                restaurant.displayMenu();
                
                int menuItemId;
                cout << "\nEnter menu item ID to update: ";
                cin >> menuItemId;
                restaurant.updateMenuItem(menuItemId);
                break;
            }
                
            case 10: // Generate Sales Report
                clearScreen();
                restaurant.generateSalesReport();
                break;
                
            case 11: { // Save Data
                clearScreen();
                string filename;
                cout << "Enter filename to save data (default: restaurant_data.txt): ";
                getline(cin, filename);
                
                if (filename.empty()) {
                    filename = "restaurant_data.txt";
                }
                
                restaurant.saveToFile(filename);
                break;
            }
                
            case 12: { // Load Data
                clearScreen();
                string filename;
                cout << "Enter filename to load data (default: restaurant_data.txt): ";
                getline(cin, filename);
                
                if (filename.empty()) {
                    filename = "restaurant_data.txt";
                }
                
                restaurant.loadFromFile(filename);
                break;
            }
                
            case 0: // Exit
                cout << "Thank you for using the Restaurant Management System!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 