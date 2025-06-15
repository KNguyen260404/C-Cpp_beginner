#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <cstdlib>
using namespace std;

struct Question {
    string question;
    vector<string> options;
    int correctAnswer;
    string explanation;
    
    Question() : correctAnswer(0) {}
    
    Question(string q, vector<string> opts, int correct, string exp)
        : question(q), options(opts), correctAnswer(correct), explanation(exp) {}
    
    void display(int questionNum) const {
        cout << "\n" << string(60, '=') << endl;
        cout << "CAU " << questionNum << ": " << question << endl;
        cout << string(60, '-') << endl;
        
        for(int i = 0; i < options.size(); i++) {
            cout << char('A' + i) << ". " << options[i] << endl;
        }
        cout << string(60, '=') << endl;
    }
    
    bool checkAnswer(char userAnswer) const {
        int answerIndex = toupper(userAnswer) - 'A';
        return answerIndex == correctAnswer;
    }
    
    void showExplanation() const {
        cout << "\n💡 GIAI THICH:" << endl;
        cout << "Dap an dung: " << char('A' + correctAnswer) 
             << ". " << options[correctAnswer] << endl;
        cout << "Ly do: " << explanation << endl;
    }
    
    string toString() const {
        string result = question + "|";
        for(int i = 0; i < options.size(); i++) {
            result += options[i];
            if(i < options.size() - 1) result += ";";
        }
        result += "|" + to_string(correctAnswer) + "|" + explanation;
        return result;
    }
    
    static Question fromString(const string& str) {
        vector<string> parts;
        string temp = "";
        int pipeCount = 0;
        
        for(char c : str) {
            if(c == '|') {
                parts.push_back(temp);
                temp = "";
                pipeCount++;
            } else {
                temp += c;
            }
        }
        parts.push_back(temp);
        
        if(parts.size() >= 4) {
            Question q;
            q.question = parts[0];
            
            // Parse options
            string optionsStr = parts[1];
            string option = "";
            for(char c : optionsStr) {
                if(c == ';') {
                    q.options.push_back(option);
                    option = "";
                } else {
                    option += c;
                }
            }
            q.options.push_back(option);
            
            q.correctAnswer = stoi(parts[2]);
            q.explanation = parts[3];
            
            return q;
        }
        return Question();
    }
};

struct QuizResult {
    string playerName;
    string category;
    int totalQuestions;
    int correctAnswers;
    int score;
    string date;
    int timeUsed; // seconds
    
    QuizResult() : totalQuestions(0), correctAnswers(0), score(0), timeUsed(0) {}
    
    double getPercentage() const {
        return totalQuestions > 0 ? (double)correctAnswers / totalQuestions * 100 : 0;
    }
    
    string getGrade() const {
        double percentage = getPercentage();
        if(percentage >= 90) return "A+";
        else if(percentage >= 80) return "A";
        else if(percentage >= 70) return "B";
        else if(percentage >= 60) return "C";
        else if(percentage >= 50) return "D";
        else return "F";
    }
    
    void display() const {
        cout << left << setw(15) << playerName
             << setw(15) << category
             << setw(8) << correctAnswers << "/" << totalQuestions
             << setw(8) << fixed << setprecision(1) << getPercentage() << "%"
             << setw(5) << getGrade()
             << setw(8) << timeUsed << "s"
             << date << endl;
    }
    
    string toString() const {
        return playerName + "|" + category + "|" + to_string(totalQuestions) + "|" +
               to_string(correctAnswers) + "|" + to_string(score) + "|" + 
               date + "|" + to_string(timeUsed);
    }
    
    static QuizResult fromString(const string& str) {
        vector<string> parts;
        string temp = "";
        
        for(char c : str) {
            if(c == '|') {
                parts.push_back(temp);
                temp = "";
            } else {
                temp += c;
            }
        }
        parts.push_back(temp);
        
        if(parts.size() >= 7) {
            QuizResult result;
            result.playerName = parts[0];
            result.category = parts[1];
            result.totalQuestions = stoi(parts[2]);
            result.correctAnswers = stoi(parts[3]);
            result.score = stoi(parts[4]);
            result.date = parts[5];
            result.timeUsed = stoi(parts[6]);
            return result;
        }
        return QuizResult();
    }
};

class QuizSystem {
private:
    map<string, vector<Question>> questionBank;
    vector<QuizResult> results;
    string questionFile;
    string resultFile;
    
public:
    QuizSystem() : questionFile("questions.txt"), resultFile("quiz_results.txt") {
        initializeQuestions();
        loadResults();
    }
    
