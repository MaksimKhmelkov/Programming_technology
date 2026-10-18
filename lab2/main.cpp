#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <cstdlib>
#include <windows.h>

using namespace std;

// ============================================================
// 1. Проверка, что текст — это HTML (а не markdown/asciidoc/др.)
// ============================================================
bool checkHtml(const string& text) {
    // В HTML обязательно должны быть теги.
    regex tag("<[a-zA-Z][^>]*>");
    if (!regex_search(text, tag)) {
        cout << "Ошибка: в тексте нет HTML-тегов.\n";
        return false;
    }

    // Признаки других языков разметки — сообщаем об ошибке.
    // Проверка на Markdown (заголовки #, жирный текст **, списки - * +)
    regex markdown("(?:^|\\n)#{1,6}\\s|\\*\\*[^*]+\\*\\*|(?:^|\\n)[-*+]\\s");
    
    // Проверка на AsciiDoc (заголовки =, списки *)
    regex asciidoc("(?:^|\\n)={1,6}\\s|(?:^|\\n)\\*{1,5}\\s");

    if (regex_search(text, markdown)) {
        cout << "Ошибка: текст похож на Markdown, а не HTML.\n";
        return false;
    }
    if (regex_search(text, asciidoc)) {
        cout << "Ошибка: текст похож на AsciiDoc, а не HTML.\n";
        return false;
    }

    // Проверка парности основных тегов (упрощённо).
    if (text.find("<html") != string::npos &&
        text.find("</html>") == string::npos) {
        cout << "Ошибка: не закрыт тег <html>.\n";
        return false;
    }
    if (text.find("<body") != string::npos &&
        text.find("</body>") == string::npos) {
        cout << "Ошибка: не закрыт тег <body>.\n";
        return false;
    }
    return true;
}

