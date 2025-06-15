#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>
using namespace std;

struct Contact {
    string name;
    string phone;
    string email;
    string address;
    string birthday;
    string notes;
    
    Contact() {}
    
    Contact(string n, string p, string e, string a, string b, string nt) 
        : name(n), phone(p), email(e), address(a), birthday(b), notes(nt) {}
    
    void display() const {
        cout << left << setw(20) << name 
             << setw(15) << phone 
             << setw(25) << email 
             << setw(30) << address
             << setw(12) << birthday
             << notes << endl;
    }
    
    string toString() const {
        return name + "|" + phone + "|" + email + "|" + address + "|" + birthday + "|" + notes;
    }
    
    static Contact fromString(const string& str) {
        vector<string> parts;
        stringstream ss(str);
        string part;
        
        while(getline(ss, part, '|')) {
            parts.push_back(part);
        }
        
        if(parts.size() >= 6) {
            return Contact(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]);
        }
        return Contact();
    }
};

class ContactManager {
private:
    vector<Contact> contacts;
    string filename;
    
public:
    ContactManager(string file = "contacts.txt") : filename(file) {
        loadFromFile();
    }
    
    ~ContactManager() {
        saveToFile();
    }
    
    void addContact() {
        Contact contact;
        
        cout << "=== THEM LIEN HE MOI ===" << endl;
        cout << "Nhap ho ten: ";
        cin.ignore();
        getline(cin, contact.name);
        
        cout << "Nhap so dien thoai: ";
        getline(cin, contact.phone);
        
        cout << "Nhap email: ";
        getline(cin, contact.email);
        
        cout << "Nhap dia chi: ";
        getline(cin, contact.address);
        
        cout << "Nhap ngay sinh (dd/mm/yyyy): ";
        getline(cin, contact.birthday);
        
        cout << "Nhap ghi chu: ";
        getline(cin, contact.notes);
        
        contacts.push_back(contact);
        saveToFile();
        
        cout << "Da them lien he thanh cong!" << endl;
    }
    
    void displayAll() {
        if(contacts.empty()) {
            cout << "Danh ba trong!" << endl;
            return;
        }
        
        cout << "\n=== DANH SACH LIEN HE ===" << endl;
        cout << left << setw(20) << "Ho Ten" 
             << setw(15) << "So DT" 
             << setw(25) << "Email" 
             << setw(30) << "Dia Chi"
             << setw(12) << "Ngay Sinh"
             << "Ghi Chu" << endl;
        cout << string(120, '-') << endl;
        
        for(const auto& contact : contacts) {
            contact.display();
        }
        cout << "\nTong cong: " << contacts.size() << " lien he" << endl;
    }
    
    void searchContact() {
        string keyword;
        cout << "Nhap tu khoa tim kiem (ten/sdt/email): ";
        cin.ignore();
        getline(cin, keyword);
        
        vector<Contact> results;
        
        for(const auto& contact : contacts) {
            if(contact.name.find(keyword) != string::npos ||
               contact.phone.find(keyword) != string::npos ||
               contact.email.find(keyword) != string::npos) {
                results.push_back(contact);
            }
        }
        
        if(results.empty()) {
            cout << "Khong tim thay lien he nao!" << endl;
        } else {
            cout << "\n=== KET QUA TIM KIEM ===" << endl;
            cout << left << setw(20) << "Ho Ten" 
                 << setw(15) << "So DT" 
                 << setw(25) << "Email" 
                 << setw(30) << "Dia Chi"
                 << setw(12) << "Ngay Sinh"
                 << "Ghi Chu" << endl;
            cout << string(120, '-') << endl;
            
            for(const auto& contact : results) {
                contact.display();
            }
            cout << "\nTim thay: " << results.size() << " lien he" << endl;
        }
    }
    
    void deleteContact() {
        string name;
        cout << "Nhap ten lien he can xoa: ";
        cin.ignore();
        getline(cin, name);
        
        for(auto it = contacts.begin(); it != contacts.end(); ++it) {
            if(it->name == name) {
                cout << "Ban co chac chan muon xoa lien he: " << name << "? (y/n): ";
                char confirm;
                cin >> confirm;
                
                if(confirm == 'y' || confirm == 'Y') {
                    contacts.erase(it);
                    saveToFile();
                    cout << "Da xoa lien he!" << endl;
                } else {
                    cout << "Khong xoa!" << endl;
                }
                return;
            }
        }
        
        cout << "Khong tim thay lien he: " << name << endl;
    }
    
