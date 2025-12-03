#include "rotation.h"
#include "GameConstants.h"

// 기본 생성자
Rotation::Rotation() : angle(GameConstants::BlockRotation::INITIAL_ROTATION) {
}

// 매개변수 생성자
Rotation::Rotation(int angle) {
    // 0-3 범위로 정규화
    this->angle = ((angle % GameConstants::BlockRotation::MAX_ROTATIONS) + GameConstants::BlockRotation::MAX_ROTATIONS) % GameConstants::BlockRotation::MAX_ROTATIONS;
}

// Getter
int Rotation::getAngle() const {
    return angle;
}

// Setter
void Rotation::setAngle(int angle) {
    // 0-3 범위로 정규화
    this->angle = ((angle % GameConstants::BlockRotation::MAX_ROTATIONS) + GameConstants::BlockRotation::MAX_ROTATIONS) % GameConstants::BlockRotation::MAX_ROTATIONS;
}

// 0도로 초기화
void Rotation::reset() {
    angle = GameConstants::BlockRotation::INITIAL_ROTATION;
}

// 다음 회전 각도 반환
int Rotation::getNextAngle() const {
    return (angle + GameConstants::BlockRotation::ROTATION_INCREMENT) % GameConstants::BlockRotation::MAX_ROTATIONS;
}