    ~QuizSystem() {
        saveResults();
    }
    
    void initializeQuestions() {
        // Load questions from file first
        loadQuestions();
        
        // If no questions loaded, create default ones
        if(questionBank.empty()) {
            createDefaultQuestions();
            saveQuestions();
        }
    }
      void createDefaultQuestions() {
        // C++ Programming Questions (Expanded)
        vector<Question> cppQuestions = {
            Question("C++ la ngon ngu lap trinh thuoc loai nao?",
                    {"Interpretive", "Compiled", "Scripting", "Markup"},
                    1, "C++ la ngon ngu bien dich (compiled), code duoc chuyen thanh machine code truoc khi chay."),
            
            Question("Tu khoa nao duoc su dung de khai bao bien trong C++?",
                    {"var", "let", "int", "dim"},
                    2, "Trong C++, su dung cac tu khoa nhu int, float, double, char de khai bao bien."),
            
            Question("Dau nao duoc su dung de ket thuc mot cau lenh trong C++?",
                    {".", ",", ";", ":"},
                    2, "Dau cham phay (;) duoc su dung de ket thuc moi cau lenh trong C++."),
            
            Question("Ham nao la diem bat dau cua chuong trinh C++?",
                    {"start()", "begin()", "main()", "init()"},
                    2, "Ham main() la diem bat dau thuc thi cua moi chuong trinh C++."),
            
            Question("Lenh nao duoc su dung de in ra man hinh trong C++?",
                    {"print", "cout", "printf", "write"},
                    1, "cout la doi tuong output stream duoc su dung de in du lieu ra man hinh."),
            
            Question("Kich thuoc cua kieu du lieu int trong C++ la bao nhieu byte?",
                    {"2 bytes", "4 bytes", "8 bytes", "16 bytes"},
                    1, "Trong hau het he thong hien dai, int co kich thuoc 4 bytes (32 bit)."),
            
            Question("Tu khoa nao duoc su dung de tao vong lap trong C++?",
                    {"loop", "for", "repeat", "iterate"},
                    1, "Tu khoa 'for' duoc su dung de tao vong lap for trong C++."),
            
            Question("Operator nao duoc su dung de gan gia tri trong C++?",
                    {"==", "=", "!=", "=>"},
                    1, "Operator '=' duoc su dung de gan gia tri, '==' de so sanh."),
            
            Question("Tu khoa nao duoc su dung de tao class trong C++?",
                    {"class", "struct", "object", "type"},
                    0, "Tu khoa 'class' duoc su dung de dinh nghia lop trong C++."),
            
            Question("Header file nao can include de su dung iostream?",
                    {"<stdio.h>", "<iostream>", "<string.h>", "<math.h>"},
                    1, "<iostream> chua cac dinh nghia cho cin, cout va cac stream operations."),
            
            Question("Namespace nao thuong duoc su dung trong C++?",
                    {"system", "std", "cpp", "standard"},
                    1, "Namespace 'std' chua tat ca cac thanh phan cua C++ Standard Library."),
            
            Question("Tu khoa nao duoc su dung de tao pointer trong C++?",
                    {"&", "*", "ptr", "->"},
                    1, "Dau '*' duoc su dung de khai bao pointer, '&' de lay dia chi."),
            
            Question("Vong lap nao kiem tra dieu kien truoc khi thuc thi?",
                    {"do-while", "while", "for", "Ca B va C"},
                    3, "Ca while va for deu kiem tra dieu kien truoc khi thuc thi, khac voi do-while."),
            
            Question("Tu khoa nao duoc su dung de ke thua trong C++?",
                    {"extends", "inherits", "public", "derives"},
                    2, "Tu khoa 'public' (hoac private, protected) duoc su dung trong inheritance."),
            
            Question("Comment trong C++ bat dau bang ky tu nao?",
                    {"#", "//", "/*", "Ca B va C"},
                    3, "// cho single-line comment, /* */ cho multi-line comment trong C++.")
        };        
        // Math Questions (Expanded)
        vector<Question> mathQuestions = {
            Question("2 + 2 = ?",
                    {"3", "4", "5", "6"},
                    1, "2 + 2 = 4. Day la phep cong co ban."),
            
            Question("Can bac hai cua 16 la?",
                    {"2", "3", "4", "5"},
                    2, "√16 = 4 vi 4 × 4 = 16."),
            
            Question("10% cua 200 la bao nhieu?",
                    {"10", "20", "30", "40"},
                    1, "10% cua 200 = (10/100) × 200 = 20."),
            
            Question("Trong tam giac vuong, dinh ly Pythagore la gi?",
                    {"a² + b² = c²", "a + b = c", "a × b = c", "a - b = c"},
                    0, "Dinh ly Pythagore: Binh phuong canh huyen bang tong binh phuong hai canh goc vuong."),
            
            Question("1 km = ? met",
                    {"100m", "1000m", "10000m", "100000m"},
                    1, "1 kilometer = 1000 meter. Day la don vi do chieu dai co ban."),
            
            Question("5! (5 giai thua) bang bao nhieu?",
                    {"60", "120", "24", "100"},
                    1, "5! = 5 × 4 × 3 × 2 × 1 = 120."),
            
            Question("Dien tich hinh tron co ban kinh 5cm la?",
                    {"25π cm²", "10π cm²", "50π cm²", "π cm²"},
                    0, "Dien tich hinh tron = π × r² = π × 5² = 25π cm²."),
            
            Question("Nghiem cua phuong trinh x² - 4 = 0 la?",
                    {"x = ±2", "x = ±4", "x = 2", "x = 4"},
                    0, "x² = 4, nen x = ±2 (x = 2 hoac x = -2)."),
            
            Question("Sin(90°) bang bao nhieu?",
                    {"0", "1", "0.5", "√3/2"},
                    1, "Sin(90°) = 1, day la gia tri toi da cua ham sin."),
            
            Question("Log₁₀(100) bang bao nhieu?",
                    {"1", "2", "10", "100"},
                    1, "Log₁₀(100) = 2 vi 10² = 100."),
            
            Question("Dao ham cua x² la gi?",
                    {"x", "2x", "x²", "2x²"},
                    1, "Dao ham cua x² la 2x theo quy tac luy thua."),
            
            Question("Tong cac goc trong tam giac la bao nhieu?",
                    {"90°", "180°", "270°", "360°"},
                    1, "Tong ba goc trong tam giac luon bang 180°."),
            
            Question("Can bac ba cua 27 la?",
                    {"3", "9", "6", "12"},
                    0, "∛27 = 3 vi 3³ = 27."),
            
            Question("Gia tri tuyet doi cua -5 la?",
                    {"-5", "5", "0", "10"},
                    1, "Gia tri tuyet doi cua so am la chinh no nhung duong: |-5| = 5."),
            
            Question("Trong day so Fibonacci, so tiep theo sau 1, 1, 2, 3, 5 la?",
                    {"6", "7", "8", "9"},
                    2, "Day Fibonacci: moi so = tong 2 so truoc do. 3 + 5 = 8.")
        };        
        // General Knowledge Questions (Expanded)
        vector<Question> gkQuestions = {
            Question("Thu do cua Viet Nam la gi?",
                    {"Ho Chi Minh", "Ha Noi", "Da Nang", "Hue"},
                    1, "Ha Noi la thu do cua nuoc Cong hoa Xa hoi Chu nghia Viet Nam."),
            
            Question("Hanh tinh nao gan Mat Troi nhat?",
                    {"Kim Tinh", "Trai Dat", "Thuy Tinh", "Hoa Tinh"},
                    2, "Thuy Tinh (Mercury) la hanh tinh gan Mat Troi nhat trong he Mat Troi."),
            
            Question("Nuoc nao co dan so dong nhat the gioi?",
                    {"An Do", "Trung Quoc", "My", "Indonesia"},
                    1, "Trung Quoc hien tai co dan so dong nhat the gioi voi hon 1.4 ty nguoi."),
            
            Question("WWW viet tat cua gi?",
                    {"World Wide Web", "World War Web", "Wide World Web", "Web World Wide"},
                    0, "WWW la viet tat cua World Wide Web - he thong thong tin toan cau."),
            
            Question("Ngon ngu lap trinh nao duoc phat trien dau tien?",
                    {"FORTRAN", "COBOL", "BASIC", "Assembly"},
                    0, "FORTRAN (1957) duoc coi la ngon ngu lap trinh bac cao dau tien."),
            
            Question("Dai duong nao lon nhat the gioi?",
                    {"Dai Tay Duong", "An Do Duong", "Thai Binh Duong", "Bac Bang Duong"},
                    2, "Thai Binh Duong la dai duong lon nhat, chiem 1/3 dien tich Trai Dat."),
            
            Question("Nua nao co Viet Nam?",
                    {"Bac ban cau", "Nam ban cau", "Dong ban cau", "Tay ban cau"},
                    2, "Viet Nam nam o Dong Nam A, thuoc Dong ban cau."),
            
            Question("Nguoi dau tien dat chan len Mat Trang la ai?",
                    {"Yuri Gagarin", "Neil Armstrong", "Buzz Aldrin", "John Glenn"},
                    1, "Neil Armstrong la nguoi dau tien buoc chan len Mat Trang (1969)."),
            
            Question("Ngon ngu nao duoc noi nhieu nhat the gioi?",
                    {"Tieng Anh", "Tieng Trung", "Tieng Tay Ban Nha", "Tieng Hindi"},
                    1, "Tieng Trung (Mandarin) co so nguoi noi ban xu nhieu nhat."),
            
            Question("DNA viet tat cua gi?",
                    {"Deoxyribonucleic Acid", "Dynamic Nuclear Acid", "Direct Nuclear Acid", "Diverse Nuclear Acid"},
                    0, "DNA la Deoxyribonucleic Acid - vat chat di truyen trong te bao."),
            
            Question("Nuoc nao co hinh dang giong chiec boot (uong)?",
                    {"Phap", "Y", "Tay Ban Nha", "Hy Lap"},
                    1, "Y (Italia) co hinh dang giong chiec boot tren ban do."),
            
            Question("Ky quan nao duoc goi la 'Lung cua the gioi'?",
                    {"Rung Congo", "Rung Amazon", "Rung Siberia", "Rung Indonesia"},
                    1, "Rung Amazon duoc goi la 'Lung cua the gioi' vi san xuat oxy."),
            
            Question("Ai la nguoi sang lap Facebook?",
                    {"Bill Gates", "Steve Jobs", "Mark Zuckerberg", "Larry Page"},
                    2, "Mark Zuckerberg sang lap Facebook nam 2004 khi con la sinh vien Harvard."),
            
            Question("HTTP viet tat cua gi?",
                    {"HyperText Transfer Protocol", "High Tech Transfer Protocol", "HyperText Transport Protocol", "High Transfer Text Protocol"},
                    0, "HTTP la HyperText Transfer Protocol - giao thuc truyen tai web."),
            
            Question("Nuoc nao co nhieu dao nhat the gioi?",
                    {"Na Uy", "Thuy Dien", "Phan Lan", "Nhat Ban"},
                    2, "Phan Lan co khoang 188,000 dao, nhieu nhat the gioi."),
            
            Question("AI viet tat cua gi trong cong nghe?",
                    {"Automatic Intelligence", "Artificial Intelligence", "Advanced Intelligence", "Applied Intelligence"},
                    1, "AI la Artificial Intelligence - Tri tue nhan tao."),
            
            Question("Nuoc nao co dien tich lon nhat the gioi?",
                    {"Canada", "Trung Quoc", "Nga", "My"},
                    2, "Nga la nuoc co dien tich lon nhat the gioi (17.1 trieu km²)."),
            
            Question("Cong ty nao so huu YouTube?",
                    {"Microsoft", "Apple", "Google", "Amazon"},
                    2, "Google mua lai YouTube nam 2006 voi gia 1.65 ty USD."),
            
            Question("Dong vat nao chay nhanh nhat the gioi?",
                    {"Bao guepard", "Su tu", "Linh duong", "Ngua"},
                    0, "Bao guepard co the chay den 120 km/h trong khoang cach ngan."),
            
            Question("Ngan hang dau tien cua Viet Nam la gi?",
                    {"Vietcombank", "BIDV", "Agribank", "Ngan hang Dong Duong"},
                    3, "Ngan hang Dong Duong (1875) la ngan hang dau tien tai Viet Nam.")
        };
        
        // Science & Technology Questions (New Category)
        vector<Question> scienceQuestions = {
            Question("Nguyen to nao co ky hieu la 'O'?",
                    {"Oxi", "Osmium", "Oganesson", "Olivine"},
                    0, "O la ky hieu hoa hoc cua nguyen to Oxi (Oxygen)."),
            
            Question("Toc do anh sang trong chan khong la bao nhieu?",
                    {"300,000 km/s", "150,000 km/s", "500,000 km/s", "1,000,000 km/s"},
                    0, "Toc do anh sang trong chan khong la khoang 300,000 km/s."),
            
            Question("Cong thuc hoa hoc cua nuoc la gi?",
                    {"H2O", "H2O2", "HO2", "H3O"},
                    0, "Nuoc co cong thuc hoa hoc la H2O (2 nguyen tu Hydro + 1 nguyen tu Oxi)."),
            
            Question("CPU viet tat cua gi?",
                    {"Computer Processing Unit", "Central Processing Unit", "Core Processing Unit", "Central Program Unit"},
                    1, "CPU la Central Processing Unit - don vi xu ly trung tam cua may tinh."),
            
            Question("Hanh tinh nao duoc goi la 'Hanh tinh Do'?",
                    {"Kim Tinh", "Hoa Tinh", "Moc Tinh", "Tho Tinh"},
                    1, "Hoa Tinh (Mars) duoc goi la Hanh tinh Do vi mau do tren be mat."),
            
            Question("RAM viet tat cua gi?",
                    {"Random Access Memory", "Rapid Access Memory", "Read Access Memory", "Real Access Memory"},
                    0, "RAM la Random Access Memory - bo nho truy cap ngau nhien."),
            
            Question("Nguyen to nao nhe nhat trong bang tuan hoan?",
                    {"Helium", "Hydro", "Lithium", "Carbon"},
                    1, "Hydro (H) la nguyen to nhe nhat voi 1 proton va 1 electron."),
            
            Question("Bluetooth duoc dat ten theo ai?",
                    {"Nha khoa hoc Bluetooth", "Vua Harald Bluetooth", "Cong ty Bluetooth", "Thanh pho Bluetooth"},
                    1, "Bluetooth duoc dat ten theo Vua Harald Bluetooth cua Dan Mach."),
            
            Question("Cong thuc Einstein noi tieng nhat la gi?",
                    {"E=mc", "E=mc²", "E=m²c", "E=mc³"},
                    1, "E=mc² la cong thuc noi tieng nhat cua Einstein ve moi quan he nang luong-khoi luong."),
            
            Question("Wi-Fi viet tat cua gi?",
                    {"Wireless Fidelity", "Wide Fidelity", "Wireless Field", "Web Fidelity"},
                    0, "Wi-Fi co nghia la Wireless Fidelity - ket noi khong day.")
        };
        
        // History Questions (New Category)
        vector<Question> historyQuestions = {
            Question("Chien tranh the gioi thu 2 bat dau nam nao?",
                    {"1938", "1939", "1940", "1941"},
                    1, "Chien tranh the gioi thu 2 bat dau ngay 1/9/1939 khi Duc tan cong Ba Lan."),
            
            Question("Ai la hoang de dau tien cua La Ma?",
                    {"Julius Caesar", "Augustus", "Nero", "Caligula"},
                    1, "Augustus (Octavianus) la hoang de La Ma dau tien (27 TCN)."),
            
            Question("Cach mang thang 10 Nga xay ra nam nao?",
                    {"1916", "1917", "1918", "1919"},
                    1, "Cach mang thang 10 Nga xay ra nam 1917 do Lenin lanh dao."),
            
            Question("Viet Nam tuyen bo doc lap nam nao?",
                    {"1944", "1945", "1946", "1947"},
                    1, "Viet Nam tuyen bo doc lap ngay 2/9/1945 tai Quang truong Ba Dinh."),
            
            Question("Tuong Berlin bi pho bo nam nao?",
                    {"1987", "1988", "1989", "1990"},
                    2, "Tuong Berlin bi pho bo ngay 9/11/1989, ket thuc su chia cat Dong-Tay Duc."),
            
            Question("Ai la chu tich dau tien cua nuoc Viet Nam Dan chu Cong hoa?",
                    {"Ho Chi Minh", "Ton Duc Thang", "Le Duan", "Pham Van Dong"},
                    0, "Ho Chi Minh la chu tich dau tien cua nuoc Viet Nam Dan chu Cong hoa."),
            
            Question("Kim tu thap Ai Cap co bao nhieu kim tu thap lon?",
                    {"2", "3", "4", "5"},
                    1, "Co 3 kim tu thap lon tai Giza: Khufu, Khafre va Menkaure."),
            
            Question("Chien tranh Viet Nam ket thuc nam nao?",
                    {"1973", "1974", "1975", "1976"},
                    2, "Chien tranh Viet Nam ket thuc ngay 30/4/1975 khi Sai Gon giai phong."),
            
            Question("Napoleon bi luu day den dao nao?",
                    {"Corsica", "Elba", "Saint Helena", "Ca B va C"},
                    3, "Napoleon bi luu day den dao Elba (1814) va dao Saint Helena (1815)."),
            
            Question("Ai la nguoi sang lap ra Apple?",
                    {"Bill Gates", "Steve Jobs", "Mark Zuckerberg", "Larry Page"},
                    1, "Steve Jobs cung voi Steve Wozniak sang lap Apple nam 1976.")
        };
        
        // Sports Questions (New Category)
        vector<Question> sportsQuestions = {
            Question("World Cup bong da dau tien to chuc nam nao?",
                    {"1928", "1930", "1932", "1934"},
                    1, "World Cup dau tien duoc to chuc tai Uruguay nam 1930."),
            
            Question("Olympic hien dai dau tien to chuc o dau?",
                    {"Paris", "London", "Athens", "Rome"},
                    2, "Olympic hien dai dau tien duoc to chuc tai Athens, Hy Lap nam 1896."),
            
            Question("Bong da co bao nhieu cau thu tren san moi doi?",
                    {"10", "11", "12", "13"},
                    1, "Moi doi bong da co 11 cau thu tren san (bao gom 1 thu mon)."),
            
            Question("Ai duoc coi la 'Vua bong da' Brazil?",
                    {"Ronaldinho", "Pele", "Ronaldo", "Neymar"},
                    1, "Pele duoc coi la 'Vua bong da' va la cau thu vi dai nhat moi thoi dai."),
            
            Question("Tennis co bao nhieu set trong tran dau Grand Slam nam?",
                    {"3 set", "5 set", "7 set", "Tuy thuoc"},
                    1, "Tennis Grand Slam nam thi dau theo che do best-of-5 sets."),
            
            Question("Doi tuyen bong da Viet Nam vo dich AFF Cup lan dau nam nao?",
                    {"2008", "2018", "2020", "Chua bao gio"},
                    0, "Doi tuyen Viet Nam vo dich AFF Cup lan dau nam 2008."),
            
            Question("Basketball co bao nhieu cau thu tren san moi doi?",
                    {"4", "5", "6", "7"},
                    1, "Basketball moi doi co 5 cau thu tren san tai moi thoi diem."),
            
            Question("Ai la VDV boi loi gianh nhieu HCV Olympic nhat?",
                    {"Mark Spitz", "Michael Phelps", "Katie Ledecky", "Ian Thorpe"},
                    1, "Michael Phelps gianh 23 HCV Olympic, ky luc cua moi VDV."),
            
            Question("Formula 1 co bao nhieu vong dua trong mua giai?",
                    {"15-18", "20-24", "25-30", "Khong co dinh"},
                    1, "F1 thong thuong co 20-24 chang dua trong mua giai, tuy nam."),
            
            Question("Cau thu bong da nao ghi nhieu ban thang nhat lich su?",
                    {"Pele", "Maradona", "Cristiano Ronaldo", "Lionel Messi"},
                    2, "Cristiano Ronaldo hien giu ky luc ghi ban nhieu nhat lich su bong da.")
        };        
        questionBank["C++ Programming"] = cppQuestions;
        questionBank["Toan Hoc"] = mathQuestions;
        questionBank["Kien Thuc Tong Hop"] = gkQuestions;
        questionBank["Khoa Hoc & Cong Nghe"] = scienceQuestions;
        questionBank["Lich Su"] = historyQuestions;
        questionBank["The Thao"] = sportsQuestions;
        questionBank["Khoa Hoc & Cong Nghe"] = scienceQuestions;
        questionBank["Lich Su & Dia Ly"] = historyQuestions;
        questionBank["The Thao"] = sportsQuestions;
    }
    
