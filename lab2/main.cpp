#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <cstdlib>
#include <windows.h>
#include <cctype>
#include <clocale>

using namespace std;

bool checkHtml(const string& text) {
    regex tag("<[a-zA-Z][^>]*>");
    if (!regex_search(text, tag)) {
        cout << "Ошибка: в тексте нет HTML-тегов.\n";
        return false;
    }

    regex markdown("(?:^|\\n)#{1,6}\\s|\\*\\*[^*]+\\*\\*|(?:^|\\n)[-*+]\\s");
    regex asciidoc("(?:^|\\n)={1,6}\\s|(?:^|\\n)\\*{1,5}\\s");

    if (regex_search(text, markdown)) {
        cout << "Ошибка: текст похож на Markdown, а не HTML.\n";
        return false;
    }
    if (regex_search(text, asciidoc)) {
        cout << "Ошибка: текст похож на AsciiDoc, а не HTML.\n";
        return false;
    }

    if (text.find("<html") != string::npos && text.find("</html>") == string::npos) {
        cout << "Ошибка: не закрыт тег <html>.\n";
        return false;
    }
    if (text.find("<body") != string::npos && text.find("</body>") == string::npos) {
        cout << "Ошибка: не закрыт тег <body>.\n";
        return false;
    }
    return true;
}

vector<string> findHeaders(const string& text) {
    vector<string> res;
    regex re("<h[1-6][^>]*>([\\s\\S]*?)</h[1-6]>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it)
        res.push_back((*it)[1].str());
    return res;
}

vector<string> findParagraphs(const string& text) {
    vector<string> res;
    regex re("<p[^>]*>([\\s\\S]*?)</p>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it)
        res.push_back((*it)[1].str());
    return res;
}

vector<string> findLists(const string& text) {
    vector<string> res;
    regex re("<(ul|ol)[^>]*>([\\s\\S]*?)</\\1>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it) {
        string raw = (*it)[2].str();
        regex re_li("<li[^>]*>", regex::icase);
        string clean = regex_replace(raw, re_li, "\n  - ");
        regex re_li_close("</li>", regex::icase);
        clean = regex_replace(clean, re_li_close, "");
        
        size_t first = clean.find_first_not_of(" \n\r\t");
        if (first != string::npos) {
            size_t last = clean.find_last_not_of(" \n\r\t");
            res.push_back(clean.substr(first, (last - first + 1)));
        } else {
            res.push_back("");
        }
    }
    return res;
}

void printVector(const string& title, const vector<string>& v) {
    cout << "\n=== " << title << " (" << v.size() << ") ===\n";
    for (size_t i = 0; i < v.size(); ++i)
        cout << i + 1 << ") " << v[i] << "\n";
}

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

#include <locale>

// Очистка ФИО через wide strings (надёжная работа с UTF-8)
string cleanNameString(const string& s) {
    // Конвертируем UTF-8 string в wstring
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len == 0) return s;
    
    vector<wchar_t> wbuf(len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wbuf.data(), len);
    wstring ws(wbuf.data());
    
    // Очищаем: оставляем только буквы
    wstring result;
    for (wchar_t wc : ws) {
        if (iswalpha(wc)) {
            result += wc;
        }
    }
    
    // Конвертируем обратно в UTF-8 string
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8_len == 0) return "";
    
    vector<char> utf8_buf(utf8_len);
    WideCharToMultiByte(CP_UTF8, 0, result.c_str(), -1, utf8_buf.data(), utf8_len, NULL, NULL);
    
    return string(utf8_buf.data());
}

