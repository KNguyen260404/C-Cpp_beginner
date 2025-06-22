#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <limits>
#include <ctime>
#include <random>
#include <cctype>

using namespace std;

// Simple encryption/decryption using XOR with a key
string encrypt(const string& text, const string& key) {
    string result = text;
    for (size_t i = 0; i < text.length(); i++) {
        result[i] = text[i] ^ key[i % key.length()];
    }
    return result;
}

string decrypt(const string& text, const string& key) {
    return encrypt(text, key); // XOR encryption is symmetric
}

class PasswordEntry {
private:
    string website;
    string username;
    string password;
    string notes;
    string dateCreated;
    string dateModified;

public:
    PasswordEntry(string site, string user, string pass, string note = "") 
        : website(site), username(user), password(pass), notes(note) {
        
        // Get current date
        time_t now = time(0);
        tm* ltm = localtime(&now);
        string date = to_string(1900 + ltm->tm_year) + "-" +
                      to_string(1 + ltm->tm_mon) + "-" +
                      to_string(ltm->tm_mday);
        
        dateCreated = date;
        dateModified = date;
    }
    
    // Getters
    string getWebsite() const { return website; }
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getNotes() const { return notes; }
    string getDateCreated() const { return dateCreated; }
    string getDateModified() const { return dateModified; }
    
    // Setters
    void setWebsite(const string& site) {
        website = site;
        updateModifiedDate();
    }
    
    void setUsername(const string& user) {
        username = user;
        updateModifiedDate();
    }
    
    void setPassword(const string& pass) {
        password = pass;
        updateModifiedDate();
    }
    
    void setNotes(const string& note) {
        notes = note;
        updateModifiedDate();
    }
    
    void updateModifiedDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        dateModified = to_string(1900 + ltm->tm_year) + "-" +
                      to_string(1 + ltm->tm_mon) + "-" +
                      to_string(ltm->tm_mday);
    }
    
    void display(bool showPassword = false) const {
        cout << "Website: " << website << endl;
        cout << "Username: " << username << endl;
        
        if (showPassword) {
            cout << "Password: " << password << endl;
        } else {
            cout << "Password: " << string(password.length(), '*') << endl;
        }
        
        if (!notes.empty()) {
            cout << "Notes: " << notes << endl;
        }
        
        cout << "Created: " << dateCreated << endl;
        cout << "Modified: " << dateModified << endl;
    }
    
    string toEncryptedString(const string& key) const {
        string data = website + "|" + username + "|" + password + "|" + notes + "|" + dateCreated + "|" + dateModified;
        return encrypt(data, key);
    }
    
    static PasswordEntry fromEncryptedString(const string& encryptedData, const string& key) {
        string decrypted = decrypt(encryptedData, key);
        
        // Parse the decrypted string
        vector<string> parts;
        string part;
        for (char c : decrypted) {
            if (c == '|') {
                parts.push_back(part);
                part.clear();
            } else {
                part += c;
            }
        }
        parts.push_back(part); // Add the last part
        
        if (parts.size() < 6) {
            // Handle error case - not enough parts
            return PasswordEntry("Error", "Error", "Error", "Corrupted data");
        }
        
        PasswordEntry entry(parts[0], parts[1], parts[2], parts[3]);
        // We don't set the dates here as they're already in the object
        return entry;
    }
};

class PasswordGenerator {
public:
    static string generatePassword(int length = 12, bool includeUppercase = true, 
                                  bool includeNumbers = true, bool includeSpecial = true) {
        string charset = "abcdefghijklmnopqrstuvwxyz";
        if (includeUppercase) charset += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (includeNumbers) charset += "0123456789";
        if (includeSpecial) charset += "!@#$%^&*()-_=+[]{}|;:,.<>?";
        
        random_device rd;
        mt19937 generator(rd());
        uniform_int_distribution<int> distribution(0, charset.size() - 1);
        
        string password;
        for (int i = 0; i < length; ++i) {
            password += charset[distribution(generator)];
        }
        
        return password;
    }
    
