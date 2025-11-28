#include <iostream>
#include "Board.h"
#include "Block.h"
#include "ActionSimulator.h"
#include "FeatureExtractor.h"

using namespace std;

/**
 * Action Simulator 테스트 프로그램
 * 블록의 가능한 액션들을 시뮬레이션하고 결과를 출력합니다.
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

void testGenerateActions()
{
    cout << "\n### Test 1: Generate Possible Actions ###" << endl;
    
    // 각 블록 타입에 대해 가능한 액션 수 확인
    BlockType types[] = {BlockType::I, BlockType::O, BlockType::T, 
                         BlockType::S, BlockType::Z, BlockType::J, BlockType::L};
    const char* typeNames[] = {"I", "O", "T", "S", "Z", "J", "L"};
    
    for (int i = 0; i < 7; i++)
    {
        vector<Action> actions = ActionSimulator::generatePossibleActions(types[i]);
        cout << "Block " << typeNames[i] << ": " << actions.size() << " possible actions" << endl;
    }
    
    cout << "\nExpected:" << endl;
    cout << "  I, S, Z: 24 actions (2 rotations × 12 columns)" << endl;
    cout << "  O: 12 actions (1 rotation × 12 columns)" << endl;
    cout << "  T, J, L: 48 actions (4 rotations × 12 columns)" << endl;
}

void testSimulateAction()
{
    cout << "\n### Test 2: Simulate Single Action ###" << endl;
    
    // 빈 보드 생성
    Board board(true);
    board.init();
    
    // I 블록 생성
    Block block(BlockType::I);
    
    // 중앙에 수평으로 놓는 액션 시뮬레이션
    Action action(0, 5); // 회전 0, 열 5
    
    cout << "\nSimulating I block at rotation 0, column 5..." << endl;
    SimulationResult result = ActionSimulator::simulateAction(board, block, action);
    
    if (result.isValid)
    {
        cout << "✓ Action is valid!" << endl;
        cout << "Lines cleared: " << result.linesCleared << endl;
        FeatureExtractor::printFeatures(result.features);
    }
    else
    {
        cout << "✗ Action is invalid!" << endl;
        if (result.gameOver)
        {
            cout << "  (Game Over)" << endl;
        }
    }
}

void testSimulateAllActions()
{
    cout << "\n### Test 3: Simulate All Actions ###" << endl;
    
    // 빈 보드 생성
    Board board(true);
    board.init();
    
    // T 블록 생성
    Block block(BlockType::T);
    
    cout << "\nSimulating all possible actions for T block..." << endl;
    vector<SimulationResult> results = ActionSimulator::simulateAllActions(board, block);
    
    cout << "Valid actions found: " << results.size() << endl;
    
    // 가장 좋은 액션 찾기 (구멍이 가장 적고, 높이가 낮은)
    if (!results.empty())
    {
        SimulationResult best = results[0];
        for (const auto& result : results)
        {
            if (result.features.holes < best.features.holes ||
                (result.features.holes == best.features.holes && 
                 result.features.maxHeight < best.features.maxHeight))
            {
                best = result;
            }
        }
        
        cout << "\nBest action found:" << endl;
        cout << "  Rotation: " << best.action.rotation << endl;
        cout << "  Column: " << best.action.column << endl;
        cout << "  Features:" << endl;
        cout << "    Holes: " << best.features.holes << endl;
        cout << "    Max Height: " << best.features.maxHeight << endl;
        cout << "    Aggregate Height: " << best.features.aggregateHeight << endl;
        cout << "    Bumpiness: " << best.features.bumpiness << endl;
    }
}

void testWithPartiallyFilledBoard()
{
    cout << "\n### Test 4: Simulate on Partially Filled Board ###" << endl;
    
    // 보드 생성 및 초기화
    Board board(true);
    board.init();
    
    cout << "Note: This test would require manual board manipulation" << endl;
    cout << "to create a partially filled board state." << endl;
    cout << "In actual usage, the board will have blocks from previous moves." << endl;
}

int main()
{
    cout << "======================================" << endl;
    cout << "Action Simulator Test Program" << endl;
    cout << "======================================" << endl;
    
    testGenerateActions();
    testSimulateAction();
    testSimulateAllActions();
    testWithPartiallyFilledBoard();
    
    cout << "\n======================================" << endl;
    cout << "All Tests Complete!" << endl;
    cout << "======================================" << endl;
    
    return 0;
}