    void editContact() {
        string name;
        cout << "Nhap ten lien he can sua: ";
        cin.ignore();
        getline(cin, name);
        
        for(auto& contact : contacts) {
            if(contact.name == name) {
                cout << "\n=== SUA THONG TIN LIEN HE ===" << endl;
                cout << "Thong tin hien tai:" << endl;
                cout << left << setw(20) << "Ho Ten" 
                     << setw(15) << "So DT" 
                     << setw(25) << "Email" 
                     << setw(30) << "Dia Chi"
                     << setw(12) << "Ngay Sinh"
                     << "Ghi Chu" << endl;
                cout << string(120, '-') << endl;
                contact.display();
                
                cout << "\nNhap thong tin moi (Enter de giu nguyen):" << endl;
                
                string input;
                cout << "Ho ten moi: ";
                getline(cin, input);
                if(!input.empty()) contact.name = input;
                
                cout << "So dien thoai moi: ";
                getline(cin, input);
                if(!input.empty()) contact.phone = input;
                
                cout << "Email moi: ";
                getline(cin, input);
                if(!input.empty()) contact.email = input;
                
                cout << "Dia chi moi: ";
                getline(cin, input);
                if(!input.empty()) contact.address = input;
                
                cout << "Ngay sinh moi: ";
                getline(cin, input);
                if(!input.empty()) contact.birthday = input;
                
                cout << "Ghi chu moi: ";
                getline(cin, input);
                if(!input.empty()) contact.notes = input;
                
                saveToFile();
                cout << "Da cap nhat thong tin!" << endl;
                return;
            }
        }
        
        cout << "Khong tim thay lien he: " << name << endl;
    }
    
    void birthdayReminder() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        
        string today = to_string(ltm->tm_mday) + "/" + to_string(1 + ltm->tm_mon);
        
        cout << "\n=== NHAC NHO SINH NHAT ===" << endl;
        cout << "Hom nay (" << today << "):" << endl;
        
        bool found = false;
        for(const auto& contact : contacts) {
            if(contact.birthday.substr(0, 5) == today) {
                cout << "🎂 Sinh nhat cua: " << contact.name << " - " << contact.phone << endl;
                found = true;
            }
        }
        
        if(!found) {
            cout << "Khong co ai sinh nhat hom nay!" << endl;
        }
        
        // Kiem tra sinh nhat trong tuan toi
        cout << "\nSinh nhat trong 7 ngay toi:" << endl;
        found = false;
        
        for(int i = 1; i <= 7; i++) {
            time_t future = now + (i * 24 * 60 * 60);
            tm* ftm = localtime(&future);
            string futureDate = to_string(ftm->tm_mday) + "/" + to_string(1 + ftm->tm_mon);
            
            for(const auto& contact : contacts) {
                if(contact.birthday.substr(0, 5) == futureDate) {
                    cout << "📅 " << futureDate << " - " << contact.name << " - " << contact.phone << endl;
                    found = true;
                }
            }
        }
        
        if(!found) {
            cout << "Khong co sinh nhat nao trong 7 ngay toi!" << endl;
        }
    }
    
    void exportToCSV() {
        string csvFile = "contacts_export.csv";
        ofstream file(csvFile);
        
        if(!file.is_open()) {
            cout << "Khong the tao file CSV!" << endl;
            return;
        }
        
        // Header
        file << "Ho Ten,So Dien Thoai,Email,Dia Chi,Ngay Sinh,Ghi Chu" << endl;
        
        // Data
        for(const auto& contact : contacts) {
            file << "\"" << contact.name << "\","
                 << "\"" << contact.phone << "\","
                 << "\"" << contact.email << "\","
                 << "\"" << contact.address << "\","
                 << "\"" << contact.birthday << "\","
                 << "\"" << contact.notes << "\"" << endl;
        }
        
        file.close();
        cout << "Da xuat danh ba ra file: " << csvFile << endl;
    }
    
private:
    void saveToFile() {
        ofstream file(filename);
        if(file.is_open()) {
            for(const auto& contact : contacts) {
                file << contact.toString() << endl;
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
                    Contact contact = Contact::fromString(line);
                    if(!contact.name.empty()) {
                        contacts.push_back(contact);
                    }
                }
            }
            file.close();
        }
    }
};

int main() {
    ContactManager manager;
    int choice;
    
    cout << "=== HE THONG QUAN LY DANH BA ===" << endl;
    cout << "Chao mung ban den voi ung dung quan ly danh ba!" << endl;
    
    do {
        cout << "\n================== MENU ==================" << endl;
        cout << "1. 📝 Them lien he moi" << endl;
        cout << "2. 📋 Hien thi tat ca lien he" << endl;
        cout << "3. 🔍 Tim kiem lien he" << endl;
        cout << "4. ✏️  Sua thong tin lien he" << endl;
        cout << "5. 🗑️  Xoa lien he" << endl;
        cout << "6. 🎂 Nhac nho sinh nhat" << endl;
        cout << "7. 📤 Xuat ra file CSV" << endl;
        cout << "8. 🚪 Thoat" << endl;
        cout << "==========================================" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                manager.addContact();
                break;
            case 2:
                manager.displayAll();
                break;
            case 3:
                manager.searchContact();
                break;
            case 4:
                manager.editContact();
                break;
            case 5:
                manager.deleteContact();
                break;
            case 6:
                manager.birthdayReminder();
                break;
            case 7:
                manager.exportToCSV();
                break;
            case 8:
                cout << "Cam on ban da su dung! Hen gap lai!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le! Vui long thu lai." << endl;
        }
        
        if(choice != 8) {
            cout << "\nNhan Enter de tiep tuc...";
            cin.ignore();
            cin.get();
        }
    } while(choice != 8);
    
    return 0;
}
