#include "ScoreManager.h"
#include "Utils.h"
#include <fstream>
#include <algorithm>

ScoreManager::ScoreManager(const std::string& fileName): fileName(fileName)
{
    loadFromFile();
}

void ScoreManager::loadFromFile()
{
    scores.clear();
    ifstream fin(fileName);

    if (!fin.is_open()) {
        // 파일이 없으면 생성만 하고 끝냄
        ofstream createFile(fileName);
        return;
    }

    int value;
    while (fin >> value) {
        scores.push_back(value);
    }

    fin.close();

    // 내림차순 정렬
    std::sort(scores.begin(), scores.end(), greater<int>());
    if (scores.size() > 10) {
        scores.resize(10);
    }
}

void ScoreManager::saveToFile() const
{
    ofstream fout(fileName, ios::trunc);

    for (int s : scores) {
        fout << s << "\n";
    }

    fout.close();
}

void ScoreManager::addScore(int score)
{
    scores.push_back(score);

    // 최신 점수 반영
    sort(scores.begin(), scores.end(), greater<int>());

    if (scores.size() > 10) {
        scores.resize(10);
    }

    saveToFile();
}

vector<int> ScoreManager::getTopN(int n) const
{
    vector<int> top;
    int size = static_cast<int>(scores.size());
    for (int i = 0; i < n && i < size; ++i) {
        top.push_back(scores[i]);
    }
    return top;
    
}

void ScoreManager::printTopN(int n, int x, int y, bool isPlayer)
{
    auto top = getTopN(n);

    Utils::setColor(COLOR::GRAY);
    Utils::gotoxy(x, y, isPlayer);
    printf("=== TOP %d ===", n);

    for (int i = 0; i < top.size(); ++i) {
        Utils::gotoxy(x, y + 1 + i, isPlayer);
        printf("%d. %d", i + 1, top[i]);
    }

    Utils::setColor(COLOR::WHITE);
}