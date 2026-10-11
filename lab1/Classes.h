#ifndef CLASSES_H
#define CLASSES_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

// ============================================================
// 1. FileReader — блочное чтение файла
// ============================================================
class FileReader {
public:
    static const size_t DEFAULT_BLOCK_SIZE = 8192;

    explicit FileReader(const std::string& path,
                        size_t blockSize = DEFAULT_BLOCK_SIZE);
    ~FileReader();

    bool open();
    void close();
    bool isOpen() const { return file_.is_open(); }

    // Возвращает количество прочитанных байт; 0 — конец файла
    size_t readBlock();

    const char* data() const { return buffer_.data(); }
    size_t size() const { return bytesRead_; }

private:
    std::string path_;
    std::ifstream file_;
    std::string buffer_;
    size_t blockSize_;
    size_t bytesRead_;
};

// ============================================================
// 2. WordCounter — задание 1
// ============================================================
class WordCounter {
public:
    explicit WordCounter(size_t blockSize = 0);

    bool processFile(const std::string& path);
    const std::map<std::string, int>& results() const { return counter_; }
    void print() const;

private:
    std::map<std::string, int> counter_;
    size_t blockSize_;
    std::string tail_;

    void handleChunk(const char* data, size_t size, bool lastChunk);
};

// ============================================================
// 3. WordIndexer — задание 2
// ============================================================
class WordIndexer {
public:
    explicit WordIndexer(size_t blockSize = 0);

    bool processFile(const std::string& path);
    const std::map<std::string, std::vector<int>>& index() const { return positions_; }
    void print() const;

private:
    std::map<std::string, std::vector<int>> positions_;
    size_t blockSize_;
    int pos_;
    std::string tail_;

    void handleChunk(const char* data, size_t size, bool lastChunk);
};

// ============================================================
// 4. VectorAlgorithms — задание 3
// ============================================================
class VectorAlgorithms {
public:
    static const int MIN_N   = 1;
    static const int MAX_N   = 10000;
    static const int MIN_VAL = -46000;
    static const int MAX_VAL =  46000;

    static std::vector<int> generate(int n, int lo, int hi);
    static void print(const std::vector<int>& v);
    static void primeToSquare(std::vector<int>& v);
    static void sortOddEven(std::vector<int>& v);
    static std::vector<int> uniqueInRange(const std::vector<int>& v,
                                          int lo, int hi);
private:
    static bool isPrime(int n);
};

// ============================================================
// 5. ConsoleInput — ввод
// ============================================================
class ConsoleInput {
public:
    static int readInt(const char* prompt);
    static int readIntInRange(const char* prompt, int lo, int hi);
};

// ============================================================
// 6. Menu — пользовательский интерфейс
// ============================================================
class Menu {
public:
    explicit Menu(const std::string& filename);
    void run();

private:
    std::string filename_;

    void task1();
    void task2();
    void task3();
    void task3Menu();
    void readGenerationParams(int& n, int& lo, int& hi);
};

#endif // CLASSES_H