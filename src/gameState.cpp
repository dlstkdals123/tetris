#include "gameState.h"
#include "GameConstants.h"
#include <cstdio>

// 생성자
gameState::gameState() : level(GameConstants::GameStateInitial::INITIAL_LEVEL), score(GameConstants::GameStateInitial::INITIAL_SCORE), line(GameConstants::GameStateInitial::INITIAL_LINES) {
}

// 초기화 함수 (gameover시 사용)
void gameState::resetState() {
    level = GameConstants::GameStateInitial::INITIAL_LEVEL;
    score = GameConstants::GameStateInitial::INITIAL_SCORE;
    line = GameConstants::GameStateInitial::INITIAL_LINES;
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
    level += GameConstants::GameStateInitial::LEVEL_INCREMENT;
    resetLines();
}

// 라인 수만 초기화
void gameState::resetLines() {
    line = GameConstants::GameStateInitial::INITIAL_LINES;
}

void gameState::show_gamestat(bool isLeft, bool printed_text)
{
    std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
    Utils::setColor(COLOR::GRAY);

    if (printed_text)
    {
        Utils::gotoxy(GameConstants::GameStatUI::STAT_LEFT_X, GameConstants::GameStatUI::STAGE_TEXT_Y, isLeft);
        printf("STAGE");

        Utils::gotoxy(GameConstants::GameStatUI::STAT_LEFT_X, GameConstants::GameStatUI::SCORE_TEXT_Y, isLeft);
        printf("SCORE");

        Utils::gotoxy(GameConstants::GameStatUI::STAT_LEFT_X, GameConstants::GameStatUI::LINES_TEXT_Y, isLeft);
        printf("LINES");
    }

    Utils::gotoxy(GameConstants::GameStatUI::LEVEL_VALUE_X, GameConstants::GameStatUI::STAGE_TEXT_Y, isLeft);
    printf("%d", this->getLevel() + GameConstants::GameStateInitial::LEVEL_DISPLAY_OFFSET);

    Utils::gotoxy(GameConstants::GameStatUI::STAT_LEFT_X, GameConstants::GameStatUI::SCORE_VALUE_Y, isLeft);
    printf(GameConstants::GameStatUI::SCORE_FORMAT, this->getScore());

    Utils::gotoxy(GameConstants::GameStatUI::STAT_LEFT_X, GameConstants::GameStatUI::LINES_VALUE_Y, isLeft);

    int remain = STAGE::getStage(this->getLevel()).getClearLine() - this->getLines();
    if (remain < GameConstants::GameStateInitial::MIN_REMAINING_LINES)
        remain = GameConstants::GameStateInitial::MIN_REMAINING_LINES;
    printf(GameConstants::GameStatUI::SCORE_FORMAT, remain);
}

void gameState::setScoreManager(ScoreManager* sm) {
    scoreManager = sm;
}