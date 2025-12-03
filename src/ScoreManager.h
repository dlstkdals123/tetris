#pragma once
#include <vector>
#include <string>
using namespace std;

class ScoreManager {
private:
    string fileName;
    vector<int> scores;

    void loadFromFile();
    void saveToFile() const;
public:
    explicit ScoreManager(const string& fileName);

    void addScore(int score);
    vector<int> getTopN(int n) const;
    void printTopN(int n, int x, int y);
};