    void showCategories() {
        cout << "\n=== DANH MUC CAU HOI ===" << endl;
        int index = 1;
        for(const auto& category : questionBank) {
            cout << index << ". " << category.first 
                 << " (" << category.second.size() << " cau hoi)" << endl;
            index++;
        }
    }
    
    string selectCategory() {
        showCategories();
        
        cout << "\nChon danh muc (nhap so): ";
        int choice;
        cin >> choice;
        
        int index = 1;
        for(const auto& category : questionBank) {
            if(index == choice) {
                return category.first;
            }
            index++;
        }
        
        // Default to first category if invalid choice
        return questionBank.begin()->first;
    }
    
    void startQuiz() {
        string playerName;
        cout << "Nhap ten cua ban: ";
        cin.ignore();
        getline(cin, playerName);
        
        string category = selectCategory();
        vector<Question> questions = questionBank[category];
        
        cout << "\nSo cau hoi ban muon tra loi (toi da " << questions.size() << "): ";
        int numQuestions;
        cin >> numQuestions;
        
        if(numQuestions > questions.size()) {
            numQuestions = questions.size();
        }
        
        // Shuffle questions
        random_shuffle(questions.begin(), questions.end());
        
        QuizResult result;
        result.playerName = playerName;
        result.category = category;
        result.totalQuestions = numQuestions;
        result.correctAnswers = 0;
        
        // Get current date
        time_t now = time(0);
        tm* ltm = localtime(&now);
        result.date = to_string(ltm->tm_mday) + "/" + 
                     to_string(1 + ltm->tm_mon) + "/" + 
                     to_string(1900 + ltm->tm_year);
        
        cout << "\n🎯 BAT DAU QUIZ: " << category << endl;
        cout << "Nguoi choi: " << playerName << endl;
        cout << "So cau hoi: " << numQuestions << endl;
        cout << "Nhan Enter de bat dau...";
        cin.ignore();
        cin.get();
        
        time_t startTime = time(0);
        
        for(int i = 0; i < numQuestions; i++) {
            questions[i].display(i + 1);
            
            char answer;
            cout << "Dap an cua ban (A/B/C/D): ";
            cin >> answer;
            
            if(questions[i].checkAnswer(answer)) {
                cout << "✅ CHINH XAC!" << endl;
                result.correctAnswers++;
                result.score += 10;
            } else {
                cout << "❌ SAI ROI!" << endl;
            }
            
            questions[i].showExplanation();
            
            if(i < numQuestions - 1) {
                cout << "\nNhan Enter de tiep tuc...";
                cin.ignore();
                cin.get();
            }
        }
        
        time_t endTime = time(0);
        result.timeUsed = endTime - startTime;
        
        showQuizResult(result);
        results.push_back(result);
        saveResults();
    }
    
