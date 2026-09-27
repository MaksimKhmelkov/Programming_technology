#include <iostream>
#include <fstream>
#include <map>
#include <vector>
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

map<string, vector<int>> indexWords(const string& filename){
    map<string, vector<int>> positions;
    ifstream file(filename);

    if(!file.is_open()){
        printf("Не удалось открыть файл %s\n", filename.c_str());
        return positions;
    }
     
    string word;
    int pos = 0;

    while(file >> word){
        string clean;
        for(char c : word){
            unsigned char uc = (unsigned char)c;
            if(isalpha(uc) || uc >= 128)
                clean +=(char)tolower(uc);
        }
        if(!clean.empty()){
            positions[clean].push_back(pos);
            pos++;
        }
    }

    file.close();
    return positions;
}

void printIndex(const map<string, vector<int>>& positions){
    for(const auto& p : positions){
        printf("%s --", p.first.c_str());
        const vector<int>& vec = p.second;
        for(size_t i = 0; i<vec.size(); i++){
            printf("%d", vec[i]);
            if(i+1<vec.size())
                printf(", ");
        }
        printf("\n");
    }
}

int main(){
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    string filename = "Tolstoy.txt";
    printf("task_1:\n");
    countWords(filename);

    printf("task_2:\n");
    map<string, vector<int>> positions = indexWords(filename);
    printIndex(positions);

    return 0;

}