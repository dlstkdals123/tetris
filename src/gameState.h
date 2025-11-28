#pragma once
#include "Utils.h"
#include "STAGE.h"
/**
* gamestate 클래스
* 현재 게임의 상태를 나타내는 클래스
*/
class gameState
{
private:
	int level;  // 현재 플레이하는 게임 레벨
	int score;  // 현재 획득한 점수
	int line;   // 현재 클리어한 라인 수
public:
    // 생성자
    gameState();

    // 초기화 함수 (gameover시 사용)
    void resetState();

    // Getter 메서드
    int getLevel() const;
    int getScore() const;
    int getLines() const;

    // Setter 메서드
    void setLevel(int lvl);

    // 상태 업데이트 메서드
    void addScore(int amount);
    void addLines(int amount);
    void levelUp();      // 레벨을 1 올리고 라인을 0으로 초기화
    void resetLines();   // 라인 수만 초기화]


    void show_gamestat(bool isPlayer, bool printed_text)
    {
        std::lock_guard<std::recursive_mutex> lock(Utils::gameMutex); // 스레드 동시 접근 방지
        Utils::setColor(COLOR::GRAY);

        if (printed_text)
        {
            Utils::gotoxy(35, 7, isPlayer);
            printf("STAGE");

            Utils::gotoxy(35, 9, isPlayer);
            printf("SCORE");

            Utils::gotoxy(35, 12, isPlayer);
            printf("LINES");
        }

        Utils::gotoxy(41, 7, isPlayer);
        printf("%d", this->getLevel() + 1);

        Utils::gotoxy(35, 10, isPlayer);
        printf("%10d", this->getScore());

        Utils::gotoxy(35, 13, isPlayer);
        
        int remain = STAGE::getStage(this->getLevel()).getClearLine() - this->getLines();
        if (remain < 0)
            remain = 0;
        printf("%10d", remain);
    }

};

