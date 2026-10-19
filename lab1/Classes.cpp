#include "Classes.h"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <set>

FileReader::FileReader(const std::string& path, size_t blockSize)
    : path_(path)
    , blockSize_(blockSize == 0 ? DEFAULT_BLOCK_SIZE : blockSize)
    , bytesRead_(0)
{
    buffer_.resize(blockSize_);
}

FileReader::~FileReader() { close(); }

bool FileReader::open() {
    if (file_.is_open()) return true;
    file_.open(path_, std::ios::binary);
    return file_.is_open();
}

void FileReader::close() {
    if (file_.is_open()) file_.close();
}

size_t FileReader::readBlock() {
    if (!file_.is_open()) { bytesRead_ = 0; return 0; }
    file_.read(&buffer_[0], static_cast<std::streamsize>(blockSize_));
    bytesRead_ = static_cast<size_t>(file_.gcount());
    return bytesRead_;
}


static inline char normalizeChar(unsigned char c) {
    if (c < 128) return static_cast<char>(std::tolower(c));
    return static_cast<char>(c);
}
static inline bool isWordChar(unsigned char c) {
    return std::isalpha(c) || c >= 128;
}

WordCounter::WordCounter(size_t blockSize) : blockSize_(blockSize) {}

void WordCounter::handleChunk(const char* data, size_t size, bool lastChunk) {
    std::string current = tail_;
    tail_.clear();

    for (size_t i = 0; i < size; ++i) {
        unsigned char uc = static_cast<unsigned char>(data[i]);
        if (isWordChar(uc)) current += normalizeChar(uc);
        else if (!current.empty()) { counter_[current]++; current.clear(); }
    }

    if (lastChunk) {
        if (!current.empty()) counter_[current]++;
    } else {
        tail_ = current;
    }
}

bool WordCounter::processFile(const std::string& path) {
    counter_.clear(); tail_.clear();

    FileReader reader(path, blockSize_);
    if (!reader.open()) {
        std::printf("Не удалось открыть файл %s\n", path.c_str());
        return false;
    }
    size_t n;
    while ((n = reader.readBlock()) > 0)
        handleChunk(reader.data(), n, false);
    handleChunk(nullptr, 0, true);
    reader.close();
    return true;
}

void WordCounter::print(const std::string& outFilename) const {
    std::ofstream out(outFilename);
    
    if (!out.is_open()){
        std::printf("Не удалось открыть файл для записи: %s\n", outFilename.c_str());
        return;
    }

    for (const auto& p : counter_){
        out << p.first << ' - ' << p.second << "\n";
    }
    
    std::printf("Результат задания 1 записан в файл: %s\n", outFilename.c_str());
}

WordIndexer::WordIndexer(size_t blockSize)
    : blockSize_(blockSize), pos_(0) {}

void WordIndexer::handleChunk(const char* data, size_t size, bool lastChunk) {
    std::string current = tail_;
    tail_.clear();

    for (size_t i = 0; i < size; ++i) {
        unsigned char uc = static_cast<unsigned char>(data[i]);
        if (isWordChar(uc)) current += normalizeChar(uc);
        else if (!current.empty()) { positions_[current].push_back(pos_++); current.clear(); }
    }

    if (lastChunk) {
        if (!current.empty()) positions_[current].push_back(pos_++);
    } else {
        tail_ = current;
    }
}

bool WordIndexer::processFile(const std::string& path) {
    positions_.clear(); tail_.clear(); pos_ = 0;

    FileReader reader(path, blockSize_);
    if (!reader.open()) {
        std::printf("Не удалось открыть файл %s\n", path.c_str());
        return false;
    }
    size_t n;
    while ((n = reader.readBlock()) > 0)
        handleChunk(reader.data(), n, false);
    handleChunk(nullptr, 0, true);
    reader.close();
    return true;
}

void WordIndexer::print(const std::string& outFilename) const {
    std::ofstream out(outFilename);
    
    if (!out.is_open()) {
        std::printf("Не удалось открыть файл для записи: %s\n", outFilename.c_str());
        return;
    }

    for (const auto& p : positions_) {
        out << p.first << " -- ";
        for (size_t i = 0; i < p.second.size(); ++i) {
            out << p.second[i];
            if (i + 1 < p.second.size()) out << ", ";
        }
        out << "\n";
    }
    
    std::printf("Результат задания 2 записан в файл: %s\n", outFilename.c_str());
}

bool VectorAlgorithms::isPrime(int n) {
    if (n < 2) return false;
    for (int i = 2; i * i <= n; ++i)
        if (n % i == 0) return false;
    return true;
}

std::vector<int> VectorAlgorithms::generate(int n, int lo, int hi) {
    std::vector<int> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i)
        v.push_back(std::rand() % (hi - lo + 1) + lo);
    return v;
}

void VectorAlgorithms::print(const std::vector<int>& v) {
    for (int x : v) std::printf("%d ", x);
    std::printf("\n");
}

void VectorAlgorithms::primeToSquare(std::vector<int>& v) {
    std::for_each(v.begin(), v.end(), [](int& x) {
        if (isPrime(x)) x *= x;
    });
}

