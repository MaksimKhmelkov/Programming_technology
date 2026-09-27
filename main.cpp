#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cctype>
#include <cstdio>
#include <windows.h>

using namespace std;

void countWords(const string& filename){
    ifstream file(filename);
    if (!file.is_open()){
        printf("Не удалось открыть файл %s\n", filename.c_str());
        return;
    }

    map<string, int> counter;
    string word;

    while(file >> word){
        string clean;
        for(char c:word){
            unsigned char uc = (unsigned char)c;
            if(isalpha(uc) || uc >= 128)
                clean += (char)tolower(uc); 
        }
        if(!clean.empty())
            counter[clean]++;
    }

    file.close();

    for(const auto& p:counter)
        printf("%s - %d\n", p.first.c_str(), p.second);

}

int main(){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    string filename = "Tolstoy.txt";
    countWords(filename);

    return 0;

}