#include "STAGE.h"

// 매개변수 생성자
STAGE::STAGE(int sp, int sr, int cl)
	: speed(sp), stick_rate(sr), clear_line(cl) {
}

// Getter 메서드
int STAGE::getSpeed() const { return speed; }
int STAGE::getStickRate() const { return stick_rate; }
int STAGE::getClearLine() const { return clear_line; }
STAGE STAGE::getStage (int level) {
    return stage_data[level];
}

// 순서대로 speed, stick_rate, clear_line
const STAGE STAGE::stage_data[10] = {
    STAGE(40, 20, 20),       // Level 1
    STAGE(38, 18, 20),       // Level 2
    STAGE(35, 18, 20),       // Level 3
    STAGE(30, 17, 20),       // Level 4
    STAGE(25, 16, 20),       // Level 5
    STAGE(20, 14, 20),       // Level 6
    STAGE(15, 14, 20),       // Level 7
    STAGE(10, 13, 20),       // Level 8
    STAGE(6,  12, 20),       // Level 9
    STAGE(4,  11, 99999)     // Level 10 (사실상 마지막)
};