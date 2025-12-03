#pragma once
#include "COLOR.h"
#include "GameConstants.h"
#include <mutex>
#include <queue>

// Forward declaration
class Board;

namespace Utils {
  //게임 렌더링 처리용
	extern std::recursive_mutex gameMutex;

  // 입력 처리용
	extern std::mutex inputMutex;
	extern std::queue<char> leftPlayerInputQueue;
	extern std::queue<char> rightPlayerInputQueue;
	
	// 상대방 Board 포인터 (attack 라인용)
	extern Board* leftPlayerBoard;
	extern Board* rightPlayerBoard;

  const int ab_x = GameConstants::BoardOffset::X;
  const int ab_y = GameConstants::BoardOffset::Y;
  int gotoxy(int x, int y, bool isLeft = true);
  void setColor(int color);
  void setColor(COLOR color);
};
