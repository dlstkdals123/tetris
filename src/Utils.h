#pragma once
#include "COLOR.h"

namespace Utils {
  const int ab_x = 5;
  const int ab_y = 1;
  int gotoxy(int x, int y);
  void setColor(int color);
  void setColor(COLOR color);
};
