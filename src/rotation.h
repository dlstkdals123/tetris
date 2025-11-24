#pragma once

/**
 * Rotation 클래스
 * 테트리스 블록의 회전 상태를 관리 (0, 1, 2, 3 = 0°, 90°, 180°, 270°)
 */
class Rotation {
private:
    int angle;  // 0-3 사이의 값 (0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°)
    
public:
    // 생성자
    Rotation();
    Rotation(int angle);
    
    // 복사 생성자
    Rotation(const Rotation& other);
    
    // 대입 연산자
    Rotation& operator=(const Rotation& other);
    
    // Getter/Setter
    int getAngle() const;
    void setAngle(int angle);
    
    // 회전 메서드
    void rotateClockwise();         // 시계방향 90도 회전
    void rotateCounterClockwise();  // 반시계방향 90도 회전
    void reset();                    // 0도로 초기화
    
    // 다음 회전 각도 반환 (현재 객체는 변경하지 않음)
    int getNextAngle() const;
    
    // 연산자 오버로딩
    Rotation& operator++();          // 전위 증가 (시계방향)
    Rotation operator++(int);        // 후위 증가
    bool operator==(const Rotation& other) const;
    bool operator!=(const Rotation& other) const;
    
    // 유틸리티
    int toDegrees() const;           // 각도를 degree로 변환 (0, 90, 180, 270)
    bool isValid() const;            // 0-3 범위 체크
    
    // 디버그용 출력
    void print() const;
};

