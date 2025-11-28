#pragma once

#include "Block.h"
#include "Utils.h"

class Board {
  private:
    char total_block[21][14];
    bool isPlayer;

  public:
    Board(bool isPlayer = true);
    ~Board();

    int init();
    // 전체 블럭 화면에 그리기
    void draw(const int &level) const; 
    // strike_check
    int isStrike(const Block &block);
    void mergeBlock(const Block &block);
    // check_full_line
    int deleteFullLine(); // 삭제된 라인 수 반환
    
    // Feature 추출을 위한 접근 함수들
    char getCell(int row, int col) const;
    const char* getRow(int row) const;
    void copyBoard(char dest[21][14]) const;
};