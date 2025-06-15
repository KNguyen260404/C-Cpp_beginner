#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

void readFile() {
    string filename;
    cout << "Nhap ten file can doc: ";
    cin >> filename;
    
    ifstream file(filename);
    if(!file.is_open()) {
        cout << "Khong the mo file " << filename << endl;
        return;
    }
    
    cout << "\n=== NOI DUNG FILE ===" << endl;
    string line;
    int lineNumber = 1;
    while(getline(file, line)) {
        cout << lineNumber << ": " << line << endl;
        lineNumber++;
    }
    file.close();
    cout << "\nDa doc xong file!" << endl;
}

void writeFile() {
    string filename, content;
    cout << "Nhap ten file can ghi: ";
    cin >> filename;
    
    cout << "Nhap noi dung (nhap 'END' de ket thuc):" << endl;
    cin.ignore();
    
    ofstream file(filename);
    if(!file.is_open()) {
        cout << "Khong the tao file " << filename << endl;
        return;
    }
    
    while(true) {
        getline(cin, content);
        if(content == "END") break;
        file << content << endl;
    }
    file.close();
    cout << "Da ghi vao file " << filename << " thanh cong!" << endl;
}

void copyFile() {
    string sourceFile, destFile;
    cout << "Nhap ten file nguon: ";
    cin >> sourceFile;
    cout << "Nhap ten file dich: ";
    cin >> destFile;
    
    ifstream source(sourceFile);
    if(!source.is_open()) {
        cout << "Khong the mo file nguon " << sourceFile << endl;
        return;
    }
    
    ofstream dest(destFile);
    if(!dest.is_open()) {
        cout << "Khong the tao file dich " << destFile << endl;
        source.close();
        return;
    }
    
    string line;
    while(getline(source, line)) {
        dest << line << endl;
    }
    
    source.close();
    dest.close();
    cout << "Da sao chep tu " << sourceFile << " sang " << destFile << endl;
}

void countWords() {
    string filename;
    cout << "Nhap ten file can dem tu: ";
    cin >> filename;
    
    ifstream file(filename);
    if(!file.is_open()) {
        cout << "Khong the mo file " << filename << endl;
        return;
    }
    
    int lineCount = 0, wordCount = 0, charCount = 0;
    string line, word;
    
    while(getline(file, line)) {
        lineCount++;
        charCount += line.length();
        
        stringstream ss(line);
        while(ss >> word) {
            wordCount++;
        }
    }
    
    file.close();
    
    cout << "\n=== THONG KE FILE ===" << endl;
    cout << "So dong: " << lineCount << endl;
    cout << "So tu: " << wordCount << endl;
    cout << "So ky tu: " << charCount << endl;
}

void searchInFile() {
    string filename, searchTerm;
    cout << "Nhap ten file can tim: ";
    cin >> filename;
    cout << "Nhap tu khoa can tim: ";
    cin.ignore();
    getline(cin, searchTerm);
    
    ifstream file(filename);
    if(!file.is_open()) {
        cout << "Khong the mo file " << filename << endl;
        return;
    }
    
    string line;
    int lineNumber = 1;
    bool found = false;
    
    cout << "\n=== KET QUA TIM KIEM ===" << endl;
    while(getline(file, line)) {
        if(line.find(searchTerm) != string::npos) {
            cout << "Dong " << lineNumber << ": " << line << endl;
            found = true;
        }
        lineNumber++;
    }
    
    if(!found) {
        cout << "Khong tim thay '" << searchTerm << "' trong file!" << endl;
    }
    
    file.close();
}

int main() {
    int choice;
    
    cout << "=== QUAN LY FILE TXT ===" << endl;
    
    do {
        cout << "\n1. Doc file" << endl;
        cout << "2. Ghi file" << endl;
        cout << "3. Sao chep file" << endl;
        cout << "4. Dem so dong/tu/ky tu" << endl;
        cout << "5. Tim kiem trong file" << endl;
        cout << "6. Thoat" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                readFile();
                break;
            case 2:
                writeFile();
                break;
            case 3:
                copyFile();
                break;
            case 4:
                countWords();
                break;
            case 5:
                searchInFile();
                break;
            case 6:
                cout << "Cam on ban da su dung!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le!" << endl;
        }
    } while(choice != 6);
    
    return 0;
}
