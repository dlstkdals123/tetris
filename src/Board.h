#pragma once

#include "Block.h"
#include "Utils.h"

class Board {
  private:
    char total_block[21][14];
    int ab_x,ab_y;

  public:
    Board();
    ~Board();

    int init();
    // 전체 블럭 화면에 그리기
    void draw(const int &level) const; 
    // strike_check
    int isStrike(const Block &block);
    void mergeBlock(const Block &block);
    // check_full_line
    int deleteFullLine(); // 삭제된 라인 수 반환

    void show_next_block();
};