// Транслитерация через проверку байтов UTF-8
string transliterate(const string& rus) {
    string res = "";
    for (size_t i = 0; i < rus.size(); ) {
        unsigned char c = (unsigned char)rus[i];
        
        // Латиница — оставляем как есть
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            res += rus[i];
            i++;
        }
        // Кириллица UTF-8 (2 байта)
        else if (c >= 0xC0 && i + 1 < rus.size()) {
            unsigned char c2 = (unsigned char)rus[i + 1];
            
            // а (D0 B0)
            if (c == 0xD0 && c2 == 0xB0) res += 'a';
            // б (D0 B1)
            else if (c == 0xD0 && c2 == 0xB1) res += 'b';
            // в (D0 B2)
            else if (c == 0xD0 && c2 == 0xB2) res += 'v';
            // г (D0 B3)
            else if (c == 0xD0 && c2 == 0xB3) res += 'g';
            // д (D0 B4)
            else if (c == 0xD0 && c2 == 0xB4) res += 'd';
            // е (D0 B5)
            else if (c == 0xD0 && c2 == 0xB5) res += 'e';
            // ё (D1 91)
            else if (c == 0xD1 && c2 == 0x91) res += "yo";
            // ж (D0 B6)
            else if (c == 0xD0 && c2 == 0xB6) res += "zh";
            // з (D0 B7)
            else if (c == 0xD0 && c2 == 0xB7) res += 'z';
            // и (D0 B8)
            else if (c == 0xD0 && c2 == 0xB8) res += 'i';
            // й (D0 B9)
            else if (c == 0xD0 && c2 == 0xB9) res += 'y';
            // к (D0 BA)
            else if (c == 0xD0 && c2 == 0xBA) res += 'k';
            // л (D0 BB)
            else if (c == 0xD0 && c2 == 0xBB) res += 'l';
            // м (D0 BC)
            else if (c == 0xD0 && c2 == 0xBC) res += 'm';
            // н (D0 BD)
            else if (c == 0xD0 && c2 == 0xBD) res += 'n';
            // о (D0 BE)
            else if (c == 0xD0 && c2 == 0xBE) res += 'o';
            // п (D0 BF)
            else if (c == 0xD0 && c2 == 0xBF) res += 'p';
            // р (D1 80)
            else if (c == 0xD1 && c2 == 0x80) res += 'r';
            // с (D1 81)
            else if (c == 0xD1 && c2 == 0x81) res += 's';
            // т (D1 82)
            else if (c == 0xD1 && c2 == 0x82) res += 't';
            // у (D1 83)
            else if (c == 0xD1 && c2 == 0x83) res += 'u';
            // ф (D1 84)
            else if (c == 0xD1 && c2 == 0x84) res += 'f';
            // х (D1 85)
            else if (c == 0xD1 && c2 == 0x85) res += 'h';
            // ц (D1 86)
            else if (c == 0xD1 && c2 == 0x86) res += 'c';
            // ч (D1 87)
            else if (c == 0xD1 && c2 == 0x87) res += "ch";
            // ш (D1 88)
            else if (c == 0xD1 && c2 == 0x88) res += "sh";
            // щ (D1 89)
            else if (c == 0xD1 && c2 == 0x89) res += "sch";
            // ъ (D1 8A)
            else if (c == 0xD1 && c2 == 0x8A) res += "";
            // ы (D1 8B)
            else if (c == 0xD1 && c2 == 0x8B) res += 'y';
            // ь (D1 8C)
            else if (c == 0xD1 && c2 == 0x8C) res += "";
            // э (D1 8D)
            else if (c == 0xD1 && c2 == 0x8D) res += 'e';
            // ю (D1 8E)
            else if (c == 0xD1 && c2 == 0x8E) res += "yu";
            // я (D1 8F)
            else if (c == 0xD1 && c2 == 0x8F) res += "ya";
            // А (D0 90)
            else if (c == 0xD0 && c2 == 0x90) res += 'a';
            // Б (D0 91)
            else if (c == 0xD0 && c2 == 0x91) res += 'b';
            // В (D0 92)
            else if (c == 0xD0 && c2 == 0x92) res += 'v';
            // Г (D0 93)
            else if (c == 0xD0 && c2 == 0x93) res += 'g';
            // Д (D0 94)
            else if (c == 0xD0 && c2 == 0x94) res += 'd';
            // Е (D0 95)
            else if (c == 0xD0 && c2 == 0x95) res += 'e';
            // Ё (D0 81)
            else if (c == 0xD0 && c2 == 0x81) res += "yo";
            // Ж (D0 96)
            else if (c == 0xD0 && c2 == 0x96) res += "zh";
            // З (D0 97)
            else if (c == 0xD0 && c2 == 0x97) res += 'z';
            // И (D0 98)
            else if (c == 0xD0 && c2 == 0x98) res += 'i';
            // Й (D0 99)
            else if (c == 0xD0 && c2 == 0x99) res += 'y';
            // К (D0 9A)
            else if (c == 0xD0 && c2 == 0x9A) res += 'k';
            // Л (D0 9B)
            else if (c == 0xD0 && c2 == 0x9B) res += 'l';
            // М (D0 9C)
            else if (c == 0xD0 && c2 == 0x9C) res += 'm';
            // Н (D0 9D)
            else if (c == 0xD0 && c2 == 0x9D) res += 'n';
            // О (D0 9E)
            else if (c == 0xD0 && c2 == 0x9E) res += 'o';
            // П (D0 9F)
            else if (c == 0xD0 && c2 == 0x9F) res += 'p';
            // Р (D0 A0)
            else if (c == 0xD0 && c2 == 0xA0) res += 'r';
            // С (D0 A1)
            else if (c == 0xD0 && c2 == 0xA1) res += 's';
            // Т (D0 A2)
            else if (c == 0xD0 && c2 == 0xA2) res += 't';
            // У (D0 A3)
            else if (c == 0xD0 && c2 == 0xA3) res += 'u';
            // Ф (D0 A4)
            else if (c == 0xD0 && c2 == 0xA4) res += 'f';
            // Х (D0 A5)
            else if (c == 0xD0 && c2 == 0xA5) res += 'h';
            // Ц (D0 A6)
            else if (c == 0xD0 && c2 == 0xA6) res += 'c';
            // Ч (D0 A7)
            else if (c == 0xD0 && c2 == 0xA7) res += "ch";
            // Ш (D0 A8)
            else if (c == 0xD0 && c2 == 0xA8) res += "sh";
            // Щ (D0 A9)
            else if (c == 0xD0 && c2 == 0xA9) res += "sch";
            // Ъ (D0 AA)
            else if (c == 0xD0 && c2 == 0xAA) res += "";
            // Ы (D0 AB)
            else if (c == 0xD0 && c2 == 0xAB) res += 'y';
            // Ь (D0 AC)
            else if (c == 0xD0 && c2 == 0xAC) res += "";
            // Э (D0 AD)
            else if (c == 0xD0 && c2 == 0xAD) res += 'e';
            // Ю (D0 AE)
            else if (c == 0xD0 && c2 == 0xAE) res += "yu";
            // Я (D0 AF)
            else if (c == 0xD0 && c2 == 0xAF) res += "ya";
            else res += '?';
            
            i += 2;
        }
        else {
            i++;
        }
    }
    return res;
}

