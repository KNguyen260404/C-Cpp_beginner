#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iomanip>
using namespace std;

void displayArray(const vector<int>& arr) {
    cout << "Mang hien tai: ";
    for(int num : arr) {
        cout << num << " ";
    }
    cout << endl;
}

double calculateAverage(const vector<int>& arr) {
    if(arr.empty()) return 0;
    return (double)accumulate(arr.begin(), arr.end(), 0) / arr.size();
}

int main() {
    vector<int> numbers;
    int choice, value, index;
    
    cout << "=== QUAN LY MANG SO NGUYEN ===" << endl;
    
    do {
        cout << "\n1. Them phan tu" << endl;
        cout << "2. Xoa phan tu tai vi tri" << endl;
        cout << "3. Tim kiem phan tu" << endl;
        cout << "4. Sap xep mang" << endl;
        cout << "5. Tinh tong va trung binh" << endl;
        cout << "6. Tim gia tri lon nhat va nho nhat" << endl;
        cout << "7. Hien thi mang" << endl;
        cout << "8. Thoat" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                cout << "Nhap gia tri can them: ";
                cin >> value;
                numbers.push_back(value);
                cout << "Da them " << value << " vao mang!" << endl;
                break;
                
            case 2:
                if(numbers.empty()) {
                    cout << "Mang rong!" << endl;
                    break;
                }
                displayArray(numbers);
                cout << "Nhap vi tri can xoa (0-" << numbers.size()-1 << "): ";
                cin >> index;
                if(index >= 0 && index < numbers.size()) {
                    numbers.erase(numbers.begin() + index);
                    cout << "Da xoa phan tu tai vi tri " << index << endl;
                } else {
                    cout << "Vi tri khong hop le!" << endl;
                }
                break;
                
            case 3:
                cout << "Nhap gia tri can tim: ";
                cin >> value;
                {
                    auto it = find(numbers.begin(), numbers.end(), value);
                    if(it != numbers.end()) {
                        cout << "Tim thay " << value << " tai vi tri " << distance(numbers.begin(), it) << endl;
                    } else {
                        cout << "Khong tim thay " << value << " trong mang!" << endl;
                    }
                }
                break;
                
            case 4:
                if(numbers.empty()) {
                    cout << "Mang rong!" << endl;
                    break;
                }
                sort(numbers.begin(), numbers.end());
                cout << "Da sap xep mang tang dan!" << endl;
                displayArray(numbers);
                break;
                
            case 5:
                if(numbers.empty()) {
                    cout << "Mang rong!" << endl;
                    break;
                }
                {
                    int sum = accumulate(numbers.begin(), numbers.end(), 0);
                    double avg = calculateAverage(numbers);
                    cout << "Tong: " << sum << endl;
                    cout << "Trung binh: " << fixed << setprecision(2) << avg << endl;
                }
                break;
                
            case 6:
                if(numbers.empty()) {
                    cout << "Mang rong!" << endl;
                    break;
                }
                {
                    int maxVal = *max_element(numbers.begin(), numbers.end());
                    int minVal = *min_element(numbers.begin(), numbers.end());
                    cout << "Gia tri lon nhat: " << maxVal << endl;
                    cout << "Gia tri nho nhat: " << minVal << endl;
                }
                break;
                
            case 7:
                if(numbers.empty()) {
                    cout << "Mang rong!" << endl;
                } else {
                    displayArray(numbers);
                }
                break;
                
            case 8:
                cout << "Cam on ban da su dung!" << endl;
                break;
                
            default:
                cout << "Lua chon khong hop le!" << endl;
        }
    } while(choice != 8);
    
    return 0;
}