    static int evaluatePasswordStrength(const string& password) {
        // Simple password strength evaluation (0-100)
        int score = 0;
        
        // Length contribution (up to 40 points)
        score += min(40, static_cast<int>(password.length()) * 4);
        
        // Character variety
        bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        
        for (char c : password) {
            if (islower(c)) hasLower = true;
            else if (isupper(c)) hasUpper = true;
            else if (isdigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        
        // Add points for character variety (up to 40 points)
        score += (hasLower ? 10 : 0);
        score += (hasUpper ? 10 : 0);
        score += (hasDigit ? 10 : 0);
        score += (hasSpecial ? 10 : 0);
        
        // Check for common patterns and deduct points
        // (This is a simplified version - a real password strength evaluator would be more complex)
        
        // Sequential characters
        for (size_t i = 0; i < password.length() - 2; i++) {
            if ((password[i+1] == password[i] + 1) && (password[i+2] == password[i] + 2)) {
                score -= 5;
                break;
            }
        }
        
        // Repeated characters
        for (size_t i = 0; i < password.length() - 2; i++) {
            if ((password[i] == password[i+1]) && (password[i] == password[i+2])) {
                score -= 5;
                break;
            }
        }
        
        return max(0, min(100, score));
    }
    
    static string getStrengthDescription(int strength) {
        if (strength < 20) return "Very Weak";
        if (strength < 40) return "Weak";
        if (strength < 60) return "Moderate";
        if (strength < 80) return "Strong";
        return "Very Strong";
    }
};

class PasswordManager {
private:
    vector<PasswordEntry> entries;
    string masterPassword;
    string filename;
    
public:
    PasswordManager(const string& masterPass) : masterPassword(masterPass), filename("passwords.dat") {}
    
    void addEntry(const PasswordEntry& entry) {
        entries.push_back(entry);
    }
    
    bool removeEntry(int index) {
        if (index < 0 || index >= static_cast<int>(entries.size())) {
            return false;
        }
        
        entries.erase(entries.begin() + index);
        return true;
    }
    
    PasswordEntry* getEntry(int index) {
        if (index < 0 || index >= static_cast<int>(entries.size())) {
            return nullptr;
        }
        
        return &entries[index];
    }
    
    vector<PasswordEntry> searchEntries(const string& searchTerm) {
        vector<PasswordEntry> results;
        
        string lowerSearchTerm = searchTerm;
        transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), ::tolower);
        
        for (const auto& entry : entries) {
            string lowerWebsite = entry.getWebsite();
            string lowerUsername = entry.getUsername();
            
            transform(lowerWebsite.begin(), lowerWebsite.end(), lowerWebsite.begin(), ::tolower);
            transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
            
            if (lowerWebsite.find(lowerSearchTerm) != string::npos || 
                lowerUsername.find(lowerSearchTerm) != string::npos) {
                results.push_back(entry);
            }
        }
        
        return results;
    }
    
    void displayAllEntries(bool showPasswords = false) const {
        if (entries.empty()) {
            cout << "No password entries found." << endl;
            return;
        }
        
        cout << "\n===== Password Entries =====" << endl;
        for (size_t i = 0; i < entries.size(); i++) {
            cout << "\nEntry " << (i + 1) << ":" << endl;
            entries[i].display(showPasswords);
            cout << "-------------------------" << endl;
        }
    }
    
