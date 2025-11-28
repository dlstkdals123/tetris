#include "ActionSimulator.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include <algorithm>

std::vector<Action> ActionSimulator::generatePossibleActions(BlockType blockType)
{
    std::vector<Action> actions;
    
    // 블록 타입에 따른 회전 수 결정
    int numRotations = 4;
    if (blockType == BlockType::O) {
        numRotations = 1; // O 블록은 회전해도 같음
    } else if (blockType == BlockType::I || blockType == BlockType::S || blockType == BlockType::Z) {
        numRotations = 2; // I, S, Z는 2개의 고유한 회전만 있음
    }
    
    // 각 회전에 대해
    for (int rot = 0; rot < numRotations; rot++)
    {
        // 각 열 위치에 대해
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
        {
            actions.push_back(Action(rot, col));
        }
    }
    
    return actions;
}

SimulationResult ActionSimulator::simulateAction(const Board& board, const Block& block, const Action& action)
{
    SimulationResult result;
    result.action = action;
    result.isValid = false;
    result.gameOver = false;
    result.linesCleared = 0;
    
    // 보드 복사
    Board simBoard(board);
    
    // 블록 복사 및 초기 위치 설정
    Block simBlock(block.getType(), Rotation(action.rotation), 
                   Position(action.column, BoardConstants::BLOCK_START_Y));
    
    // 블록을 목표 위치로 이동
    moveBlockToPosition(simBlock, action.rotation, action.column);
    
    // 블록이 유효한 위치인지 확인
    if (!isValidPosition(simBoard, simBlock))
    {
        return result; // isValid는 이미 false
    }
    
    // 블록을 바닥까지 떨어뜨림
    if (!dropBlock(simBoard, simBlock))
    {
        result.gameOver = true;
        return result; // isValid는 이미 false
    }
    
    // 블록을 보드에 병합
    simBoard.mergeBlock(simBlock);
    
    // 완성된 라인 제거
    result.linesCleared = simBoard.deleteFullLine();
    
    // Feature 추출
    result.features = FeatureExtractor::extractFeatures(simBoard);
    
    result.isValid = true;
    return result;
}

std::vector<SimulationResult> ActionSimulator::simulateAllActions(const Board& board, const Block& block)
{
    std::vector<SimulationResult> results;
    
    // 가능한 모든 액션 생성
    std::vector<Action> actions = generatePossibleActions(block.getType());
    
    // 각 액션 시뮬레이션
    for (const Action& action : actions)
    {
        SimulationResult result = simulateAction(board, block, action);
        if (result.isValid)
        {
            results.push_back(result);
        }
    }
    
    return results;
}

void ActionSimulator::moveBlockToPosition(Block& block, int rotation, int column)
{
    // 회전 설정
    for (int i = 0; i < rotation; i++)
    {
        block.rotate();
    }
    
    // 수평 위치 조정
    const Position& pos = block.getPos();
    int currentX = pos.getX();
    int targetX = column;
    
    int diff = targetX - currentX;
    if (diff > 0)
    {
        for (int i = 0; i < diff; i++)
        {
            block.moveRight();
        }
    }
    else if (diff < 0)
    {
        for (int i = 0; i < -diff; i++)
        {
            block.moveLeft();
        }
    }
}

bool ActionSimulator::dropBlock(const Board& board, Block& block)
{
    // 블록을 한 칸씩 아래로 이동
    while (true)
    {
        block.moveDown();
        
        // 충돌 체크
        if (board.isStrike(block))
        {
            // 한 칸 위로 되돌림
            block.moveUp();
            
            // 블록이 화면 위에 있으면 게임 오버
            const Position& pos = block.getPos();
            if (pos.getY() < 0)
            {
                return false; // 게임 오버
            }
            
            return true; // 정상적으로 착지
        }
    }
}

bool ActionSimulator::isValidPosition(const Board& board, const Block& block)
{
    return board.isStrike(block) == 0;
}

