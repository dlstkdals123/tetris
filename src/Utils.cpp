#include "Utils.h"
#include <Windows.h>

std::recursive_mutex Utils::consoleMutex;

std::mutex Utils::inputMutex;
std::queue<char> Utils::playerInputQueue;
std::queue<char> Utils::aiInputQueue;

int Utils::gotoxy(int x, int y, bool isPlayer) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	COORD pos; 
	pos.Y=y;
	isPlayer ? pos.X = x : pos.X = x + 60;
	SetConsoleCursorPosition(hConsole, pos); 
	return 0;
}
void Utils::setColor(int color) {
  static HANDLE std_output_handle=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(std_output_handle, color); 
}
void Utils::setColor(COLOR color) {
  Utils::setColor(static_cast<int>(color));
}