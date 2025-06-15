#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

bool isPalindrome(string str) {
    // Chuyen ve chu thuong va loai bo khoang trang
    string cleaned = "";
    for(char c : str) {
        if(isalnum(c)) {
            cleaned += tolower(c);
        }
    }
    
    // Kiem tra palindrome
    string reversed = cleaned;
    reverse(reversed.begin(), reversed.end());
    
    return cleaned == reversed;
}

int countWords(string str) {
    int count = 0;
    bool inWord = false;
    
    for(char c : str) {
        if(c != ' ' && c != '\t' && c != '\n') {
            if(!inWord) {
                count++;
                inWord = true;
            }
        } else {
            inWord = false;
        }
    }
    return count;
}

int countVowels(string str) {
    int count = 0;
    string vowels = "aeiouAEIOU";
    
    for(char c : str) {
        if(vowels.find(c) != string::npos) {
            count++;
        }
    }
    return count;
}

string reverseString(string str) {
    string reversed = str;
    reverse(reversed.begin(), reversed.end());
    return reversed;
}

int main() {
    string text;
    int choice;
    
    cout << "=== CHUONG TRINH XU LY CHUOI ===" << endl;
    
    do {
        cout << "\n1. Kiem tra chuoi Palindrome" << endl;
        cout << "2. Dem so tu trong chuoi" << endl;
        cout << "3. Dem so nguyen am" << endl;
        cout << "4. Dao nguoc chuoi" << endl;
        cout << "5. Thoat" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                cout << "Nhap chuoi can kiem tra: ";
                cin.ignore();
                getline(cin, text);
                if(isPalindrome(text)) {
                    cout << "'" << text << "' la chuoi Palindrome!" << endl;
                } else {
                    cout << "'" << text << "' khong phai chuoi Palindrome!" << endl;
                }
                break;
            case 2:
                cout << "Nhap chuoi can dem tu: ";
                cin.ignore();
                getline(cin, text);
                cout << "So tu trong chuoi: " << countWords(text) << endl;
                break;
            case 3:
                cout << "Nhap chuoi can dem nguyen am: ";
                cin.ignore();
                getline(cin, text);
                cout << "So nguyen am trong chuoi: " << countVowels(text) << endl;
                break;
            case 4:
                cout << "Nhap chuoi can dao nguoc: ";
                cin.ignore();
                getline(cin, text);
                cout << "Chuoi dao nguoc: " << reverseString(text) << endl;
                break;
            case 5:
                cout << "Cam on ban da su dung!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le!" << endl;
        }
    } while(choice != 5);
    
    return 0;
}
