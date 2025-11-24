#include "gameState.h"

// ������
gameState::gameState() : level(0), score(0), line(0) {
}

// �ʱ�ȭ �Լ� (gameover�� ���)
void gameState::resetState() {
    level = 0;
    score = 0;
    line = 0;
}

// Getter �޼���
int gameState::getLevel() const { return level; }
int gameState::getScore() const { return score; }
int gameState::getLines() const { return line; }

// Setter �޼��� : level ����
void gameState::setLevel(int lvl) {
    level = lvl;
}

// ���� ������Ʈ �޼���
void gameState::addScore(int amount) {
    score += amount;
}

void gameState::addLines(int amount) {
    line += amount;
}

// ������ 1 �ø��� ������ 0���� �ʱ�ȭ
void gameState::levelUp() {
    level++;
    resetLines();
}

// ���� ���� �ʱ�ȭ
void gameState::resetLines() {
    line = 0;
}