    void showQuizResult(const QuizResult& result) {
        cout << "\n" << string(60, '=') << endl;
        cout << "🏆 KET QUA QUIZ" << endl;
        cout << string(60, '=') << endl;
        cout << "Nguoi choi: " << result.playerName << endl;
        cout << "Danh muc: " << result.category << endl;
        cout << "Diem so: " << result.score << "/" << (result.totalQuestions * 10) << endl;
        cout << "Dung: " << result.correctAnswers << "/" << result.totalQuestions << endl;
        cout << "Ti le: " << fixed << setprecision(1) << result.getPercentage() << "%" << endl;
        cout << "Xep loai: " << result.getGrade() << endl;
        cout << "Thoi gian: " << result.timeUsed << " giay" << endl;
        
        // Performance evaluation
        double percentage = result.getPercentage();
        cout << "\n💬 NHAN XET: ";
        if(percentage >= 90) {
            cout << "XUAT SAC! Ban thuc su gioi!" << endl;
        } else if(percentage >= 70) {
            cout << "TOT LAM! Ket qua rat an tuong!" << endl;
        } else if(percentage >= 50) {
            cout << "KHUYEN KHICH! Hay co gang hon nhe!" << endl;
        } else {
            cout << "CAN CO GANG THEM! Dung nan long!" << endl;
        }
        cout << string(60, '=') << endl;
    }
    
