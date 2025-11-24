#include "STAGE.h"

// 매개변수 생성자
STAGE::STAGE(int sp, int sr, int cl)
	: speed(sp), stick_rate(sr), clear_line(cl) {
}

// Getter 메서드
int STAGE::getSpeed() const { return speed; }
int STAGE::getStickRate() const { return stick_rate; }
int STAGE::getClearLine() const { return clear_line; }