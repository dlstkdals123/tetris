#pragma once

#include "FeatureExtractor.h"
#include "ActionSimulator.h"
#include <vector>
#include <string>

/**
 * Evaluator 클래스
 * Linear function approximation을 사용하여 보드 상태를 평가합니다.
 * 
 * 평가 함수: V(s) = w1*f1 + w2*f2 + ... + wn*fn
 * 여기서 fi는 feature, wi는 가중치입니다.
 */
class Evaluator {
public:
    /**
     * Weights 구조체
     * 각 feature에 대한 가중치를 저장합니다.
     */
    struct Weights {
        // Bertsekas & Tsitsiklis 논문 스타일: 26개 feature (12열 기준)
        // 1. 각 열의 높이 가중치 (12개)
        double columnHeights[12];
        
        // 2. 인접한 열의 높이 차이 가중치 (11개)
        double heightDiffs[11];
        
        // 3. 최대 높이 가중치 (1개)
        double maxHeight;
        
        // 4. 구멍의 개수 가중치 (1개)
        double holes;
        
        // 5. 우물 깊이 합 가중치 (1개)
        double wells;
        
        // 기본 생성자 - 초기 가중치 설정
        Weights();
        
        // 커스텀 가중치로 생성
        Weights(const double heights[12], const double diffs[11],
                double maxH, double holes, double wells);
    };
    
    /**
     * 생성자
     * @param weights 사용할 가중치 (기본값: 휴리스틱 기반 초기 가중치)
     */
    Evaluator(const Weights& weights = Weights());
    
    /**
     * Feature를 사용하여 보드 상태를 평가합니다.
     * @param features 평가할 feature
     * @return 평가 점수 (높을수록 좋은 상태)
     */
    double evaluate(const FeatureExtractor::Features& features) const;
    
    /**
     * 보드 상태를 직접 평가합니다.
     * @param board 평가할 보드
     * @return 평가 점수
     */
    double evaluateBoard(const Board& board) const;
    
    /**
     * 시뮬레이션 결과를 평가합니다.
     * @param result 시뮬레이션 결과
     * @return 평가 점수
     */
    double evaluateResult(const SimulationResult& result) const;
    
    /**
     * 가능한 모든 액션 중 최고의 액션을 선택합니다.
     * @param board 현재 보드 상태
     * @param block 현재 블록
     * @return 최고의 액션과 그 평가 점수
     */
    std::pair<Action, double> selectBestAction(const Board& board, const Block& block) const;
    
    /**
     * 다음 블록까지 고려하여 최고의 액션을 선택합니다 (Look-ahead).
     * @param board 현재 보드 상태
     * @param currentBlock 현재 블록
     * @param nextBlock 다음 블록 (선택적, nullptr이면 look-ahead 없음)
     * @param lookAheadDiscount 다음 블록 점수에 적용할 할인 인자 (기본값: 1.0)
     * @return 최고의 액션과 그 평가 점수
     */
    std::pair<Action, double> selectBestActionWithLookAhead(
        const Board& board, 
        const Block& currentBlock, 
        const Block* nextBlock = nullptr,
        double lookAheadDiscount = 1.0) const;
    
    /**
     * 가능한 모든 액션을 평가하고 정렬된 결과를 반환합니다.
     * @param board 현재 보드 상태
     * @param block 현재 블록
     * @return (액션, 점수) 쌍의 벡터 (점수 내림차순 정렬)
     */
    std::vector<std::pair<Action, double>> evaluateAllActions(const Board& board, const Block& block) const;
    
    /**
     * 현재 가중치를 반환합니다.
     */
    const Weights& getWeights() const { return weights_; }
    
    /**
     * 가중치를 설정합니다.
     */
    void setWeights(const Weights& weights) { weights_ = weights; }
    
    /**
     * 가중치를 파일에 저장합니다.
     * @param filename 저장할 파일명
     * @return 성공 여부
     */
    bool saveWeights(const std::string& filename) const;
    
    /**
     * 파일에서 가중치를 로드합니다.
     * @param filename 로드할 파일명
     * @param silent true이면 로드 메시지를 출력하지 않음 (기본값: false)
     * @return 성공 여부
     */
    bool loadWeights(const std::string& filename, bool silent = false);
    
    /**
     * 가중치를 출력합니다 (디버깅용).
     */
    void printWeights() const;
    
private:
    Weights weights_;
};