    void viewHighScores() {
        if(results.empty()) {
            cout << "Chua co ket qua nao!" << endl;
            return;
        }
        
        // Sort by score descending
        vector<QuizResult> sortedResults = results;
        sort(sortedResults.begin(), sortedResults.end(), 
             [](const QuizResult& a, const QuizResult& b) {
                 return a.score > b.score;
             });
        
        cout << "\n🏆 BANG XEP HANG (TOP 10)" << endl;
        cout << left << setw(15) << "Ten"
             << setw(15) << "Danh muc"
             << setw(8) << "Dung"
             << setw(8) << "Ti le"
             << setw(5) << "Loai"
             << setw(8) << "Thoi gian"
             << "Ngay" << endl;
        cout << string(80, '-') << endl;
        
        int count = 0;
        for(const auto& result : sortedResults) {
            cout << (count + 1) << ". ";
            result.display();
            count++;
            if(count >= 10) break;
        }
    }
    
    void viewStatistics() {
        if(results.empty()) {
            cout << "Chua co du lieu thong ke!" << endl;
            return;
        }
        
        cout << "\n📊 THONG KE TONG QUAN" << endl;
        cout << string(40, '=') << endl;
        
        // Overall stats
        int totalQuizzes = results.size();
        int totalQuestions = 0;
        int totalCorrect = 0;
        map<string, int> categoryCount;
        map<string, double> categoryAverage;
        
        for(const auto& result : results) {
            totalQuestions += result.totalQuestions;
            totalCorrect += result.correctAnswers;
            categoryCount[result.category]++;
            categoryAverage[result.category] += result.getPercentage();
        }
        
        cout << "Tong so quiz da lam: " << totalQuizzes << endl;
        cout << "Tong so cau hoi: " << totalQuestions << endl;
        cout << "Ti le dung chung: " << fixed << setprecision(1) 
             << ((double)totalCorrect / totalQuestions * 100) << "%" << endl;
        
        cout << "\nThong ke theo danh muc:" << endl;
        for(const auto& pair : categoryCount) {
            double avgPercentage = categoryAverage[pair.first] / pair.second;
            cout << "- " << pair.first << ": " << pair.second 
                 << " quiz, TB: " << fixed << setprecision(1) 
                 << avgPercentage << "%" << endl;
        }
    }
    
