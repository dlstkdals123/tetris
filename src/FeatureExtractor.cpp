#include "FeatureExtractor.h"
#include "BoardConstants.h"
#include <iostream>
#include <cmath>

using namespace std;

FeatureExtractor::Features FeatureExtractor::extractFeatures(const Board& board)
{
    Features features;
    
    // Bertsekas & Tsitsiklis 논문 스타일: 26개 feature
    
    // 1. 각 열의 높이 계산 (12개)
    int maxH = 0;
    for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
    {
        int height = getColumnHeight(board, col);
        features.columnHeights[col - BoardConstants::MIN_COLUMN] = height;
        if (height > maxH)
            maxH = height;
    }
    features.maxHeight = maxH;
    
    // 2. 인접한 열의 높이 차이 계산 (11개)
    for (int i = 0; i < 11; i++)
    {
        int h1 = features.columnHeights[i];
        int h2 = features.columnHeights[i + 1];
        features.heightDiffs[i] = abs(h1 - h2);
    }
    
    // 3. 구멍의 개수 (1개)
    features.holes = countHoles(board);
    
    // 4. 우물 깊이 합 (1개)
    features.wells = calculateWells(board);
    
    return features;
}

int FeatureExtractor::getColumnHeight(const Board& board, int col)
{
    if (col < BoardConstants::MIN_COLUMN || col > BoardConstants::MAX_COLUMN) 
        return 0;
    
    // 위에서부터 아래로 스캔하여 첫 번째 블록을 찾음
    for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
    {
        if (board.getCell(row, col) == 1)
        {
            // 높이는 바닥에서부터의 거리
            return BoardConstants::PLAY_HEIGHT - row;
        }
    }
    
    // 블록이 없으면 높이 0
    return 0;
}

int FeatureExtractor::countHoles(const Board& board)
{
    int holes = 0;
    
    // 각 열에 대해
    for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
    {
        bool blockFound = false;
        
        // 위에서 아래로 스캔
        for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
        {
            if (board.getCell(row, col) == 1)
            {
                blockFound = true;
            }
            else if (blockFound && board.getCell(row, col) == 0)
            {
                // 블록을 발견한 후 빈 공간이 나오면 구멍
                holes++;
            }
        }
    }
    
    return holes;
}


int FeatureExtractor::countRowTransitions(const Board& board)
{
    int transitions = 0;
    
    // 각 행에 대해
    for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
    {
        // 벽(0)과 첫 번째 칸 비교
        int prevCell = 1; // 왼쪽 벽은 채워진 것으로 간주
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
        {
            int currentCell = board.getCell(row, col);
            if (currentCell != prevCell)
            {
                transitions++;
            }
            prevCell = currentCell;
        }
        // 마지막 칸과 오른쪽 벽(13) 비교
        if (prevCell != 1) // 오른쪽 벽은 채워진 것으로 간주
        {
            transitions++;
        }
    }
    
    return transitions;
}

int FeatureExtractor::countColumnTransitions(const Board& board)
{
    int transitions = 0;
    
    // 각 열에 대해
    for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
    {
        // 바닥(20)과 첫 번째 칸 비교
        int prevCell = 1; // 바닥은 채워진 것으로 간주
        for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
        {
            int currentCell = board.getCell(row, col);
            if (currentCell != prevCell)
            {
                transitions++;
            }
            prevCell = currentCell;
        }
    }
    
    return transitions;
}

int FeatureExtractor::calculateWells(const Board& board)
{
    int totalWellDepth = 0;
    
    // 각 열에 대해
    for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
    {
        int wellDepth = 0;
        bool inWell = false;
        
        // 위에서 아래로 스캔
        for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
        {
            int currentCell = board.getCell(row, col);
            
            // 양옆이 막혀있는지 확인
            bool leftBlocked = (col == BoardConstants::MIN_COLUMN) || 
                              (board.getCell(row, col - 1) == 1);
            bool rightBlocked = (col == BoardConstants::MAX_COLUMN) || 
                               (board.getCell(row, col + 1) == 1);
            
            if (currentCell == 0 && leftBlocked && rightBlocked)
            {
                // 우물 안에 있음
                if (!inWell)
                {
                    inWell = true;
                    wellDepth = 0;
                }
                wellDepth++;
            }
            else if (currentCell == 1)
            {
                // 블록을 만남 - 우물 종료
                if (inWell)
                {
                    // 우물 깊이의 제곱 합 (Dellacherie 스타일)
                    totalWellDepth += wellDepth * (wellDepth + 1) / 2;
                    inWell = false;
                }
            }
        }
        
        // 우물이 끝까지 이어지는 경우
        if (inWell)
        {
            totalWellDepth += wellDepth * (wellDepth + 1) / 2;
        }
    }
    
    return totalWellDepth;
}

void FeatureExtractor::printFeatures(const Features& features)
{
    cout << "=== Features (Bertsekas & Tsitsiklis - 26 features) ===" << endl;
    
    // 1. 각 열의 높이 (12개)
    cout << "Column Heights: ";
    for (int i = 0; i < 12; i++) {
        cout << features.columnHeights[i];
        if (i < 11) cout << " ";
    }
    cout << endl;
    
    // 2. 인접한 열의 높이 차이 (11개)
    cout << "Height Diffs:   ";
    for (int i = 0; i < 11; i++) {
        cout << features.heightDiffs[i];
        if (i < 10) cout << " ";
    }
    cout << endl;
    
    // 3. 최대 높이 (1개)
    cout << "Max Height:     " << features.maxHeight << endl;
    
    // 4. 구멍의 개수 (1개)
    cout << "Holes:          " << features.holes << endl;
    
    // 5. 우물 깊이 합 (1개)
    cout << "Wells:          " << features.wells << endl;
    
    cout << "=======================================================" << endl;
}

