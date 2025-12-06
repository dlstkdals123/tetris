#pragma once

#include "Block.h"
#include "Utils.h"
#include "BoardConstants.h"
#include <utility>
#include <cstdint>

class Board {
  private:
    // 각 행을 32비트 정수로 저장 (13개 컬럼 * 2비트 = 26비트)
    uint32_t rows[BoardConstants::BOARD_HEIGHT];
    bool isPlayer;

  public:
    Board(bool isPlayer = true);
    Board(const Board& other); // 복사 생성자
    ~Board();

    // Feature 추출을 위한 접근 함수들
    char getCell(int row, int col) const;
    void setCell(int row, int col, char value);

    // 전체 블럭 화면에 그리기
    void draw(const int &level) const; 
    // strike_check
    int isStrike(const Block &block) const;
    void mergeBlock(const Block &block);
    // check_full_line
    // 반환값: (총 삭제된 줄 수, attack 가능한 줄 수)
    std::pair<int, int> deleteFullLine();
    
    // Attack 라인 추가 (상대방에게 공격)
    int addAttackLines(int numLines); // 추가된 라인 수 반환, 게임오버 시 -1 반환
};