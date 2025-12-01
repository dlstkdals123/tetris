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
    
    // Getter/Setter
    int getAngle() const;
    void setAngle(int angle);
    
    // 회전 메서드
    void reset();                    // 0도로 초기화
    
    // 다음 회전 각도 반환 (현재 객체는 변경하지 않음)
    int getNextAngle() const;
};

