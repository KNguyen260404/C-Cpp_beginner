#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

void displayTasks(const vector<string>& tasks) {
    cout << "\n=== DANH SACH CONG VIEC ===" << endl;
    if(tasks.empty()) {
        cout << "Chua co cong viec nao." << endl;
        return;
    }
    
    for(int i = 0; i < tasks.size(); i++) {
        cout << (i + 1) << ". " << tasks[i] << endl;
    }
}

void saveTasks(const vector<string>& tasks) {
    ofstream file("tasks.txt");
    for(const string& task : tasks) {
        file << task << endl;
    }
    file.close();
}

void loadTasks(vector<string>& tasks) {
    ifstream file("tasks.txt");
    string task;
    while(getline(file, task)) {
        tasks.push_back(task);
    }
    file.close();
}

int main() {
    vector<string> tasks;
    loadTasks(tasks);
    
    int choice;
    string newTask;
    int taskIndex;
    
    do {
        cout << "\n=== QUAN LY CONG VIEC ===" << endl;
        cout << "1. Xem danh sach" << endl;
        cout << "2. Them cong viec moi" << endl;
        cout << "3. Xoa cong viec" << endl;
        cout << "4. Thoat" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                displayTasks(tasks);
                break;
            case 2:
                cout << "Nhap cong viec moi: ";
                cin.ignore(); // Clear buffer
                getline(cin, newTask);
                tasks.push_back(newTask);
                saveTasks(tasks);
                cout << "Da them cong viec!" << endl;
                break;
            case 3:
                displayTasks(tasks);
                if(!tasks.empty()) {
                    cout << "Nhap so thu tu cong viec can xoa: ";
                    cin >> taskIndex;
                    if(taskIndex > 0 && taskIndex <= tasks.size()) {
                        tasks.erase(tasks.begin() + taskIndex - 1);
                        saveTasks(tasks);
                        cout << "Da xoa cong viec!" << endl;
                    } else {
                        cout << "So thu tu khong hop le!" << endl;
                    }
                }
                break;
            case 4:
                cout << "Cam on ban da su dung!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le!" << endl;
        }
    } while(choice != 4);
    
    return 0;
}
