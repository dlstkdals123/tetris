#pragma once

/**
 * Position 클래스
 * 테트리스 게임에서 블록의 위치를 나타내는 클래스
 */
class Position {
private:
    int x;  // x 좌표
    int y;  // y 좌표

public:
    // 생성자
    Position();
    Position(int x, int y);
    
    // 복사 생성자
    Position(const Position& other);
    
    // 대입 연산자
    Position& operator=(const Position& other);
    
    // Getter 메서드
    int getX() const;
    int getY() const;
    
    // Setter 메서드
    void setX(int x);
    void setY(int y);
    void set(int x, int y);
    
    // 위치 이동 메서드
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void move(int dx, int dy);
    
    // 연산자 오버로딩
    Position operator+(const Position& other) const;
    Position operator-(const Position& other) const;
    Position& operator+=(const Position& other);
    Position& operator-=(const Position& other);
    bool operator==(const Position& other) const;
    bool operator!=(const Position& other) const;
    
    // 유틸리티 메서드
    bool isValid(int minX, int maxX, int minY, int maxY) const;
    int distanceTo(const Position& other) const;
    
    // 디버그용 출력
    void print() const;
};