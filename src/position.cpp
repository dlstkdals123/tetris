#include "position.h"

// 기본 생성자
Position::Position() : x(0), y(0) {
}

// 매개변수 생성자
Position::Position(int x, int y) : x(x), y(y) {
}

// Getter 메서드
int Position::getX() const {
    return x;
}

int Position::getY() const {
    return y;
}

// Setter 메서드
void Position::set(int x, int y) {
    this->x = x;
    this->y = y;
}

// 위치 이동 메서드
void Position::moveLeft() {
    x--;
}

void Position::moveRight() {
    x++;
}

void Position::moveUp() {
    y--;
}

void Position::moveDown() {
    y++;
}

