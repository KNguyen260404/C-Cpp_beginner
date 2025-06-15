#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <ctime>
#include <sstream>
using namespace std;

struct Transaction {
    int id;
    string date;
    string category;
    string description;
    double amount;
    string type; // "income" or "expense"
    
    Transaction() : id(0), amount(0) {}
    
    Transaction(int i, string d, string c, string desc, double a, string t)
        : id(i), date(d), category(c), description(desc), amount(a), type(t) {}
    
    void display() const {
        cout << left << setw(5) << id
             << setw(12) << date
             << setw(15) << category
             << setw(25) << description
             << setw(12) << fixed << setprecision(0) << amount
             << (type == "income" ? "Thu nhap" : "Chi tieu") << endl;
    }
    
    string toString() const {
        stringstream ss;
        ss << id << "|" << date << "|" << category << "|" 
           << description << "|" << amount << "|" << type;
        return ss.str();
    }
    
    static Transaction fromString(const string& str) {
        vector<string> parts;
        stringstream ss(str);
        string part;
        
        while(getline(ss, part, '|')) {
            parts.push_back(part);
        }
        
        if(parts.size() >= 6) {
            return Transaction(stoi(parts[0]), parts[1], parts[2], 
                             parts[3], stod(parts[4]), parts[5]);
        }
        return Transaction();
    }
};

class FinanceManager {
private:
    vector<Transaction> transactions;
    int nextId;
    string filename;
    map<string, vector<string>> categories;
    
public:
    FinanceManager(string file = "finance.txt") : filename(file), nextId(1) {
        initializeCategories();
        loadFromFile();
    }
    
    ~FinanceManager() {
        saveToFile();
    }
    
    void initializeCategories() {
        categories["income"] = {
            "Luong", "Thuong", "Lam them", "Dau tu", "Ban hang", "Khac"
        };
        
        categories["expense"] = {
            "An uong", "Di chuyen", "Mua sam", "Hoc tap", "Giai tri",
            "Y te", "Tien nha", "Tien dien nuoc", "Dien thoai", "Khac"
        };
    }
    
