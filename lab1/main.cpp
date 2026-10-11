#include "Classes.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Menu menu("Tolstoy.txt");
    menu.run();
    return 0;
}