string capitalizeFirstUtf8(const string& s) {
    if (s.empty()) return s;
\
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len == 0) return s;
    
    vector<wchar_t> wbuf(len);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wbuf.data(), len);
    wstring ws(wbuf.data());
    
    if (ws.empty()) return s;
    
    // Первая буква — заглавная
    wchar_t first = ws[0];
    // а-п (0x430-0x43F) → А-П (0x410-0x41F)
    if (first >= 0x430 && first <= 0x43F) first = first - 0x20;
    // р-я (0x440-0x44F) → Р-Я (0x420-0x42F)
    else if (first >= 0x440 && first <= 0x44F) first = first - 0x20;
    // ё (0x451) → Ё (0x401)
    else if (first == 0x451) first = 0x401;
    // a-z → A-Z
    else if (first >= 'a' && first <= 'z') first = first - 32;
    ws[0] = first;
    
    // Остальные буквы — строчные
    for (size_t i = 1; i < ws.size(); i++) {
        wchar_t c = ws[i];
        // А-П (0x410-0x41F) → а-п (0x430-0x43F)
        if (c >= 0x410 && c <= 0x41F) c = c + 0x20;
        // Р-Я (0x420-0x42F) → р-я (0x440-0x44F)
        else if (c >= 0x420 && c <= 0x42F) c = c + 0x20;
        // Ё (0x401) → ё (0x451)
        else if (c == 0x401) c = 0x451;
        // A-Z → a-z
        else if (c >= 'A' && c <= 'Z') c = c + 32;
        ws[i] = c;
    }

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8_len == 0) return "";
    
    vector<char> utf8_buf(utf8_len);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, utf8_buf.data(), utf8_len, NULL, NULL);
    
    return string(utf8_buf.data());
}



bool validateUser(string& surname, string& name, string& patronymic,
                  string& birth, string& phone, string& email) {
    bool ok = true;

    surname = trim(surname);
    name = trim(name);
    patronymic = trim(patronymic);
    birth = trim(birth);
    phone = trim(phone);
    email = trim(email);

    surname    = capitalizeFirstUtf8(cleanNameString(surname));
    name       = capitalizeFirstUtf8(cleanNameString(name));
    patronymic = capitalizeFirstUtf8(cleanNameString(patronymic));

    if (surname.empty() || name.empty()) {
        cout << "Ошибка: Фамилия или имя пусты после очистки.\n";
        ok = false;
    }

    smatch m;
    regex reYMD("^(\\d{4})-(\\d{2})-(\\d{2})$");
    regex reDMY("^(\\d{2})[./](\\d{2})[./](\\d{4})$");
    
    if (regex_match(birth, m, reYMD)) {
        birth = m[3].str() + "." + m[2].str() + "." + m[1].str();
    } else if (regex_match(birth, m, reDMY)) {
        birth = m[1].str() + "." + m[2].str() + "." + m[3].str();
    } else {
        cout << "Ошибка: Неверный формат даты: '" << birth << "'\n";
        ok = false;
    }

    string digits;
    for (char c : phone) {
        if (isdigit((unsigned char)c)) digits += c;
    }
    if (digits.size() == 11 && (digits[0] == '7' || digits[0] == '8')) {
        digits[0] = '7';
        phone = "+" + digits;
    } else if (digits.size() == 10) {
        phone = "+7" + digits;
    } else {
        cout << "Ошибка: Неверное количество цифр в телефоне (" << digits.size() << ").\n";
        ok = false;
    }

    string cleanEmail;
    for (char c : email) {
        if (c != ' ') {
            cleanEmail += tolower((unsigned char)c);
        }
    }
    email = cleanEmail;
    
    regex emailRe("^[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,}$");
    if (!regex_match(email, emailRe)) {
        cout << "Ошибка: Неверный формат почты: '" << email << "'\n";
        ok = false;
    }

    return ok;
}

