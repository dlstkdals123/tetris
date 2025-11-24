#include "rotation.h"
#include <iostream>

using namespace std;

// 기본 생성자
Rotation::Rotation() : angle(0) {
}

// 매개변수 생성자
Rotation::Rotation(int angle) {
    // 0-3 범위로 정규화
    this->angle = ((angle % 4) + 4) % 4;
}

// 복사 생성자
Rotation::Rotation(const Rotation& other) : angle(other.angle) {
}

// 대입 연산자
Rotation& Rotation::operator=(const Rotation& other) {
    if (this != &other) {
        angle = other.angle;
    }
    return *this;
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

// 시계방향 90도 회전
void Rotation::rotateClockwise() {
    angle = (angle + 1) % 4;
}

// 반시계방향 90도 회전
void Rotation::rotateCounterClockwise() {
    angle = (angle - 1 + 4) % 4;
}

// 0도로 초기화
void Rotation::reset() {
    angle = 0;
}

// 다음 회전 각도 반환
int Rotation::getNextAngle() const {
    return (angle + 1) % 4;
}

// 전위 증가 연산자
Rotation& Rotation::operator++() {
    rotateClockwise();
    return *this;
}

// 후위 증가 연산자
Rotation Rotation::operator++(int) {
    Rotation temp(*this);
    rotateClockwise();
    return temp;
}

// 동등 비교 연산자
bool Rotation::operator==(const Rotation& other) const {
    return angle == other.angle;
}

// 부등 비교 연산자
bool Rotation::operator!=(const Rotation& other) const {
    return !(*this == other);
}

// 각도를 degree로 변환
int Rotation::toDegrees() const {
    return angle * 90;
}

// 유효성 검사
bool Rotation::isValid() const {
    return angle >= 0 && angle <= 3;
}

// 디버그용 출력
void Rotation::print() const {
    cout << "Rotation(angle=" << angle << ", degrees=" << toDegrees() << "°)" << endl;
}

