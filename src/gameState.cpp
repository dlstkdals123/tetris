#include "gamestate.h"

// 생성자
gameState::gameState() : level(0), score(0), line(0) {
}

// 초기화 함수 (gameover시 사용)
void gameState::resetState() {
    level = 0;
    score = 0;
    line = 0;
}

// Getter 메서드
int gameState::getLevel() const { return level; }
int gameState::getScore() const { return score; }
int gameState::getLines() const { return line; }

// Setter 메서드 : level 설정
void gameState::setLevel(int lvl) {
    level = lvl;
}

// 상태 업데이트 메서드
void gameState::addScore(int amount) {
    score += amount;
}

void gameState::addLines(int amount) {
    line += amount;
}

// 레벨을 1 올리고 라인을 0으로 초기화
void gameState::levelUp() {
    level++;
    resetLines();
}

// 라인 수만 초기화
void gameState::resetLines() {
    line = 0;
}