    string getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        
        return to_string(ltm->tm_mday) + "/" + 
               to_string(1 + ltm->tm_mon) + "/" + 
               to_string(1900 + ltm->tm_year);
    }
    
    void addIncome() {
        Transaction trans;
        trans.id = nextId++;
        trans.type = "income";
        trans.date = getCurrentDate();
        
        cout << "=== THEM THU NHAP ===" << endl;
        
        // Show categories
        cout << "Chon danh muc thu nhap:" << endl;
        for(int i = 0; i < categories["income"].size(); i++) {
            cout << (i + 1) << ". " << categories["income"][i] << endl;
        }
        
        int catChoice;
        cout << "Lua chon danh muc: ";
        cin >> catChoice;
        
        if(catChoice > 0 && catChoice <= categories["income"].size()) {
            trans.category = categories["income"][catChoice - 1];
        } else {
            trans.category = "Khac";
        }
        
        cout << "Nhap mo ta: ";
        cin.ignore();
        getline(cin, trans.description);
        
        cout << "Nhap so tien: ";
        cin >> trans.amount;
        
        transactions.push_back(trans);
        saveToFile();
        
        cout << "Da them thu nhap thanh cong! (ID: " << trans.id << ")" << endl;
    }
    
    void addExpense() {
        Transaction trans;
        trans.id = nextId++;
        trans.type = "expense";
        trans.date = getCurrentDate();
        
        cout << "=== THEM CHI TIEU ===" << endl;
        
        // Show categories
        cout << "Chon danh muc chi tieu:" << endl;
        for(int i = 0; i < categories["expense"].size(); i++) {
            cout << (i + 1) << ". " << categories["expense"][i] << endl;
        }
        
        int catChoice;
        cout << "Lua chon danh muc: ";
        cin >> catChoice;
        
        if(catChoice > 0 && catChoice <= categories["expense"].size()) {
            trans.category = categories["expense"][catChoice - 1];
        } else {
            trans.category = "Khac";
        }
        
        cout << "Nhap mo ta: ";
        cin.ignore();
        getline(cin, trans.description);
        
        cout << "Nhap so tien: ";
        cin >> trans.amount;
        
        transactions.push_back(trans);
        saveToFile();
        
        cout << "Da them chi tieu thanh cong! (ID: " << trans.id << ")" << endl;
    }
    
    void viewAllTransactions() {
        if(transactions.empty()) {
            cout << "Chua co giao dich nao!" << endl;
            return;
        }
        
        cout << "\n=== TAT CA GIAO DICH ===" << endl;
        cout << left << setw(5) << "ID"
             << setw(12) << "Ngay"
             << setw(15) << "Danh muc"
             << setw(25) << "Mo ta"
             << setw(12) << "So tien"
             << "Loai" << endl;
        cout << string(80, '-') << endl;
        
        for(const auto& trans : transactions) {
            trans.display();
        }
        
        cout << "\nTong cong: " << transactions.size() << " giao dich" << endl;
    }
    
    void viewByCategory() {
        string type;
        cout << "Xem theo loai (1-Thu nhap, 2-Chi tieu): ";
        int choice;
        cin >> choice;
        
        if(choice == 1) type = "income";
        else if(choice == 2) type = "expense";
        else {
            cout << "Lua chon khong hop le!" << endl;
            return;
        }
        
        cout << "\n=== " << (type == "income" ? "THU NHAP" : "CHI TIEU") << " THEO DANH MUC ===" << endl;
        
        map<string, double> categoryTotals;
        map<string, int> categoryCount;
        
        for(const auto& trans : transactions) {
            if(trans.type == type) {
                categoryTotals[trans.category] += trans.amount;
                categoryCount[trans.category]++;
            }
        }
        
        if(categoryTotals.empty()) {
            cout << "Khong co giao dich nao!" << endl;
            return;
        }
        
        cout << left << setw(20) << "Danh muc"
             << setw(15) << "So luong"
             << setw(15) << "Tong tien"
             << "Trung binh" << endl;
        cout << string(70, '-') << endl;
        
        double grandTotal = 0;
        for(const auto& pair : categoryTotals) {
            double average = pair.second / categoryCount[pair.first];
            cout << left << setw(20) << pair.first
                 << setw(15) << categoryCount[pair.first]
                 << setw(15) << fixed << setprecision(0) << pair.second
                 << fixed << setprecision(0) << average << endl;
            grandTotal += pair.second;
        }
        
        cout << string(70, '-') << endl;
        cout << "TONG CONG: " << fixed << setprecision(0) << grandTotal << " VND" << endl;
    }
    
    void viewMonthlyReport() {
        string month;
        cout << "Nhap thang can xem (mm/yyyy): ";
        cin >> month;
        
        double totalIncome = 0, totalExpense = 0;
        int incomeCount = 0, expenseCount = 0;
        
        cout << "\n=== BAO CAO THANG " << month << " ===" << endl;
        cout << left << setw(5) << "ID"
             << setw(12) << "Ngay"
             << setw(15) << "Danh muc"
             << setw(25) << "Mo ta"
             << setw(12) << "So tien"
             << "Loai" << endl;
        cout << string(80, '-') << endl;
        
        for(const auto& trans : transactions) {
            // Kiem tra xem giao dich co trong thang khong
            if(trans.date.substr(3) == month) {
                trans.display();
                
                if(trans.type == "income") {
                    totalIncome += trans.amount;
                    incomeCount++;
                } else {
                    totalExpense += trans.amount;
                    expenseCount++;
                }
            }
        }
        
        cout << string(80, '=') << endl;
        cout << "📊 THONG KE THANG " << month << ":" << endl;
        cout << "💰 Tong thu nhap: " << fixed << setprecision(0) << totalIncome 
             << " VND (" << incomeCount << " giao dich)" << endl;
        cout << "💸 Tong chi tieu: " << fixed << setprecision(0) << totalExpense 
             << " VND (" << expenseCount << " giao dich)" << endl;
        cout << "💹 So du: " << fixed << setprecision(0) << (totalIncome - totalExpense) << " VND" << endl;
        
        if(totalIncome > 0) {
            double savingRate = ((totalIncome - totalExpense) / totalIncome) * 100;
            cout << "📈 Ti le tiet kiem: " << fixed << setprecision(1) << savingRate << "%" << endl;
        }
    }
    
    void deleteTransaction() {
        int id;
        cout << "Nhap ID giao dich can xoa: ";
        cin >> id;
        
        for(auto it = transactions.begin(); it != transactions.end(); ++it) {
            if(it->id == id) {
                cout << "Giao dich can xoa:" << endl;
                cout << left << setw(5) << "ID"
                     << setw(12) << "Ngay"
                     << setw(15) << "Danh muc"
                     << setw(25) << "Mo ta"
                     << setw(12) << "So tien"
                     << "Loai" << endl;
                cout << string(80, '-') << endl;
                it->display();
                
                cout << "Ban co chac chan muon xoa? (y/n): ";
                char confirm;
                cin >> confirm;
                
                if(confirm == 'y' || confirm == 'Y') {
                    transactions.erase(it);
                    saveToFile();
                    cout << "Da xoa giao dich!" << endl;
                } else {
                    cout << "Khong xoa!" << endl;
                }
                return;
            }
        }
        
        cout << "Khong tim thay giao dich co ID: " << id << endl;
    }
    
    void searchTransactions() {
        string keyword;
        cout << "Nhap tu khoa tim kiem (mo ta hoac danh muc): ";
        cin.ignore();
        getline(cin, keyword);
        
        vector<Transaction> results;
        
        for(const auto& trans : transactions) {
            if(trans.description.find(keyword) != string::npos ||
               trans.category.find(keyword) != string::npos) {
                results.push_back(trans);
            }
        }
        
        if(results.empty()) {
            cout << "Khong tim thay giao dich nao!" << endl;
        } else {
            cout << "\n=== KET QUA TIM KIEM ===" << endl;
            cout << left << setw(5) << "ID"
                 << setw(12) << "Ngay"
                 << setw(15) << "Danh muc"
                 << setw(25) << "Mo ta"
                 << setw(12) << "So tien"
                 << "Loai" << endl;
            cout << string(80, '-') << endl;
            
            for(const auto& trans : results) {
                trans.display();
            }
            cout << "\nTim thay: " << results.size() << " giao dich" << endl;
        }
    }
    
    void exportReport() {
        string reportFile = "finance_report.txt";
        ofstream file(reportFile);
        
        if(!file.is_open()) {
            cout << "Khong the tao file bao cao!" << endl;
            return;
        }
        
        file << "BAO CAO TAI CHINH TONG HOP" << endl;
        file << "Ngay tao: " << getCurrentDate() << endl;
        file << string(50, '=') << endl;
        
        double totalIncome = 0, totalExpense = 0;
        map<string, double> incomeByCategory, expenseByCategory;
        
        for(const auto& trans : transactions) {
            if(trans.type == "income") {
                totalIncome += trans.amount;
                incomeByCategory[trans.category] += trans.amount;
            } else {
                totalExpense += trans.amount;
                expenseByCategory[trans.category] += trans.amount;
            }
        }
        
        file << "\n1. TONG QUAN:" << endl;
        file << "   - Tong thu nhap: " << fixed << setprecision(0) << totalIncome << " VND" << endl;
        file << "   - Tong chi tieu: " << fixed << setprecision(0) << totalExpense << " VND" << endl;
        file << "   - So du hien tai: " << fixed << setprecision(0) << (totalIncome - totalExpense) << " VND" << endl;
        
        file << "\n2. THU NHAP THEO DANH MUC:" << endl;
        for(const auto& pair : incomeByCategory) {
            double percentage = (pair.second / totalIncome) * 100;
            file << "   - " << pair.first << ": " << fixed << setprecision(0) 
                 << pair.second << " VND (" << setprecision(1) << percentage << "%)" << endl;
        }
        
        file << "\n3. CHI TIEU THEO DANH MUC:" << endl;
        for(const auto& pair : expenseByCategory) {
            double percentage = (pair.second / totalExpense) * 100;
            file << "   - " << pair.first << ": " << fixed << setprecision(0) 
                 << pair.second << " VND (" << setprecision(1) << percentage << "%)" << endl;
        }
        
        file << "\n4. CHI TIET TAT CA GIAO DICH:" << endl;
        for(const auto& trans : transactions) {
            file << trans.id << " | " << trans.date << " | " << trans.category 
                 << " | " << trans.description << " | " << trans.amount 
                 << " | " << (trans.type == "income" ? "Thu nhap" : "Chi tieu") << endl;
        }
        
        file.close();
        cout << "Da xuat bao cao ra file: " << reportFile << endl;
    }
    
