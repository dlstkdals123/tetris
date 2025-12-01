#pragma once

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
}

