#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main() {
    srand(time(0)); // Khoi tao random seed
    int secretNumber = rand() % 100 + 1; // So ngau nhien tu 1-100
    int guess;
    int attempts = 0;
    
    cout << "=== GAME DOAN SO ===" << endl;
    cout << "Toi da nghi ra mot so tu 1 den 100. Ban hay doan xem!" << endl;
    
    do {
        cout << "Nhap so ban doan: ";
        cin >> guess;
        attempts++;
        
        if(guess < secretNumber) {
            cout << "So ban doan nho qua! Thu lai." << endl;
        } else if(guess > secretNumber) {
            cout << "So ban doan lon qua! Thu lai." << endl;
        } else {
            cout << "Chuc mung! Ban da doan dung!" << endl;
            cout << "So can tim la: " << secretNumber << endl;
            cout << "Ban da doan trong " << attempts << " lan thu." << endl;
        }
    } while(guess != secretNumber);
    
    // Danh gia ket qua
    if(attempts <= 5) {
        cout << "Tuyet voi! Ban rat gioi doan so!" << endl;
    } else if(attempts <= 10) {
        cout << "Khong toi! Ban doan kha tot!" << endl;
    } else {
        cout << "Ban can luyen tap them!" << endl;
    }
    
    return 0;
}