private:
    void saveToFile() {
        ofstream file(filename);
        if(file.is_open()) {
            for(const auto& trans : transactions) {
                file << trans.toString() << endl;
            }
            file.close();
        }
    }
    
    void loadFromFile() {
        ifstream file(filename);
        if(file.is_open()) {
            string line;
            while(getline(file, line)) {
                if(!line.empty()) {
                    Transaction trans = Transaction::fromString(line);
                    if(trans.id > 0) {
                        transactions.push_back(trans);
                        if(trans.id >= nextId) {
                            nextId = trans.id + 1;
                        }
                    }
                }
            }
            file.close();
        }
    }
};

int main() {
    FinanceManager manager;
    int choice;
    
    cout << "=== HE THONG QUAN LY TAI CHINH CA NHAN ===" << endl;
    cout << "Chao mung ban den voi ung dung quan ly tai chinh!" << endl;
    
    do {
        cout << "\n================= MENU CHINH =================" << endl;
        cout << "1. 💰 Them thu nhap" << endl;
        cout << "2. 💸 Them chi tieu" << endl;
        cout << "3. 📋 Xem tat ca giao dich" << endl;
        cout << "4. 📊 Xem theo danh muc" << endl;
        cout << "5. 📅 Bao cao theo thang" << endl;
        cout << "6. 🔍 Tim kiem giao dich" << endl;
        cout << "7. 🗑️  Xoa giao dich" << endl;
        cout << "8. 📤 Xuat bao cao" << endl;
        cout << "9. 🚪 Thoat" << endl;
        cout << "=============================================" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                manager.addIncome();
                break;
            case 2:
                manager.addExpense();
                break;
            case 3:
                manager.viewAllTransactions();
                break;
            case 4:
                manager.viewByCategory();
                break;
            case 5:
                manager.viewMonthlyReport();
                break;
            case 6:
                manager.searchTransactions();
                break;
            case 7:
                manager.deleteTransaction();
                break;
            case 8:
                manager.exportReport();
                break;
            case 9:
                cout << "Cam on ban da su dung! Chuc ban quan ly tai chinh tot!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le! Vui long thu lai." << endl;
        }
        
        if(choice != 9) {
            cout << "\nNhan Enter de tiep tuc...";
            cin.ignore();
            cin.get();
        }
    } while(choice != 9);
    
    return 0;
}
