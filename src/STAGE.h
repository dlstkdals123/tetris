#pragma once
#include "GameConstants.h"

/**
* STAGE 클래스
* 각 스테이지별 환경 초기화하는 클래스
*/
class STAGE
{
private:
	const int speed;		// 현재 속도
	const int stick_rate;	// 'ㅣ' 막대기 개수
	const int clear_line;	// 클리어해야 하는 라인 개수
	static const STAGE stage_data[GameConstants::Stage::COUNT];
public:
	// 생성자
	STAGE(int sp, int sr, int cl);

	// Getter 메서드
	int getSpeed() const;
	int getStickRate() const;
	int getClearLine() const;
	static STAGE getStage(int level);
};

