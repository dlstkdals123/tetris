#pragma once

#include "Board.h"
#include "Block.h"
#include "FeatureExtractor.h"
#include <vector>

/**
 * Action 구조체
 * 테트리스 블록의 한 가지 가능한 액션(회전 + 위치)을 나타냅니다.
 */
struct Action {
    int rotation;      // 회전 각도 (0, 1, 2, 3)
    int column;        // 목표 열 위치 (1~12)
    
    Action(int rot = 0, int col = 5) : rotation(rot), column(col) {}
};

/**
 * SimulationResult 구조체
 * 액션 시뮬레이션 결과를 담습니다.
 */
struct SimulationResult {
    Action action;                          // 실행한 액션
    FeatureExtractor::Features features;    // 결과 보드의 feature들
    int linesCleared;                       // 제거된 라인 수
    bool isValid;                           // 유효한 액션인지 여부
    bool gameOver;                          // 게임 오버 여부
    
    SimulationResult() : linesCleared(0), isValid(false), gameOver(false) {}
};

/**
 * ActionSimulator 클래스
 * 주어진 보드 상태와 블록에 대해 가능한 모든 액션을 시뮬레이션합니다.
 */
class ActionSimulator {
public:
    /**
     * 주어진 블록에 대해 가능한 모든 액션을 생성합니다.
     * @param blockType 블록 타입
     * @return 가능한 액션들의 벡터
     */
    static std::vector<Action> generatePossibleActions(BlockType blockType);
    
    /**
     * 특정 액션을 시뮬레이션합니다.
     * @param board 현재 보드 상태
     * @param block 현재 블록
     * @param action 시뮬레이션할 액션
     * @return 시뮬레이션 결과
     */
    static SimulationResult simulateAction(const Board& board, const Block& block, const Action& action);
    
    /**
     * 모든 가능한 액션을 시뮬레이션하고 결과를 반환합니다.
     * @param board 현재 보드 상태
     * @param block 현재 블록
     * @return 모든 시뮬레이션 결과들의 벡터
     */
    static std::vector<SimulationResult> simulateAllActions(const Board& board, const Block& block);
    
    /**
     * 블록을 특정 회전과 열 위치로 이동시킵니다.
     * @param block 이동시킬 블록 (참조)
     * @param rotation 목표 회전
     * @param column 목표 열
     */
    static void moveBlockToPosition(Block& block, int rotation, int column);
    
    /**
     * 블록을 바닥까지 떨어뜨립니다.
     * @param board 보드 상태
     * @param block 떨어뜨릴 블록 (참조)
     * @return 성공 여부
     */
    static bool dropBlock(const Board& board, Block& block);
    
private:
    /**
     * 블록이 현재 위치에서 유효한지 확인합니다.
     * @param board 보드 상태
     * @param block 확인할 블록
     * @return 유효 여부
     */
    static bool isValidPosition(const Board& board, const Block& block);
    
    /**
     * 블록을 목표 회전으로 회전시킵니다 (보드 충돌 체크 포함).
     * @param board 보드 상태
     * @param block 회전시킬 블록 (참조)
     * @param targetRotation 목표 회전 수
     * @return 회전 성공 여부
     */
    static bool rotateBlock(const Board& board, Block& block, int targetRotation);
    
    /**
     * 블록을 목표 회전으로 회전시킵니다 (보드 충돌 체크 미포함).
     * @param block 회전시킬 블록 (참조)
     * @param targetRotation 목표 회전 수
     */
    static void rotateBlock(Block& block, int targetRotation);
    
    /**
     * 블록을 목표 열로 수평 이동시킵니다.
     * @param block 이동시킬 블록 (참조)
     * @param targetColumn 목표 열
     */
    static void moveBlockHorizontally(Block& block, int targetColumn);
};

