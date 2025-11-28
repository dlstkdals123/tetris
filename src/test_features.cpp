#include <iostream>
#include "Board.h"
#include "FeatureExtractor.h"

using namespace std;

/**
 * Feature 추출 테스트 프로그램
 * 빈 보드에서 feature가 올바르게 추출되는지 확인합니다.
 */

void printBoard(const Board& board)
{
    cout << "\n=== Board State ===" << endl;
    for (int i = 0; i < 21; i++)
    {
        for (int j = 0; j < 14; j++)
        {
            if (board.getCell(i, j) == 1)
                cout << "■";
            else
                cout << "  ";
        }
        cout << endl;
    }
    cout << "===================" << endl;
}

void testEmptyBoard()
{
    cout << "\n### Test 1: Empty Board ###" << endl;
    Board board(true);
    board.init();
    
    printBoard(board);
    
    FeatureExtractor::Features features = FeatureExtractor::extractFeatures(board);
    FeatureExtractor::printFeatures(features);
    
    // 예상 결과: 모든 feature가 0이어야 함
    cout << "Expected: All features should be 0" << endl;
    
    // 검증
    bool passed = (features.aggregateHeight == 0 && 
                   features.completeLines == 0 && 
                   features.holes == 0 && 
                   features.bumpiness == 0 && 
                   features.maxHeight == 0);
    
    if (passed)
        cout << "✓ TEST PASSED" << endl;
    else
        cout << "✗ TEST FAILED" << endl;
}

void testColumnHeight()
{
    cout << "\n### Test 2: Column Height Test ###" << endl;
    Board board(true);
    board.init();
    
    cout << "Testing column heights on empty board:" << endl;
    for (int col = 1; col <= 12; col++)
    {
        int height = FeatureExtractor::getColumnHeight(board, col);
        cout << "Column " << col << ": " << height << endl;
    }
    cout << "Expected: All heights should be 0" << endl;
    
    bool passed = true;
    for (int col = 1; col <= 12; col++)
    {
        if (FeatureExtractor::getColumnHeight(board, col) != 0)
        {
            passed = false;
            break;
        }
    }
    
    if (passed)
        cout << "✓ TEST PASSED" << endl;
    else
        cout << "✗ TEST FAILED" << endl;
}

int main()
{
    cout << "======================================" << endl;
    cout << "Feature Extractor Test Program" << endl;
    cout << "======================================" << endl;
    
    testEmptyBoard();
    testColumnHeight();
    
    cout << "\n======================================" << endl;
    cout << "Basic Tests Complete!" << endl;
    cout << "======================================" << endl;
    cout << "\nNote: To fully test with actual game states," << endl;
    cout << "integrate with tetris gameplay or manually" << endl;
    cout << "manipulate board states." << endl;
    
    return 0;
}

