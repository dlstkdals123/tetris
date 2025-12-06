#include "ScoreManager.h"
#include "Utils.h"
#include "GameConstants.h"
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
        // 파일 없으면 생성
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

    // 최신 점수 반영, 정렬
    sort(scores.begin(), scores.end(), greater<int>());

    // 필요하면 상위 100개 정도만 유지
    if (scores.size() > GameConstants::ScoreManager::MAX_SCORES) {
        scores.resize(GameConstants::ScoreManager::MAX_SCORES);
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

void ScoreManager::printTopN(int n, int x, int y)
{
    auto top = getTopN(n);

    Utils::setColor(COLOR::GRAY);

    int boxWidth = 14;

    // 상단
    Utils::gotoxy(x, y);
    printf("┏");
    for (int i = 0; i < boxWidth - 2; ++i) printf("━");
    printf("┓");

    // 제목줄
    Utils::gotoxy(x, y + 1);
    printf("┃ TOP %2d     ┃", n);

    // 구분선
    Utils::gotoxy(x, y + 2);
    printf("┣");
    for (int i = 0; i < boxWidth - 2; ++i) printf("━");
    printf("┫");

    // 점수
    for (int i = 0; i < n; ++i) {
        Utils::gotoxy(x, y + 3 + i);
        if (i < static_cast<int>(top.size())) {
            printf("┃%2d.%7d  ┃",
                i + GameConstants::ScoreManager::RANK_OFFSET,
                top[i]);
        }
        else {
            printf("┃            ┃");
        }
    }

    // 하단
    Utils::gotoxy(x, y + 3 + n);
    printf("┗");
    for (int i = 0; i < boxWidth - 2; ++i) printf("━");
    printf("┛");

    Utils::setColor(COLOR::WHITE);
}