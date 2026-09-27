#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

const int MIN_N = 1;
const int MAX_N = 10000;
const int MIN_VAL = -46000;
const int MAX_VAL = 46000;

int readInt(const char* prompt) {
    int value;
    while (true) {
        printf("%s", prompt);
        fflush(stdout);
        if (scanf("%d", &value) == 1)
            return value;
        printf("Ошибка: введите целое число.\n");
        while (getchar() != '\n');
    }
}

int readIntInRange(const char* prompt, int lo, int hi) {
    while (true) {
        int value = readInt(prompt);
        if (value >= lo && value <= hi)
            return value;
        printf("Ошибка: число должно быть в диапазоне [%d, %d].\n", lo, hi);
    }
}

int readIntMax(const char* prompt, int maxVal) {
    while (true) {
        int value = readInt(prompt);
        if (value <= maxVal)
            return value;
        printf("Ошибка: число не должно превышать %d.\n", maxVal);
    }
}

int readIntMin(const char* prompt, int minVal) {
    while (true) {
        int value = readInt(prompt);
        if (value >= minVal)
            return value;
        printf("Ошибка: число должно быть не меньше %d.\n", minVal);
    }
}

void countWords(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        printf("Не удалось открыть файл %s\n", filename.c_str());
        return;
    }

    map<string, int> counter;
    string word;

    while (file >> word) {
        string clean;
        for (char c : word) {
            unsigned char uc = (unsigned char)c;
            if (isalpha(uc) || uc >= 128)
                clean += (char)tolower(uc);
        }
        if (!clean.empty())
            counter[clean]++;
    }

    file.close();

    for (const auto& p : counter)
        printf("%s - %d\n", p.first.c_str(), p.second);
}

map<string, vector<int>> indexWords(const string& filename) {
    map<string, vector<int>> positions;
    ifstream file(filename);

    if (!file.is_open()) {
        printf("Не удалось открыть файл %s\n", filename.c_str());
        return positions;
    }

    string word;
    int pos = 0;

    while (file >> word) {
        string clean;
        for (char c : word) {
            unsigned char uc = (unsigned char)c;
            if (isalpha(uc) || uc >= 128)
                clean += (char)tolower(uc);
        }
        if (!clean.empty()) {
            positions[clean].push_back(pos);
            pos++;
        }
    }

    file.close();
    return positions;
}

void printIndex(const map<string, vector<int>>& positions) {
    for (const auto& p : positions) {
        printf("%s -- ", p.first.c_str());
        const vector<int>& vec = p.second;
        for (size_t i = 0; i < vec.size(); i++) {
            printf("%d", vec[i]);
            if (i + 1 < vec.size())
                printf(", ");
        }
        printf("\n");
    }
}

void printVector(const vector<int>& v) {
    for (int x : v)
        printf("%d ", x);
    printf("\n");
}

bool isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

vector<int> generateVector(int n, int lo, int hi) {
    vector<int> v;
    for (int i = 0; i < n; i++)
        v.push_back(rand() % (hi - lo + 1) + lo);
    return v;
}

void readGenerationParams(int& n, int& lo, int& hi) {
    n = readIntInRange("Сколько чисел? ", MIN_N, MAX_N);

    while (true) {
        lo = readIntInRange("Минимальное значение [-46000, 46000]: ", MIN_VAL, MAX_VAL);
        hi = readIntInRange("Максимальное значение [-46000, 46000]: ", MIN_VAL, MAX_VAL);

        if (lo <= hi)
            break;
        printf("Ошибка: минимум не может быть больше максимума.\n");
    }
}

void task3a() {
    int n, lo, hi;
    readGenerationParams(n, lo, hi);

    vector<int> v = generateVector(n, lo, hi);

    printf("\nДо:    ");
    printVector(v);

    for_each(v.begin(), v.end(), [](int& x) {
        if (isPrime(x)) x *= x;
    });

    printf("После: ");
    printVector(v);
}

void task3b() {
    int n, lo, hi;
    readGenerationParams(n, lo, hi);

    vector<int> v = generateVector(n, lo, hi);

    printf("\nДо:    ");
    printVector(v);

    sort(v.begin(), v.end(), [](int a, int b) {
        if (a % 2 != 0 && b % 2 != 0) return a < b;
        if (a % 2 == 0 && b % 2 == 0) return a > b;
        return a % 2 != 0;
    });

    printf("После: ");
    printVector(v);
}

void task3c() {
    int n, lo, hi;
    readGenerationParams(n, lo, hi);

    vector<int> v = generateVector(n, lo, hi);

    printf("\nИсходный: ");
    printVector(v);

    int rlo, rhi;
    while (true) {
        rlo = readIntInRange("Диапазон поиска, от: ", MIN_VAL, MAX_VAL);
        rhi = readIntInRange("Диапазон поиска, до: ", MIN_VAL, MAX_VAL);

        if (rlo <= rhi)
            break;
        printf("Ошибка: начало диапазона не может быть больше конца.\n");
    }

    set<int> result;
    for (int x : v)
        if (x >= rlo && x <= rhi)
            result.insert(x);

    vector<int> res(result.begin(), result.end());

    printf("Уникальные в [%d, %d]: ", rlo, rhi);
    printVector(res);
}

void task3Menu() {
    int choice;
    do {
        printf("\n--- Задание 3 ---\n");
        printf("1. Простые числа в квадрат\n");
        printf("2. Сортировка (нечётные ↑, чётные ↓)\n");
        printf("3. Уникальные числа в диапазоне\n");
        printf("0. Назад\n");
        printf("Выбор: ");
        fflush(stdout);
        scanf("%d", &choice);

        switch (choice) {
            case 1: task3a(); break;
            case 2: task3b(); break;
            case 3: task3c(); break;
            case 0: break;
            default: printf("Неверный выбор\n");
        }
    } while (choice != 0);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setvbuf(stdout, NULL, _IONBF, 0);
    srand((unsigned)time(NULL));

    string filename = "Tolstoy.txt";

    int choice;
    do {
        printf("\n===== МЕНЮ =====\n");
        printf("1. Подсчёт уникальных слов\n");
        printf("2. Индексация позиций слов\n");
        printf("3. Алгоритмы STL\n");
        printf("0. Выход\n");
        printf("Выбор: ");
        fflush(stdout);
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("\n=== Задание 1 ===\n");
                countWords(filename);
                break;
            case 2: {
                printf("\n=== Задание 2 ===\n");
                map<string, vector<int>> positions = indexWords(filename);
                printIndex(positions);
                break;
            }
            case 3:
                task3Menu();
                break;
            case 0:
                printf("Выход.\n");
                break;
            default:
                printf("Неверный выбор\n");
        }
    } while (choice != 0);

    return 0;
}