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
    
    // Getter 메서드
    int getX() const;
    int getY() const;
    
    // Setter 메서드
    void set(int x, int y);
    
    // 위치 이동 메서드
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
};