#include "Utils.h"
#include <window.h>

int Utils::gotoxy(int x, int y); {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); 
	COORD pos; 
	pos.Y=y;
	pos.X=x;
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