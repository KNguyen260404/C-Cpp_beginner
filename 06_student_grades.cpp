#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
using namespace std;

struct Student {
    int id;
    string name;
    float math, physics, chemistry;
    float average;
    
    void calculateAverage() {
        average = (math + physics + chemistry) / 3.0;
    }
    
    string getGrade() {
        if(average >= 9.0) return "Xuat sac";
        else if(average >= 8.0) return "Gioi";
        else if(average >= 6.5) return "Kha";
        else if(average >= 5.0) return "Trung binh";
        else return "Yeu";
    }
};

void displayStudent(const Student& s) {
    cout << left << setw(5) << s.id 
         << setw(20) << s.name 
         << setw(8) << fixed << setprecision(1) << s.math
         << setw(8) << s.physics 
         << setw(8) << s.chemistry
         << setw(8) << s.average 
         << s.getGrade() << endl;
}

void displayHeader() {
    cout << left << setw(5) << "ID" 
         << setw(20) << "Ho Ten" 
         << setw(8) << "Toan"
         << setw(8) << "Ly" 
         << setw(8) << "Hoa"
         << setw(8) << "TB" 
         << "Xep loai" << endl;
    cout << string(70, '-') << endl;
}

int main() {
    Student students[100];
    int count = 0;
    int choice;
    
    cout << "=== QUAN LY DIEM HOC SINH ===" << endl;
    
    do {
        cout << "\n1. Them hoc sinh" << endl;
        cout << "2. Hien thi danh sach" << endl;
        cout << "3. Tim hoc sinh theo ID" << endl;
        cout << "4. Hien thi hoc sinh gioi (TB >= 8.0)" << endl;
        cout << "5. Luu vao file" << endl;
        cout << "6. Doc tu file" << endl;
        cout << "7. Thoat" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                if(count < 100) {
                    Student& s = students[count];
                    cout << "Nhap ID: ";
                    cin >> s.id;
                    cout << "Nhap ho ten: ";
                    cin.ignore();
                    getline(cin, s.name);
                    cout << "Nhap diem Toan: ";
                    cin >> s.math;
                    cout << "Nhap diem Ly: ";
                    cin >> s.physics;
                    cout << "Nhap diem Hoa: ";
                    cin >> s.chemistry;
                    s.calculateAverage();
                    count++;
                    cout << "Da them hoc sinh thanh cong!" << endl;
                } else {
                    cout << "Khong the them hoc sinh (da day)!" << endl;
                }
                break;
                
            case 2:
                if(count == 0) {
                    cout << "Chua co hoc sinh nao!" << endl;
                } else {
                    displayHeader();
                    for(int i = 0; i < count; i++) {
                        displayStudent(students[i]);
                    }
                }
                break;
                
            case 3:
                {
                    int searchId;
                    cout << "Nhap ID can tim: ";
                    cin >> searchId;
                    bool found = false;
                    for(int i = 0; i < count; i++) {
                        if(students[i].id == searchId) {
                            displayHeader();
                            displayStudent(students[i]);
                            found = true;
                            break;
                        }
                    }
                    if(!found) {
                        cout << "Khong tim thay hoc sinh co ID " << searchId << endl;
                    }
                }
                break;
                
            case 4:
                {
                    bool hasGoodStudent = false;
                    displayHeader();
                    for(int i = 0; i < count; i++) {
                        if(students[i].average >= 8.0) {
                            displayStudent(students[i]);
                            hasGoodStudent = true;
                        }
                    }
                    if(!hasGoodStudent) {
                        cout << "Khong co hoc sinh nao dat loai gioi!" << endl;
                    }
                }
                break;
                
            case 5:
                {
                    ofstream file("students.txt");
                    file << count << endl;
                    for(int i = 0; i < count; i++) {
                        file << students[i].id << endl;
                        file << students[i].name << endl;
                        file << students[i].math << " " << students[i].physics << " " << students[i].chemistry << endl;
                    }
                    file.close();
                    cout << "Da luu vao file students.txt!" << endl;
                }
                break;
                
            case 6:
                {
                    ifstream file("students.txt");
                    if(file.is_open()) {
                        file >> count;
                        file.ignore();
                        for(int i = 0; i < count; i++) {
                            file >> students[i].id;
                            file.ignore();
                            getline(file, students[i].name);
                            file >> students[i].math >> students[i].physics >> students[i].chemistry;
                            students[i].calculateAverage();
                        }
                        file.close();
                        cout << "Da doc tu file students.txt!" << endl;
                    } else {
                        cout << "Khong the mo file!" << endl;
                    }
                }
                break;
                
            case 7:
                cout << "Cam on ban da su dung!" << endl;
                break;
                
            default:
                cout << "Lua chon khong hop le!" << endl;
        }
    } while(choice != 7);
    
    return 0;
}
