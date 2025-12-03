#include "ActionSimulator.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include "GameConstants.h"
#include <vector>

std::vector<Action> ActionSimulator::generatePossibleActions(BlockType blockType)
{
    std::vector<Action> actions;
    
    // 블록 타입에 따른 회전 수 결정
    int numRotations = GameConstants::BlockRotation::MAX_ROTATIONS;
    if (blockType == BlockType::O) {
        numRotations = GameConstants::BlockRotation::O_BLOCK_ROTATIONS;
    } else if (blockType == BlockType::I || blockType == BlockType::S || blockType == BlockType::Z) {
        numRotations = GameConstants::BlockRotation::I_S_Z_BLOCK_ROTATIONS;
    }
    
    // 각 회전에 대해
    for (int rot = 0; rot < numRotations; rot++)
        // 각 열 위치에 대해
        for (int col = BoardConstants::MIN_COLUMN; col <= BoardConstants::MAX_COLUMN; col++)
            actions.push_back(Action(rot, col));

    
    return actions;
}

SimulationResult ActionSimulator::simulateAction(const Board& board, const Block& block, const Action& action)
{
    SimulationResult result;
    result.action = action;
    result.isValid = false;
    result.gameOver = false;
    result.linesCleared = GameConstants::Simulation::INITIAL_LINES_CLEARED;
    
    // 보드 복사
    Board simBoard(board);
    
    // 블록 복사 및 초기 위치 설정
    Block simBlock(block.getType(), Rotation(GameConstants::BlockRotation::INITIAL_ROTATION), 
                   Position(BoardConstants::BLOCK_START_X, BoardConstants::BLOCK_START_Y));
    
    // 회전 시도 (보드 충돌 체크 포함)
    if (!rotateBlock(simBoard, simBlock, action.rotation))
    {
        // 회전 불가능하면 이 액션은 무효
        return result; // isValid = false
    }
    
    // 목표 열로 이동
    moveBlockHorizontally(simBlock, action.column);
    
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
    auto lineResult = simBoard.deleteFullLine();
    result.linesCleared = lineResult.first; // 총 삭제된 줄 수
    
    // Feature 추출 (Bertsekas & Tsitsiklis 스타일: 26개 feature)
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
    // 회전 설정 (보드 충돌 체크 미포함)
    rotateBlock(block, rotation);
    
    // 수평 위치 조정
    moveBlockHorizontally(block, column);
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
            if (pos.getY() < GameConstants::Simulation::GAME_OVER_Y_THRESHOLD)
            {
                return false; // 게임 오버
            }
            
            return true; // 정상적으로 착지
        }
    }
}

bool ActionSimulator::isValidPosition(const Board& board, const Block& block)
{
    return board.isStrike(block) == GameConstants::Simulation::NO_COLLISION;
}

bool ActionSimulator::rotateBlock(const Board& board, Block& block, int targetRotation)
{
    // 목표 회전까지 회전 시도
    for (int i = 0; i < targetRotation; i++)
    {
        Rotation next_rotation(block.getRotation() + GameConstants::BlockRotation::ROTATION_INCREMENT);
        Block testBlock(block.getType(), next_rotation, block.getPos());
        
        // 회전 가능 여부 확인
        if (board.isStrike(testBlock) == GameConstants::Simulation::NO_COLLISION)
        {
            block.rotate();
        }
        else
        {
            // 회전 불가능
            return false;
        }
    }
    return true;
}

void ActionSimulator::rotateBlock(Block& block, int targetRotation)
{
    // 보드 충돌 체크 없이 회전
    for (int i = 0; i < targetRotation; i++)
    {
        block.rotate();
    }
}

void ActionSimulator::moveBlockHorizontally(Block& block, int targetColumn)
{
    const Position& pos = block.getPos();
    int currentX = pos.getX();
    int diff = targetColumn - currentX;
    
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