    bool saveToFile() {
        ofstream file(filename, ios::binary);
        if (!file) {
            return false;
        }
        
        // Write the number of entries
        size_t numEntries = entries.size();
        file.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));
        
        // Write each entry
        for (const auto& entry : entries) {
            string encryptedData = entry.toEncryptedString(masterPassword);
            size_t dataSize = encryptedData.size();
            
            file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            file.write(encryptedData.c_str(), dataSize);
        }
        
        return true;
    }
    
    bool loadFromFile() {
        ifstream file(filename, ios::binary);
        if (!file) {
            return false;
        }
        
        entries.clear();
        
        // Read the number of entries
        size_t numEntries;
        file.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));
        
        // Read each entry
        for (size_t i = 0; i < numEntries; i++) {
            size_t dataSize;
            file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
            
            string encryptedData(dataSize, ' ');
            file.read(&encryptedData[0], dataSize);
            
            PasswordEntry entry = PasswordEntry::fromEncryptedString(encryptedData, masterPassword);
            entries.push_back(entry);
        }
        
        return true;
    }
    
    size_t getEntryCount() const {
        return entries.size();
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
    string masterPassword;
    bool isFirstRun = true;
    
    // Check if the password file exists
    ifstream testFile("passwords.dat");
    if (testFile.good()) {
        isFirstRun = false;
    }
    testFile.close();
    
    if (isFirstRun) {
        cout << "Welcome to Password Manager!" << endl;
        cout << "This appears to be your first time running the application." << endl;
        cout << "Please create a master password: ";
        getline(cin, masterPassword);
        
        // In a real application, you'd want to confirm the password and validate its strength
        cout << "Master password created successfully!" << endl;
    } else {
        cout << "Welcome back to Password Manager!" << endl;
        cout << "Please enter your master password: ";
        getline(cin, masterPassword);
        
        // In a real application, you'd verify the password here
    }
    
    PasswordManager manager(masterPassword);
    
    // Try to load existing passwords
    if (!isFirstRun) {
        if (!manager.loadFromFile()) {
            cout << "Error loading password file or incorrect master password." << endl;
            return 1;
        }
    }
    
    int choice;
    while (true) {
        cout << "\n===== Password Manager =====" << endl;
        cout << "1. View All Passwords" << endl;
        cout << "2. Add New Password" << endl;
        cout << "3. Edit Password" << endl;
        cout << "4. Delete Password" << endl;
        cout << "5. Search Passwords" << endl;
        cout << "6. Generate Secure Password" << endl;
        cout << "7. Save and Exit" << endl;
        cout << "Enter your choice: ";
        
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number." << endl;
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        switch (choice) {
            case 1: {
                clearScreen();
                char showPass;
                cout << "Show passwords? (y/n): ";
                cin >> showPass;
                clearInputBuffer();
                
                manager.displayAllEntries(tolower(showPass) == 'y');
                break;
            }
            
            case 2: {
                clearScreen();
                string website, username, password, notes;
                
                cout << "Enter website: ";
                getline(cin, website);
                
                cout << "Enter username: ";
                getline(cin, username);
                
                char genPass;
                cout << "Generate a secure password? (y/n): ";
                cin >> genPass;
                clearInputBuffer();
                
                if (tolower(genPass) == 'y') {
                    int length;
                    bool upper, numbers, special;
                    
                    cout << "Password length (8-30): ";
                    cin >> length;
                    length = max(8, min(30, length));
                    
                    cout << "Include uppercase letters? (y/n): ";
                    cin >> genPass;
                    upper = (tolower(genPass) == 'y');
                    
                    cout << "Include numbers? (y/n): ";
                    cin >> genPass;
                    numbers = (tolower(genPass) == 'y');
                    
                    cout << "Include special characters? (y/n): ";
                    cin >> genPass;
                    special = (tolower(genPass) == 'y');
                    
                    clearInputBuffer();
                    
                    password = PasswordGenerator::generatePassword(length, upper, numbers, special);
                    cout << "Generated password: " << password << endl;
                    
                    int strength = PasswordGenerator::evaluatePasswordStrength(password);
                    cout << "Password strength: " << strength << "/100 (" 
                         << PasswordGenerator::getStrengthDescription(strength) << ")" << endl;
                } else {
                    cout << "Enter password: ";
                    getline(cin, password);
                    
                    int strength = PasswordGenerator::evaluatePasswordStrength(password);
                    cout << "Password strength: " << strength << "/100 (" 
                         << PasswordGenerator::getStrengthDescription(strength) << ")" << endl;
                }
                
                cout << "Enter notes (optional): ";
                getline(cin, notes);
                
                PasswordEntry entry(website, username, password, notes);
                manager.addEntry(entry);
                
                cout << "Password entry added successfully!" << endl;
                break;
            }
            
            case 3: {
                clearScreen();
                manager.displayAllEntries(false);
                
                if (manager.getEntryCount() == 0) {
                    break;
                }
                
                int index;
                cout << "Enter entry number to edit (1-" << manager.getEntryCount() << "): ";
                cin >> index;
                clearInputBuffer();
                
                PasswordEntry* entry = manager.getEntry(index - 1);
                if (!entry) {
                    cout << "Invalid entry number." << endl;
                    break;
                }
                
                cout << "\nEditing entry:" << endl;
                entry->display(true);
                
                cout << "\nLeave fields blank to keep current values." << endl;
                
                string website, username, password, notes;
                
                cout << "New website: ";
                getline(cin, website);
                if (!website.empty()) entry->setWebsite(website);
                
                cout << "New username: ";
                getline(cin, username);
                if (!username.empty()) entry->setUsername(username);
                
                char genPass;
                cout << "Generate a new password? (y/n): ";
                cin >> genPass;
                clearInputBuffer();
                
                if (tolower(genPass) == 'y') {
                    int length;
                    bool upper, numbers, special;
                    
                    cout << "Password length (8-30): ";
                    cin >> length;
                    length = max(8, min(30, length));
                    
                    cout << "Include uppercase letters? (y/n): ";
                    cin >> genPass;
                    upper = (tolower(genPass) == 'y');
                    
                    cout << "Include numbers? (y/n): ";
                    cin >> genPass;
                    numbers = (tolower(genPass) == 'y');
                    
                    cout << "Include special characters? (y/n): ";
                    cin >> genPass;
                    special = (tolower(genPass) == 'y');
                    
                    clearInputBuffer();
                    
                    password = PasswordGenerator::generatePassword(length, upper, numbers, special);
                    cout << "Generated password: " << password << endl;
                    entry->setPassword(password);
                    
                    int strength = PasswordGenerator::evaluatePasswordStrength(password);
                    cout << "Password strength: " << strength << "/100 (" 
                         << PasswordGenerator::getStrengthDescription(strength) << ")" << endl;
                } else {
                    cout << "New password (leave blank to keep current): ";
                    getline(cin, password);
                    if (!password.empty()) {
                        entry->setPassword(password);
                        
                        int strength = PasswordGenerator::evaluatePasswordStrength(password);
                        cout << "Password strength: " << strength << "/100 (" 
                             << PasswordGenerator::getStrengthDescription(strength) << ")" << endl;
                    }
                }
                
                cout << "New notes (leave blank to keep current): ";
                getline(cin, notes);
                if (!notes.empty()) entry->setNotes(notes);
                
                cout << "Entry updated successfully!" << endl;
                break;
            }
            
            case 4: {
                clearScreen();
                manager.displayAllEntries(false);
                
                if (manager.getEntryCount() == 0) {
                    break;
                }
                
                int index;
                cout << "Enter entry number to delete (1-" << manager.getEntryCount() << "): ";
                cin >> index;
                clearInputBuffer();
                
                if (manager.removeEntry(index - 1)) {
                    cout << "Entry deleted successfully!" << endl;
                } else {
                    cout << "Invalid entry number." << endl;
                }
                break;
            }
            
            case 5: {
                clearScreen();
                string searchTerm;
                cout << "Enter search term: ";
                getline(cin, searchTerm);
                
                vector<PasswordEntry> results = manager.searchEntries(searchTerm);
                
                if (results.empty()) {
                    cout << "No matching entries found." << endl;
                } else {
                    cout << "\nSearch results for '" << searchTerm << "':" << endl;
                    for (size_t i = 0; i < results.size(); i++) {
                        cout << "\nResult " << (i + 1) << ":" << endl;
                        results[i].display(false);
                        cout << "-------------------------" << endl;
                    }
                    
                    char showPass;
                    cout << "Show passwords for these results? (y/n): ";
                    cin >> showPass;
                    clearInputBuffer();
                    
                    if (tolower(showPass) == 'y') {
                        cout << "\nResults with passwords:" << endl;
                        for (size_t i = 0; i < results.size(); i++) {
                            cout << "\nResult " << (i + 1) << ":" << endl;
                            results[i].display(true);
                            cout << "-------------------------" << endl;
                        }
                    }
                }
                break;
            }
            
            case 6: {
                clearScreen();
                int length;
                char includeUpper, includeNumbers, includeSpecial;
                
                cout << "Password length (8-30): ";
                cin >> length;
                length = max(8, min(30, length));
                
                cout << "Include uppercase letters? (y/n): ";
                cin >> includeUpper;
                
                cout << "Include numbers? (y/n): ";
                cin >> includeNumbers;
                
                cout << "Include special characters? (y/n): ";
                cin >> includeSpecial;
                
                clearInputBuffer();
                
                string password = PasswordGenerator::generatePassword(
                    length, 
                    tolower(includeUpper) == 'y',
                    tolower(includeNumbers) == 'y',
                    tolower(includeSpecial) == 'y'
                );
                
                int strength = PasswordGenerator::evaluatePasswordStrength(password);
                
                cout << "\nGenerated password: " << password << endl;
                cout << "Password strength: " << strength << "/100 (" 
                     << PasswordGenerator::getStrengthDescription(strength) << ")" << endl;
                
                char savePass;
                cout << "\nSave this password? (y/n): ";
                cin >> savePass;
                clearInputBuffer();
                
                if (tolower(savePass) == 'y') {
                    string website, username, notes;
                    
                    cout << "Enter website: ";
                    getline(cin, website);
                    
                    cout << "Enter username: ";
                    getline(cin, username);
                    
                    cout << "Enter notes (optional): ";
                    getline(cin, notes);
                    
                    PasswordEntry entry(website, username, password, notes);
                    manager.addEntry(entry);
                    
                    cout << "Password entry added successfully!" << endl;
                }
                break;
            }
            
            case 7:
                if (manager.saveToFile()) {
                    cout << "Passwords saved successfully. Goodbye!" << endl;
                } else {
                    cout << "Error saving passwords to file." << endl;
                }
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 