    void addQuestion() {
        showCategories();
        cout << "Chon danh muc de them cau hoi (nhap so): ";
        int choice;
        cin >> choice;
        
        string category;
        int index = 1;
        for(const auto& cat : questionBank) {
            if(index == choice) {
                category = cat.first;
                break;
            }
            index++;
        }
        
        if(category.empty()) {
            cout << "Danh muc khong hop le!" << endl;
            return;
        }
        
        Question newQuestion;
        cout << "\nNhap cau hoi: ";
        cin.ignore();
        getline(cin, newQuestion.question);
        
        cout << "Nhap 4 lua chon:" << endl;
        for(int i = 0; i < 4; i++) {
            string option;
            cout << "Lua chon " << char('A' + i) << ": ";
            getline(cin, option);
            newQuestion.options.push_back(option);
        }
        
        cout << "Nhap dap an dung (0-3): ";
        cin >> newQuestion.correctAnswer;
        
        cout << "Nhap giai thich: ";
        cin.ignore();
        getline(cin, newQuestion.explanation);
        
        questionBank[category].push_back(newQuestion);
        saveQuestions();
        
        cout << "Da them cau hoi moi vao danh muc: " << category << endl;
    }
    
private:
    void saveQuestions() {
        ofstream file(questionFile);
        if(file.is_open()) {
            for(const auto& category : questionBank) {
                for(const auto& question : category.second) {
                    file << category.first << "|" << question.toString() << endl;
                }
            }
            file.close();
        }
    }
    
