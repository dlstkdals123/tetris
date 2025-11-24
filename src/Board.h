#pragma once

#include "Utils.h"

 // ======== ?„?‹œ ?´?˜?Š¤ =========
// class Block {
//   public:
//     int shape;
//     int angle;
//     int x;
//     int y;
//     int getShape(int x, int y) const;
//     int getx() const;
//     int gety() const;
   
// };
// =============================

class Board {
  private:
    char total_block[21][14];
    int ab_x,ab_y;

  public:
    Board();
    ~Board();

    int init();
    // ? „ì²? ë¸”ëŸ­ ?™”ë©´ì— ê·¸ë¦¬ê¸?
    void draw(const int &level) const; 
    // strike_check
    int isStrike(const Block &block);
    void mergeBlock(const Block &block);
    // check_full_line
    int deleteFullLine(); // ?‚­? œ?œ ?¼?¸ ?ˆ˜ ë°˜í™˜

    void show_next_block();
};