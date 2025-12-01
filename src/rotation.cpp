#include "rotation.h"

// 기본 생성자
Rotation::Rotation() : angle(0) {
}

// 매개변수 생성자
Rotation::Rotation(int angle) {
    // 0-3 범위로 정규화
    this->angle = ((angle % 4) + 4) % 4;
}

// Getter
int Rotation::getAngle() const {
    return angle;
}

// Setter
void Rotation::setAngle(int angle) {
    // 0-3 범위로 정규화
    this->angle = ((angle % 4) + 4) % 4;
}

// 0도로 초기화
void Rotation::reset() {
    angle = 0;
}

// 다음 회전 각도 반환
int Rotation::getNextAngle() const {
    return (angle + 1) % 4;
}