string randomFrom(const vector<string>& v) {
    return v[rand() % v.size()];
}

void generateUser(string& surname, string& name, string& patronymic,
                  string& birth, string& phone, string& email) {
    static vector<string> surnames  = {"Иванов","Сидоров","Петров","Смирнов","Кузнецов"};
    static vector<string> names     = {"Иван","Пётр","Алексей","Сергей","Дмитрий"};
    static vector<string> patronyms = {"Иванович","Петрович","Алексеевич","Сергеевич"};

    surname    = randomFrom(surnames);
    name       = randomFrom(names);
    patronymic = randomFrom(patronyms);

    int year  = 1970 + rand() % 36;
    int month = 1 + rand() % 12;
    int day   = 1 + rand() % 28;
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    birth = buf;

    snprintf(buf, sizeof(buf), "+7996-%04d-%03d", rand() % 10000, rand() % 1000);
    phone = buf;

    string translit_surname = transliterate(surname);
    for (char& c : translit_surname) c = tolower((unsigned char)c);
    snprintf(buf, sizeof(buf), "%s%d@pochta.ru", translit_surname.c_str(), rand() % 1000);
    email = buf;
}

int main() {
    setlocale(LC_ALL, ".UTF-8");
    system("chcp 65001 > nul");
    SetConsoleOutputCP(65001);
    srand((unsigned)time(nullptr));

    string filename = "test.html";
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Файл " << filename << " не найден. Используем встроенный тест.\n";
        filename = "Встроенный тест";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    string html_text = buffer.str();

    if (html_text.empty()) {
        html_text = "<html><body>\n<h1>Привет, мир!</h1>\n<p>Это абзац.</p>\n<ul>\n<li>Пункт 1</li>\n<li>Пункт 2</li>\n</ul>\n</body></html>";
    }

    cout << "--- Анализ файла: " << filename << " ---\n";
    if (checkHtml(html_text)) {
        cout << "HTML корректен.\n";
        printVector("Заголовки", findHeaders(html_text));
        printVector("Абзацы",    findParagraphs(html_text));
        printVector("Списки",    findLists(html_text));
    } else {
        cout << "Файл не является корректным HTML.\n";
    }

    cout << "\n--- Анкета ---\n";
    string s, n, p, b, ph, e;

    cout << "1) Ручной ввод (введите данные, можно с ошибками):\n";
    cout << "Фамилия: "; getline(cin >> ws, s);
    cout << "Имя: "; getline(cin, n);
    cout << "Отчество: "; getline(cin, p);
    cout << "Дата рождения (ГГГГ-ММ-ДД или ДД.ММ.ГГГГ): "; getline(cin, b);
    cout << "Телефон: "; getline(cin, ph);
    cout << "Почта: "; getline(cin, e);

    cout << "\n[Обработка данных...]\n";
    if (validateUser(s, n, p, b, ph, e)) {
        cout << "Успешно исправлено и проверено:\n";
        cout << s << " " << n << " " << p << " | " << b << " | " << ph << " | " << e << "\n";
    } else {
        cout << "Данные содержат критические ошибки и не могут быть исправлены.\n";
        cout << "Текущее состояние: " << s << " " << n << " " << p << " | " << b << " | " << ph << " | " << e << "\n";
    }

    cout << "\n2) Сгенерированные данные:\n";
    generateUser(s, n, p, b, ph, e);
    cout << "Сгенерировано: " << s << " " << n << " " << p << " | " << b << " | " << ph << " | " << e << "\n";
    
    if (validateUser(s, n, p, b, ph, e)) {
        cout << "Генератор выдал корректные данные, проверка пройдена:\n";
        cout << s << " " << n << " " << p << " | " << b << " | " << ph << " | " << e << "\n";
    }

    return 0;
}