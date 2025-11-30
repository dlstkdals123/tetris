#pragma once

#include "Board.h"

/**
 * FeatureExtractor 클래스
 * 테트리스 보드 상태에서 강화학습을 위한 feature들을 추출합니다.
 */
class FeatureExtractor {
public:
    struct Features {
        int aggregateHeight;    // 모든 열의 높이 합
        int holes;              // 블록 위에 빈 공간이 있는 셀 수
        int bumpiness;          // 인접한 열들의 높이 차이 합
        int maxHeight;          // 최대 높이
        int minHeight;          // 최소 높이 (벽 제외)
        
        Features() : aggregateHeight(0), holes(0), 
                     bumpiness(0), maxHeight(0), minHeight(0) {}
    };
    
    /**
     * 보드 상태에서 feature들을 추출합니다.
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
     * 인접한 열들의 높이 차이 합을 계산합니다.
     * @param board 보드 상태
     * @return bumpiness 값
     */
    static int calculateBumpiness(const Board& board);
    
    /**
     * Feature를 문자열로 출력 (디버깅용)
     * @param features 출력할 feature
     */
    static void printFeatures(const Features& features);
};

