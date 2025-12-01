#pragma once

#include "Board.h"

/**
 * FeatureExtractor 클래스
 * 테트리스 보드 상태에서 강화학습을 위한 feature들을 추출합니다.
 */
class FeatureExtractor {
public:
    struct Features {
        // Bertsekas & Tsitsiklis 논문 스타일 26개 feature (12열 기준)
        // 1. 각 열의 높이 (12개)
        int columnHeights[12];  // h1, h2, ..., h12
        
        // 2. 인접한 열의 높이 차이 (11개)
        int heightDiffs[11];    // |h1-h2|, |h2-h3|, ..., |h11-h12|
        
        // 3. 최대 높이 (1개)
        int maxHeight;
        
        // 4. 구멍의 개수 (1개)
        int holes;
        
        // 5. 우물 깊이 합 (1개)
        int wells;
        
        Features() {
            for (int i = 0; i < 12; i++) {
                columnHeights[i] = 0;
            }
            for (int i = 0; i < 11; i++) {
                heightDiffs[i] = 0;
            }
            maxHeight = 0;
            holes = 0;
            wells = 0;
        }
    };

    /**
     * 보드 상태에서 Bertsekas & Tsitsiklis 논문 스타일 feature들을 추출합니다.
     * 26개 feature: 각 열 높이(12) + 인접 열 높이 차이(11) + 최대 높이(1) + 구멍(1) + 우물(1)
     * @param board 현재 보드 상태
     * @return 추출된 feature 구조체
     */
    static Features extractFeatures(const Board& board);
    
    /**
     * 특정 열의 높이를 계산합니다 (맨 위의 블록까지의 높이)
     * @param board 보드 상태
     * @param col 열 번호 (1~12, 0과 13은 벽)
     * @return 해당 열의 높이 (0~20)
     */
    static int getColumnHeight(const Board& board, int col);
    
    /**
     * 보드에서 구멍(hole)의 개수를 셉니다.
     * 구멍: 블록 위에 빈 공간이 있는 셀
     * @param board 보드 상태
     * @return 구멍의 총 개수
     */
    static int countHoles(const Board& board);
    
    /**
     * 행 전이(Row Transitions)를 계산합니다.
     * 각 행에서 빈칸 <-> 채워진 칸으로 바뀌는 횟수의 합
     * @param board 보드 상태
     * @return 행 전이 수
     */
    static int countRowTransitions(const Board& board);
    
    /**
     * 열 전이(Column Transitions)를 계산합니다.
     * 각 열에서 빈칸 <-> 채워진 칸으로 바뀌는 횟수의 합
     * @param board 보드 상태
     * @return 열 전이 수
     */
    static int countColumnTransitions(const Board& board);
    
    /**
     * 우물(Wells) 깊이 합을 계산합니다.
     * 양옆이 벽이나 블록으로 막힌 1칸 너비의 구덩이 깊이의 합
     * @param board 보드 상태
     * @return 우물 깊이 합
     */
    static int calculateWells(const Board& board);
    
    /**
     * Feature를 문자열로 출력 (디버깅용)
     * @param features 출력할 feature
     */
    static void printFeatures(const Features& features);
};

