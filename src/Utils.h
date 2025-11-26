#pragma once
#include "COLOR.h"
#include <mutex>
#include <queue>
namespace Utils {
  //게임 렌더링 처리용
	extern std::recursive_mutex gameMutex;

  // 입력 처리용
	extern std::mutex inputMutex;
	extern std::queue<char> playerInputQueue;
	extern std::queue<char> aiInputQueue;

  const int ab_x = 5;
  const int ab_y = 1;
  int gotoxy(int x, int y, bool isPlayer = true);
  void setColor(int color);
  void setColor(COLOR color);
};
