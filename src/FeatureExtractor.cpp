#include "FeatureExtractor.h"
#include "BoardConstants.h"
#include <iostream>
#include <cmath>

using namespace std;

FeatureExtractor::Features FeatureExtractor::extractFeatures(const Board& board)
{
    Features features;
    
    // 각 열의 높이를 저장 (bumpiness 계산에 재사용)
    int columnHeights[BoardConstants::PLAY_WIDTH];
    
    int totalHeight = 0;
    int maxH = 0;
    int minH = BoardConstants::PLAY_HEIGHT;
    
    // 각 열의 높이 계산
    for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
    {
        int height = getColumnHeight(board, col);
        columnHeights[col - 1] = height;
        totalHeight += height;
        
        if (height > maxH)
            maxH = height;
        if (height < minH)
            minH = height;
    }
    
    features.aggregateHeight = totalHeight;
    features.maxHeight = maxH;
    features.minHeight = minH;
    
    // Bumpiness 계산 (저장된 높이 배열 사용)
    int bumpiness = 0;
    for (int i = 0; i < BoardConstants::PLAY_WIDTH - 1; i++)
    {
        bumpiness += abs(columnHeights[i] - columnHeights[i + 1]);
    }
    features.bumpiness = bumpiness;
    
    // 구멍 개수 계산
    features.holes = countHoles(board);
    
    // 완성된 줄 개수 계산
    features.completeLines = countCompleteLines(board);
    
    // Feature 정규화 (0~1 범위로 스케일링)
    features.aggregateHeight /= 240.0;  // 최대 20*12 = 240
    features.completeLines /= 4.0;       // 최대 4줄
    features.holes /= 50.0;             // 최대 약 50개
    features.bumpiness /= 50.0;         // 최대 약 50
    features.maxHeight /= 20.0;          // 최대 20
    features.minHeight /= 20.0;          // 최대 20
    
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

int FeatureExtractor::countCompleteLines(const Board& board)
{
    int completeLines = 0;
    
    // 각 행에 대해
    for (int row = BoardConstants::MIN_ROW; row < BoardConstants::PLAY_HEIGHT; row++)
    {
        bool isComplete = true;
        
        // 플레이 영역의 모든 열 확인
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
        {
            if (board.getCell(row, col) == 0)
            {
                isComplete = false;
                break;
            }
        }
        
        if (isComplete)
        {
            completeLines++;
        }
    }
    
    return completeLines;
}

int FeatureExtractor::calculateBumpiness(const Board& board)
{
    int bumpiness = 0;
    
    // 인접한 열들의 높이 차이를 계산
    for (int col = BoardConstants::MIN_COLUMN; col < BoardConstants::MAX_COLUMN; col++)
    {
        int height1 = getColumnHeight(board, col);
        int height2 = getColumnHeight(board, col + 1);
        bumpiness += abs(height1 - height2);
    }
    
    return bumpiness;
}

void FeatureExtractor::printFeatures(const Features& features)
{
    cout << "=== Features ===" << endl;
    cout << "Aggregate Height: " << features.aggregateHeight << endl;
    cout << "Complete Lines:   " << features.completeLines << endl;
    cout << "Holes:            " << features.holes << endl;
    cout << "Bumpiness:        " << features.bumpiness << endl;
    cout << "Max Height:       " << features.maxHeight << endl;
    cout << "Min Height:       " << features.minHeight << endl;
    cout << "================" << endl;
}