void VectorAlgorithms::sortOddEven(std::vector<int>& v) {
    std::sort(v.begin(), v.end(), [](int a, int b) {
        if (a % 2 != 0 && b % 2 != 0) return a < b;
        if (a % 2 == 0 && b % 2 == 0) return a > b;
        return a % 2 != 0;
    });
}

std::vector<int> VectorAlgorithms::uniqueInRange(const std::vector<int>& v,
                                                 int lo, int hi) {
    std::set<int> result;
    for (int x : v)
        if (x >= lo && x <= hi) result.insert(x);
    return std::vector<int>(result.begin(), result.end());
}

int ConsoleInput::readInt(const char* prompt) {
    int value;
    while (true) {
        std::printf("%s", prompt);
        std::fflush(stdout);
        if (std::scanf("%d", &value) == 1) return value;
        std::printf("Ошибка: введите целое число.\n");
        while (std::getchar() != '\n');
    }
}

int ConsoleInput::readIntInRange(const char* prompt, int lo, int hi) {
    while (true) {
        int value = readInt(prompt);
        if (value >= lo && value <= hi) return value;
        std::printf("Ошибка: число должно быть в диапазоне [%d, %d].\n", lo, hi);
    }
}

Menu::Menu(const std::string& filename) : filename_(filename) {}

void Menu::task1() {
    std::printf("\nЗадание 1\n");
    WordCounter counter;
    if (counter.processFile(filename_)) counter.print("result1.txt");
}

void Menu::task2() {
    std::printf("\nЗадание 2\n");
    WordIndexer indexer;
    if (indexer.processFile(filename_)) indexer.print("result2.txt");
}

void Menu::readGenerationParams(int& n, int& lo, int& hi) {
    using VA = VectorAlgorithms;
    n = ConsoleInput::readIntInRange("Сколько чисел? ", VA::MIN_N, VA::MAX_N);
    while (true) {
        lo = ConsoleInput::readIntInRange(
                "Минимальное значение [-46000, 46000]: ",
                VA::MIN_VAL, VA::MAX_VAL);
        hi = ConsoleInput::readIntInRange(
                "Максимальное значение [-46000, 46000]: ",
                VA::MIN_VAL, VA::MAX_VAL);
        if (lo <= hi) break;
        std::printf("Ошибка: минимум не может быть больше максимума.\n");
    }
}

void Menu::task3Menu() {
    int choice;
    do {
        std::printf("\nЗадание 3\n");
        std::printf("1. Простые числа в квадрат\n");
        std::printf("2. Сортировка\n");
        std::printf("3. Уникальные числа в диапазоне\n");
        std::printf("0. Назад\n");
        std::printf("Выбор: ");
        std::fflush(stdout);
        std::scanf("%d", &choice);

        switch (choice) {
        case 1: {
            int n, lo, hi;
            readGenerationParams(n, lo, hi);
            auto v = VectorAlgorithms::generate(n, lo, hi);
            std::printf("\nДо:    ");
            VectorAlgorithms::print(v);
            VectorAlgorithms::primeToSquare(v);
            std::printf("После: ");
            VectorAlgorithms::print(v);
            break;
        }
        case 2: {
            int n, lo, hi;
            readGenerationParams(n, lo, hi);
            auto v = VectorAlgorithms::generate(n, lo, hi);
            std::printf("\nДо:    ");
            VectorAlgorithms::print(v);
            VectorAlgorithms::sortOddEven(v);
            std::printf("После: ");
            VectorAlgorithms::print(v);
            break;
        }
        case 3: {
            int n, lo, hi;
            readGenerationParams(n, lo, hi);
            auto v = VectorAlgorithms::generate(n, lo, hi);
            std::printf("\nИсходный: ");
            VectorAlgorithms::print(v);

            int rlo, rhi;
            while (true) {
                rlo = ConsoleInput::readIntInRange(
                        "Диапазон поиска, от: ",
                        VectorAlgorithms::MIN_VAL, VectorAlgorithms::MAX_VAL);
                rhi = ConsoleInput::readIntInRange(
                        "Диапазон поиска, до: ",
                        VectorAlgorithms::MIN_VAL, VectorAlgorithms::MAX_VAL);
                if (rlo <= rhi) break;
                std::printf("Ошибка: начало диапазона не может быть больше конца.\n");
            }
            auto res = VectorAlgorithms::uniqueInRange(v, rlo, rhi);
            std::printf("Уникальные в [%d, %d]: ", rlo, rhi);
            VectorAlgorithms::print(res);
            break;
        }
        case 0: break;
        default: std::printf("Неверный выбор\n");
        }
    } while (choice != 0);
}

void Menu::task3() { task3Menu(); }

void Menu::run() {
    int choice;
    do {
        std::printf("1. Подсчёт уникальных слов\n");
        std::printf("2. Индексация позиций слов\n");
        std::printf("3. Алгоритмы STL\n");
        std::printf("0. Выход\n");
        std::printf("Выбор: ");
        std::fflush(stdout);
        std::scanf("%d", &choice);

        switch (choice) {
        case 1: task1(); break;
        case 2: task2(); break;
        case 3: task3(); break;
        case 0: std::printf("Выход.\n"); break;
        default: std::printf("Неверный выбор\n");
        }
    } while (choice != 0);
}