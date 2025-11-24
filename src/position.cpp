#include "position.h"
#include <iostream>
#include <cmath>

using namespace std;

// 기본 생성자
Position::Position() : x(0), y(0) {
}

// 매개변수 생성자
Position::Position(int x, int y) : x(x), y(y) {
}

// 복사 생성자
Position::Position(const Position& other) : x(other.x), y(other.y) {
}

// 대입 연산자
Position& Position::operator=(const Position& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
    }
    return *this;
}

// Getter 메서드
int Position::getX() const {
    return x;
}

int Position::getY() const {
    return y;
}

// Setter 메서드
void Position::setX(int x) {
    this->x = x;
}

void Position::setY(int y) {
    this->y = y;
}

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

void Position::move(int dx, int dy) {
    x += dx;
    y += dy;
}

// 연산자 오버로딩 - 덧셈
Position Position::operator+(const Position& other) const {
    return Position(x + other.x, y + other.y);
}

// 연산자 오버로딩 - 뺄셈
Position Position::operator-(const Position& other) const {
    return Position(x - other.x, y - other.y);
}

// 연산자 오버로딩 - 복합 대입 덧셈
Position& Position::operator+=(const Position& other) {
    x += other.x;
    y += other.y;
    return *this;
}

// 연산자 오버로딩 - 복합 대입 뺄셈
Position& Position::operator-=(const Position& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

// 연산자 오버로딩 - 동등 비교
bool Position::operator==(const Position& other) const {
    return (x == other.x) && (y == other.y);
}

// 연산자 오버로딩 - 부등 비교
bool Position::operator!=(const Position& other) const {
    return !(*this == other);
}

// 유효성 검사 메서드
bool Position::isValid(int minX, int maxX, int minY, int maxY) const {
    return (x >= minX && x <= maxX && y >= minY && y <= maxY);
}

// 맨해튼 거리 계산
int Position::distanceTo(const Position& other) const {
    return abs(x - other.x) + abs(y - other.y);
}

// 디버그용 출력
void Position::print() const {
    cout << "Position(" << x << ", " << y << ")" << endl;
}