    void loadQuestions() {
        ifstream file(questionFile);
        if(file.is_open()) {
            string line;
            while(getline(file, line)) {
                if(!line.empty()) {
                    size_t pos = line.find('|');
                    if(pos != string::npos) {
                        string category = line.substr(0, pos);
                        string questionData = line.substr(pos + 1);
                        Question q = Question::fromString(questionData);
                        if(!q.question.empty()) {
                            questionBank[category].push_back(q);
                        }
                    }
                }
            }
            file.close();
        }
    }
    
    void saveResults() {
        ofstream file(resultFile);
        if(file.is_open()) {
            for(const auto& result : results) {
                file << result.toString() << endl;
            }
            file.close();
        }
    }
    
    void loadResults() {
        ifstream file(resultFile);
        if(file.is_open()) {
            string line;
            while(getline(file, line)) {
                if(!line.empty()) {
                    QuizResult result = QuizResult::fromString(line);
                    if(!result.playerName.empty()) {
                        results.push_back(result);
                    }
                }
            }
            file.close();
        }
    }
};

int main() {
    srand(time(0)); // Initialize random seed
    
    QuizSystem quiz;
    int choice;
    
    cout << "🎓 CHAO MUNG DEN VOI HE THONG QUIZ THONG MINH! 🎓" << endl;
    cout << "Hay san sang kiem tra kien thuc cua ban!" << endl;
    
    do {
        cout << "\n================ MENU CHINH ================" << endl;
        cout << "1. 🎯 Bat dau Quiz" << endl;
        cout << "2. 🏆 Xem bang xep hang" << endl;
        cout << "3. 📊 Xem thong ke" << endl;
        cout << "4. ➕ Them cau hoi moi" << endl;
        cout << "5. 🚪 Thoat" << endl;
        cout << "===========================================" << endl;
        cout << "Lua chon cua ban: ";
        cin >> choice;
        
        switch(choice) {
            case 1:
                quiz.startQuiz();
                break;
            case 2:
                quiz.viewHighScores();
                break;
            case 3:
                quiz.viewStatistics();
                break;
            case 4:
                quiz.addQuestion();
                break;
            case 5:
                cout << "Cam on ban da su dung! Chuc ban hoc tot!" << endl;
                break;
            default:
                cout << "Lua chon khong hop le! Vui long thu lai." << endl;
        }
        
        if(choice != 5) {
            cout << "\nNhan Enter de tiep tuc...";
            cin.ignore();
            cin.get();
        }
    } while(choice != 5);
    
    return 0;
}
