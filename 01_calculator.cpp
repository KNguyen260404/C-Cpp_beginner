#include <iostream>
#include <iomanip>
using namespace std;

int main() {
    double num1, num2, result;
    char operation;
    
    cout << "=== CALCULATOR C++ ===" << endl;
    cout << "Nhap so thu nhat: ";
    cin >> num1;
    
    cout << "Nhap phep toan (+, -, *, /): ";
    cin >> operation;
    
    cout << "Nhap so thu hai: ";
    cin >> num2;
    
    switch(operation) {
        case '+':
            result = num1 + num2;
            cout << fixed << setprecision(2);
            cout << num1 << " + " << num2 << " = " << result << endl;
            break;
        case '-':
            result = num1 - num2;
            cout << fixed << setprecision(2);
            cout << num1 << " - " << num2 << " = " << result << endl;
            break;
        case '*':
            result = num1 * num2;
            cout << fixed << setprecision(2);
            cout << num1 << " * " << num2 << " = " << result << endl;
            break;
        case '/':
            if(num2 != 0) {
                result = num1 / num2;
                cout << fixed << setprecision(2);
                cout << num1 << " / " << num2 << " = " << result << endl;
            } else {
                cout << "Loi: Khong the chia cho 0!" << endl;
            }
            break;
        default:
            cout << "Phep toan khong hop le!" << endl;
    }
    
    return 0;
}
