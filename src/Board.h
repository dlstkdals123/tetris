#pragma once

#include "Block.h"
#include "Utils.h"
#include <random>

class Board {
  private:
    char total_block[21][14];
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
    int deleteFullLine(); // 삭제된 라인 수 반환
    
    // Feature 추출을 위한 접근 함수들
    char getCell(int row, int col) const;
    const char* getRow(int row) const;
    void copyBoard(char dest[21][14]) const;
};