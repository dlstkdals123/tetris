#pragma once
#include "Utils.h"
#include "STAGE.h"
#include "ScoreManager.h"
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

    ScoreManager* scoreManager = nullptr;
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

    void show_gamestat(bool isLeft = true, bool printed_text = false);
    void setScoreManager(ScoreManager* sm);
};