// ============================================================
// 2. Поиск заголовков, абзацев, списков
// ============================================================
vector<string> findHeaders(const string& text) {
    vector<string> res;
    // <h1>...</h1> ... <h6>...</h6>
    regex re("<h[1-6][^>]*>(.*?)</h[1-6]>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it)
        res.push_back((*it)[1].str());
    return res;
}

vector<string> findParagraphs(const string& text) {
    vector<string> res;
    regex re("<p[^>]*>(.*?)</p>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it)
        res.push_back((*it)[1].str());
    return res;
}

vector<string> findLists(const string& text) {
    vector<string> res;
    // весь блок <ul>...</ul> или <ol>...</ol>
    regex re("<(ul|ol)[^>]*>(.*?)</(ul|ol)>", regex::icase);
    for (sregex_iterator it(text.begin(), text.end(), re), end; it != end; ++it)
        res.push_back((*it)[2].str());
    return res;
}

void printVector(const string& title, const vector<string>& v) {
    cout << "\n=== " << title << " (" << v.size() << ") ===\n";
    for (size_t i = 0; i < v.size(); ++i)
        cout << i + 1 << ") " << v[i] << "\n";
}

// ============================================================
// 3. Анкета: ввод + проверка + генератор
// ============================================================

// 3a. Ввод пользовательских данных
void inputUser(string& surname, string& name, string& patronymic,
               string& birth, string& phone, string& email) {
    cout << "Введите: Фамилия Имя Отчество ДатаРождения Телефон Почта\n> ";
    cin >> surname >> name >> patronymic >> birth >> phone >> email;
}

// Вспомогательная: первая буква заглавная, остальные строчные
string normalizeWord(const string& w) {
    if (w.empty()) return w;
    string r = w;
    r[0] = toupper((unsigned char)r[0]);
    for (size_t i = 1; i < r.size(); ++i)
        r[i] = tolower((unsigned char)r[i]);
    return r;
}

// 3b. Проверка и приведение к шаблону
// Возвращает true, если все поля удалось привести к корректному виду.
bool validateUser(string& surname, string& name, string& patronymic,
                  string& birth, string& phone, string& email) {
    bool ok = true;

    // --- ФИО: только буквы (кириллица/латиница), дефис; приводим регистр ---
    regex nameRe("^[A-Za-zА-Яа-яЁё-]+$");
    if (!regex_match(surname, nameRe))    { cout << "Ошибка в фамилии\n";  ok = false; }
    if (!regex_match(name, nameRe))       { cout << "Ошибка в имени\n";    ok = false; }
    if (!regex_match(patronymic, nameRe)) { cout << "Ошибка в отчестве\n"; ok = false; }
    surname    = normalizeWord(surname);
    name       = normalizeWord(name);
    patronymic = normalizeWord(patronymic);

    // --- Дата рождения: принимаем YYYY-MM-DD, DD.MM.YYYY, DD/MM/YYYY ---
    // приводим к DD.MM.YYYY
    {
        smatch m;
        regex reYMD("^(\\d{4})-(\\d{2})-(\\d{2})$");
        regex reDMY("^(\\d{2})[./](\\d{2})[./](\\d{4})$");
        if (regex_match(birth, m, reYMD)) {
            birth = m[3].str() + "." + m[2].str() + "." + m[1].str();
        } else if (regex_match(birth, m, reDMY)) {
            birth = m[1].str() + "." + m[2].str() + "." + m[3].str();
        } else {
            cout << "Ошибка в дате рождения\n";
            ok = false;
        }
    }

    // --- Телефон: убираем все разделители, приводим к +7XXXXXXXXXX ---
    {
        string digits;
        for (char c : phone)
            if (isdigit((unsigned char)c)) digits += c;
        // ожидаем 11 цифр, начинающихся с 7 или 8
        if (digits.size() == 11 && (digits[0] == '7' || digits[0] == '8')) {
            digits[0] = '7';
            phone = "+" + digits;
        } else {
            cout << "Ошибка в телефоне\n";
            ok = false;
        }
    }

    // --- Почта: проверка формата и приведение к нижнему регистру ---
    {
        regex emailRe("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
        if (regex_match(email, emailRe)) {
            for (char& c : email) c = tolower((unsigned char)c);
        } else {
            cout << "Ошибка в почте\n";
            ok = false;
        }
    }
    return ok;
}

// 3c. Генератор случайных данных
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

    // дата: 1970-01-01 .. 2005-12-31
    int year  = 1970 + rand() % 36;
    int month = 1 + rand() % 12;
    int day   = 1 + rand() % 28;
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    birth = buf;

    // телефон: +7996-XXXX-XXX
    snprintf(buf, sizeof(buf), "+7996-%04d-%03d", rand() % 10000, rand() % 1000);
    phone = buf;

    // почта: фамилия в нижнем регистре + случайное число
    string lower = surname;
    for (char& c : lower) c = tolower((unsigned char)c);
    snprintf(buf, sizeof(buf), "%s%d@pochta.ru", lower.c_str(), rand() % 1000);
    email = buf;
}

// ============================================================
// main
// ============================================================
int main() {
    SetConsoleCP(65001);       // Устанавливаем кодировку ввода UTF-8
    SetConsoleOutputCP(65001); // Устанавливаем кодировку вывода UTF-8
    srand((unsigned)time(nullptr));

    // ---------- 1 и 2: HTML ----------
    string html =
        "<html><body>"
        "<h1>Заголовок</h1>"
        "<p>Первый абзац.</p>"
        "<p>Второй абзац.</p>"
        "<ul><li>пункт 1</li><li>пункт 2</li></ul>"
        "<ol><li>раз</li><li>два</li></ol>"
        "</body></html>";

    if (checkHtml(html))
        cout << "HTML корректен.\n";

    printVector("Заголовки", findHeaders(html));
    printVector("Абзацы",    findParagraphs(html));
    printVector("Списки",    findLists(html));

    // Проверка «плохого» текста
    cout << "\n--- Проверка Markdown-текста ---\n";
    checkHtml("# Заголовок\n**жирный**");

    // ---------- 3: анкета ----------
    cout << "\n--- Анкета ---\n";
    string s, n, p, b, ph, e;

    cout << "1) Ручной ввод\n";
    inputUser(s, n, p, b, ph, e);
    if (validateUser(s, n, p, b, ph, e))
        cout << "Результат: " << s << " " << n << " " << p << " "
             << b << " " << ph << " " << e << "\n";
    else
        cout << "Данные некорректны.\n";

    cout << "\n2) Сгенерированные данные\n";
    generateUser(s, n, p, b, ph, e);
    cout << "Сгенерировано: " << s << " " << n << " " << p << " "
         << b << " " << ph << " " << e << "\n";
    if (validateUser(s, n, p, b, ph, e))
        cout << "После проверки: " << s << " " << n << " " << p << " "
             << b << " " << ph << " " << e << "\n";

    return 0;
}