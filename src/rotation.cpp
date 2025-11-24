#include "rotation.h"
#include <iostream>

using namespace std;

// ê¸°ë³¸ ?ƒ?„±?ž
Rotation::Rotation() : angle(0) {
}

// ë§¤ê°œë³??ˆ˜ ?ƒ?„±?ž
Rotation::Rotation(int angle) {
    // 0-3 ë²”ìœ„ë¡? ? •ê·œí™”
    this->angle = ((angle % 4) + 4) % 4;
}

// ë³µì‚¬ ?ƒ?„±?ž
Rotation::Rotation(const Rotation& other) : angle(other.angle) {
}

// ????ž… ?—°?‚°?ž
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
    // 0-3 ë²”ìœ„ë¡? ? •ê·œí™”
    this->angle = ((angle % 4) + 4) % 4;
}

// ?‹œê³„ë°©?–¥ 90?„ ?šŒ? „
void Rotation::rotateClockwise() {
    angle = (angle + 1) % 4;
}

// ë°˜ì‹œê³„ë°©?–¥ 90?„ ?šŒ? „
void Rotation::rotateCounterClockwise() {
    angle = (angle - 1 + 4) % 4;
}

// 0?„ë¡? ì´ˆê¸°?™”
void Rotation::reset() {
    angle = 0;
}

// ?‹¤?Œ ?šŒ? „ ê°ë„ ë°˜í™˜
int Rotation::getNextAngle() const {
    return (angle + 1) % 4;
}

// ? „?œ„ ì¦ê?? ?—°?‚°?ž
Rotation& Rotation::operator++() {
    rotateClockwise();
    return *this;
}

// ?›„?œ„ ì¦ê?? ?—°?‚°?ž
Rotation Rotation::operator++(int) {
    Rotation temp(*this);
    rotateClockwise();
    return temp;
}

// ?™?“± ë¹„êµ ?—°?‚°?ž
bool Rotation::operator==(const Rotation& other) const {
    return angle == other.angle;
}

// ë¶??“± ë¹„êµ ?—°?‚°?ž
bool Rotation::operator!=(const Rotation& other) const {
    return !(*this == other);
}

// ê°ë„ë¥? degreeë¡? ë³??™˜
int Rotation::toDegrees() const {
    return angle * 90;
}

// ?œ ?š¨?„± ê²??‚¬
bool Rotation::isValid() const {
    return angle >= 0 && angle <= 3;
}

// ?””ë²„ê·¸?š© ì¶œë ¥
void Rotation::print() const {
    cout << "Rotation(angle=" << angle << ", degrees=" << toDegrees() << "Â°)" << endl;
}

