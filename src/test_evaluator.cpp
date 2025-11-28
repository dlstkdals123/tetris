#include <iostream>
#include "Board.h"
#include "Block.h"
#include "Evaluator.h"
#include "FeatureExtractor.h"

using namespace std;

/**
 * Evaluator 테스트 프로그램
 * Linear evaluation function의 동작을 테스트합니다.
 */

void testDefaultWeights()
{
    cout << "\n### Test 1: Default Weights ###" << endl;
    
    Evaluator evaluator;
    evaluator.printWeights();
    
    cout << "\nThese are heuristic-based initial weights." << endl;
    cout << "Negative weights: lower is better (height, holes, bumpiness)" << endl;
    cout << "Positive weights: higher is better (complete lines)" << endl;
}

void testEvaluateEmptyBoard()
{
    cout << "\n### Test 2: Evaluate Empty Board ###" << endl;
    
    Board board(true);
    board.init();
    
    Evaluator evaluator;
    double score = evaluator.evaluateBoard(board);
    
    cout << "Empty board score: " << score << endl;
    cout << "Expected: 0.0 (no features present)" << endl;
}

void testSelectBestAction()
{
    cout << "\n### Test 3: Select Best Action ###" << endl;
    
    Board board(true);
    board.init();
    
    // T 블록으로 테스트
    Block block(BlockType::T);
    
    Evaluator evaluator;
    
    cout << "\nFinding best action for T block on empty board..." << endl;
    auto [bestAction, bestScore] = evaluator.selectBestAction(board, block);
    
    cout << "\nBest action found:" << endl;
    cout << "  Rotation: " << bestAction.rotation << endl;
    cout << "  Column:   " << bestAction.column << endl;
    cout << "  Score:    " << bestScore << endl;
}

void testEvaluateAllActions()
{
    cout << "\n### Test 4: Evaluate All Actions ###" << endl;
    
    Board board(true);
    board.init();
    
    Block block(BlockType::I);
    
    Evaluator evaluator;
    
    cout << "\nEvaluating all actions for I block..." << endl;
    auto actionScores = evaluator.evaluateAllActions(board, block);
    
    cout << "Total valid actions: " << actionScores.size() << endl;
    
    // 상위 5개 액션 출력
    cout << "\nTop 5 actions:" << endl;
    for (size_t i = 0; i < min(size_t(5), actionScores.size()); i++)
    {
        cout << "  #" << (i+1) << ": ";
        cout << "Rot=" << actionScores[i].first.rotation;
        cout << ", Col=" << actionScores[i].first.column;
        cout << ", Score=" << actionScores[i].second << endl;
    }
    
    // 하위 3개 액션 출력
    if (actionScores.size() > 3)
    {
        cout << "\nWorst 3 actions:" << endl;
        for (size_t i = actionScores.size() - 3; i < actionScores.size(); i++)
        {
            cout << "  Rot=" << actionScores[i].first.rotation;
            cout << ", Col=" << actionScores[i].first.column;
            cout << ", Score=" << actionScores[i].second << endl;
        }
    }
}

void testCustomWeights()
{
    cout << "\n### Test 5: Custom Weights ###" << endl;
    
    // 라인 제거를 매우 중요하게 생각하는 가중치
    Evaluator::Weights customWeights(
        -0.5,  // aggregateHeight
        10.0,  // completeLines (매우 높게)
        -1.0,  // holes
        -0.2,  // bumpiness
        -0.5,  // maxHeight
        0.0    // minHeight
    );
    
    Evaluator evaluator(customWeights);
    
    cout << "\nCustom weights (prioritizing line clears):" << endl;
    evaluator.printWeights();
    
    Board board(true);
    board.init();
    Block block(BlockType::I);
    
    auto [bestAction, bestScore] = evaluator.selectBestAction(board, block);
    
    cout << "\nBest action with custom weights:" << endl;
    cout << "  Rotation: " << bestAction.rotation << endl;
    cout << "  Column:   " << bestAction.column << endl;
    cout << "  Score:    " << bestScore << endl;
}

void testSaveLoadWeights()
{
    cout << "\n### Test 6: Save and Load Weights ###" << endl;
    
    Evaluator evaluator1;
    
    cout << "\nOriginal weights:" << endl;
    evaluator1.printWeights();
    
    // 가중치 저장
    string filename = "test_weights.txt";
    if (evaluator1.saveWeights(filename))
    {
        cout << "\n✓ Weights saved successfully" << endl;
        
        // 새로운 evaluator에 로드
        Evaluator::Weights customWeights(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
        Evaluator evaluator2(customWeights);
        
        cout << "\nBefore loading (custom weights):" << endl;
        evaluator2.printWeights();
        
        if (evaluator2.loadWeights(filename))
        {
            cout << "\n✓ Weights loaded successfully" << endl;
            cout << "\nAfter loading:" << endl;
            evaluator2.printWeights();
        }
    }
}

void testCompareBlocks()
{
    cout << "\n### Test 7: Compare Different Blocks ###" << endl;
    
    Board board(true);
    board.init();
    
    Evaluator evaluator;
    
    BlockType types[] = {BlockType::I, BlockType::O, BlockType::T, BlockType::L};
    const char* names[] = {"I", "O", "T", "L"};
    
    cout << "\nBest action scores for different blocks on empty board:" << endl;
    
    for (int i = 0; i < 4; i++)
    {
        Block block(types[i]);
        auto [action, score] = evaluator.selectBestAction(board, block);
        
        cout << "  " << names[i] << " block: ";
        cout << "Score=" << score;
        cout << " (Rot=" << action.rotation << ", Col=" << action.column << ")" << endl;
    }
}

int main()
{
    cout << "======================================" << endl;
    cout << "Evaluator Test Program" << endl;
    cout << "======================================" << endl;
    
    testDefaultWeights();
    testEvaluateEmptyBoard();
    testSelectBestAction();
    testEvaluateAllActions();
    testCustomWeights();
    testSaveLoadWeights();
    testCompareBlocks();
    
    cout << "\n======================================" << endl;
    cout << "All Tests Complete!" << endl;
    cout << "======================================" << endl;
    
    return 0;
}

