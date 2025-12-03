#pragma once

#include "Block.h"
#include "Utils.h"
#include "BoardConstants.h"
#include <random>
#include <utility>

class Board {
  private:
    char total_block[BoardConstants::BOARD_HEIGHT][BoardConstants::BOARD_WIDTH];
    bool isPlayer;

  public:
    Board(bool isPlayer = true);
    Board(const Board& other); // 복사 생성자
    ~Board();

    int init();
    // 초기 상태 설정 (0=비어있음, 1=1줄, 2=2줄, 3=3줄, 4=4줄 - 각각 1자 비어있음)
    void initWithState(int stateType, std::mt19937& rng);
    // 전체 블럭 화면에 그리기
    void draw(const int &level) const; 
    // strike_check
    int isStrike(const Block &block) const;
    void mergeBlock(const Block &block);
    // check_full_line
    // 반환값: (총 삭제된 줄 수, attack 가능한 줄 수)
    // attack 가능한 줄 = 1로만 이루어진 줄 (2가 없음)
    std::pair<int, int> deleteFullLine();
    
    // Attack 라인 추가 (상대방에게 공격)
    int addAttackLines(int numLines); // 추가된 라인 수 반환, 게임오버 시 -1 반환
    
    // Feature 추출을 위한 접근 함수들
    char getCell(int row, int col) const;
    const char* getRow(int row) const;
};