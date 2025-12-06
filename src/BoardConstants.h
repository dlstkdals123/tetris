#pragma once

#include <cstdint>

/**
 * BoardConstants
 * 테트리스 보드 관련 상수 정의
 */
namespace BoardConstants {
    // 보드 크기
    constexpr int BOARD_WIDTH = 14;      // 전체 너비 (벽 포함)
    constexpr int BOARD_HEIGHT = 21;     // 전체 높이 (바닥 포함)
    constexpr int PLAY_WIDTH = 12;       // 플레이 영역 너비 (벽 제외)
    constexpr int PLAY_HEIGHT = 20;      // 플레이 영역 높이 (바닥 제외)
    
    // 열 범위
    constexpr int MIN_COLUMN = 1;        // 최소 열 (벽 다음)
    constexpr int MAX_COLUMN = 12;       // 최대 열 (벽 이전)
    
    // 행 범위
    constexpr int MIN_ROW = 0;           // 최소 행
    constexpr int MAX_ROW = 19;          // 최대 행 (바닥 이전)
    constexpr int FLOOR_ROW = 20;        // 바닥 행
    
    // 벽 위치
    constexpr int LEFT_WALL = 0;         // 왼쪽 벽
    constexpr int RIGHT_WALL = 13;       // 오른쪽 벽
    
    // 블록 시작 위치
    constexpr int BLOCK_START_Y = -4;    // 블록 생성 시 Y 위치
    constexpr int BLOCK_START_X = 5;     // 블록 생성 시 X 위치 (중앙)
    
    // 충돌 상태
    constexpr int STRIKE_TRUE = 1;       // 충돌 발생
    constexpr int STRIKE_FALSE = 0;      // 충돌 없음
    
    // 셀 상태
    constexpr char CELL_EMPTY = 0;       // 빈 셀
    constexpr char CELL_FILLED = 1;      // 채워진 셀 (벽, 바닥, 일반 블록)
    constexpr char CELL_ATTACK = 2;      // 공격 라인
    
    // 비트 마스크 상수
    constexpr uint32_t CELL_MASK = 0x3;  // 2비트 마스크 (0b11)
    constexpr uint32_t LEFT_WALL_MASK = CELL_MASK << (LEFT_WALL * 2);  // 왼쪽 벽 마스크 (비트 0-1)
    constexpr uint32_t RIGHT_WALL_MASK = CELL_MASK << (RIGHT_WALL * 2); // 오른쪽 벽 마스크 (비트 26-27)
    // 플레이 영역 마스크: MIN_COLUMN부터 MAX_COLUMN까지 (비트 2-25)
    // PLAY_WIDTH = 12, 각 컬럼은 2비트이므로 24비트
    constexpr uint32_t PLAY_AREA_MASK = ((1U << (PLAY_WIDTH * 2)) - 1) << (MIN_COLUMN * 2); // 0x3FFFFFC
    constexpr uint32_t FLOOR_MASK = 0x5555555;
}

