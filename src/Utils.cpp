#include "Utils.h"
#include "Board.h"
#include "GameConstants.h"
#include <Windows.h>

std::recursive_mutex Utils::gameMutex;

std::mutex Utils::inputMutex;
std::queue<char> Utils::leftPlayerInputQueue;
std::queue<char> Utils::rightPlayerInputQueue;

Board* Utils::leftPlayerBoard = nullptr;
Board* Utils::rightPlayerBoard = nullptr;

int Utils::gotoxy(int x, int y, bool isLeft) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	COORD pos; 
	pos.Y=y;
	isLeft ? pos.X = x : pos.X = x + GameConstants::Utils::RIGHT_PLAYER_X_OFFSET;
	SetConsoleCursorPosition(hConsole, pos); 
	return GameConstants::Utils::GOTOXY_SUCCESS;
}
void Utils::setColor(int color) {
  static HANDLE std_output_handle=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, color); 
}
void Utils::setColor(COLOR color) {
  Utils::setColor(static_cast<int>(color));
}