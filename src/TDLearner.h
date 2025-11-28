#pragma once

#include "Evaluator.h"
#include "Board.h"
#include "Block.h"
#include "ActionSimulator.h"
#include <vector>
#include <string>
#include <random>

/**
 * TDLearner 클래스
 * Temporal Difference Learning을 사용하여 가중치를 학습합니다.
 * 
 * TD(0) 업데이트 규칙:
 * w ← w + α[r + γ·V(s') - V(s)]·∇V(s)
 * 
 * 여기서:
 * - α (alpha): 학습률
 * - γ (gamma): 할인 인자
 * - r: 즉시 보상
 * - V(s): 현재 상태의 가치
 * - V(s'): 다음 상태의 가치
 * - ∇V(s): 가치 함수의 그래디언트 (= feature 값들)
 */
class TDLearner {
public:
    /**
     * 학습 통계 구조체
     */
    struct Statistics {
        int episode;
        int totalLines;
        int totalScore;
        int maxHeight;
        int moves;
        double averageReward;
        
        Statistics() : episode(0), totalLines(0), totalScore(0), 
                       maxHeight(0), moves(0), averageReward(0.0) {}
    };
    
    /**
     * 학습 단계 구조체
     */
    struct Phase {
        int startEpisode;
        int endEpisode;
        double startEpsilon;
        double endEpsilon;
        double learningRate;
        
        Phase(int start, int end, double startEps, double endEps, double lr)
            : startEpisode(start), endEpisode(end)
            , startEpsilon(startEps), endEpsilon(endEps)
            , learningRate(lr)
        {}
    };
    
    /**
     * 학습 설정 구조체
     */
    struct Config {
        double learningRate;      // α: 학습률 (0.001 ~ 0.01)
        double discountFactor;    // γ: 할인 인자 (0.9 ~ 0.99)
        double epsilon;           // ε: exploration 확률 (0.0 ~ 1.0)
        double epsilonDecay;      // ε 감소율
        double minEpsilon;        // 최소 ε 값
        int maxEpisodes;          // 최대 에피소드 수
        int maxMovesPerEpisode;   // 에피소드당 최대 이동 수
        bool verbose;             // 상세 로그 출력 여부
        bool useMultiStage;       // Multi-stage training 사용 여부
        std::vector<Phase> phases; // 학습 단계들
        
        Config()
            : learningRate(0.001)
            , discountFactor(0.95)
            , epsilon(0.1)
            , epsilonDecay(0.9995)
            , minEpsilon(0.01)
            , maxEpisodes(1000)
            , maxMovesPerEpisode(10000)
            , verbose(true)
            , useMultiStage(false)
        {}
        
        // Multi-stage training 설정 (3단계)
        void setupMultiStage() {
            useMultiStage = true;
            maxEpisodes = 30000;
            
            phases.clear();
            // Phase 1: Exploration (0-5000)
            phases.push_back(Phase(0, 5000, 0.3, 0.1, 0.005));
            // Phase 2: Exploitation (5000-15000)
            phases.push_back(Phase(5000, 15000, 0.1, 0.05, 0.001));
            // Phase 3: Fine-tuning (15000-30000)
            phases.push_back(Phase(15000, 30000, 0.05, 0.01, 0.0001));
        }
    };
    
    /**
     * 생성자
     * @param config 학습 설정
     * @param initialWeights 초기 가중치 (기본값: 휴리스틱 가중치)
     */
    TDLearner(const Config& config = Config(), 
              const Evaluator::Weights& initialWeights = Evaluator::Weights());
    
    /**
     * 한 에피소드를 실행합니다.
     * @return 에피소드 통계
     */
    Statistics runEpisode();
    
    /**
     * 여러 에피소드를 실행하여 학습합니다.
     * @param numEpisodes 실행할 에피소드 수
     * @return 모든 에피소드의 통계 벡터
     */
    std::vector<Statistics> train(int numEpisodes);
    
    /**
     * ε-greedy 정책으로 액션을 선택합니다.
     * @param board 현재 보드
     * @param block 현재 블록
     * @return 선택된 액션
     */
    Action selectAction(const Board& board, const Block& block);
    
    /**
     * TD 업데이트를 수행합니다.
     * @param currentFeatures 현재 상태의 feature
     * @param reward 즉시 보상
     * @param nextFeatures 다음 상태의 feature
     * @param isTerminal 종료 상태 여부
     */
    void updateWeights(const FeatureExtractor::Features& currentFeatures,
                      double reward,
                      const FeatureExtractor::Features& nextFeatures,
                      bool isTerminal);
    
    /**
     * 보상 함수: 상태 전이에 대한 보상을 계산합니다.
     * @param linesCleared 제거된 라인 수
     * @param heightDiff 높이 변화
     * @param holesDiff 구멍 변화
     * @param gameOver 게임 오버 여부
     * @return 보상 값
     */
    double calculateReward(int linesCleared, int heightDiff, int holesDiff, bool gameOver) const;
    
    /**
     * 현재 evaluator를 반환합니다.
     */
    const Evaluator& getEvaluator() const { return evaluator_; }
    
    /**
     * 현재 가중치를 반환합니다.
     */
    const Evaluator::Weights& getWeights() const { return evaluator_.getWeights(); }
    
    /**
     * 가중치를 설정합니다.
     */
    void setWeights(const Evaluator::Weights& weights) { evaluator_.setWeights(weights); }
    
    /**
     * 학습 설정을 반환합니다.
     */
    const Config& getConfig() const { return config_; }
    
    /**
     * epsilon 값을 반환합니다.
     */
    double getEpsilon() const { return config_.epsilon; }
    
    /**
     * 가중치를 파일에 저장합니다.
     */
    bool saveWeights(const std::string& filename) const {
        return evaluator_.saveWeights(filename);
    }
    
    /**
     * 파일에서 가중치를 로드합니다.
     */
    bool loadWeights(const std::string& filename) {
        return evaluator_.loadWeights(filename);
    }
    
    /**
     * 학습 통계를 출력합니다.
     */
    void printStatistics(const Statistics& stats) const;
    
    /**
     * 학습 진행 상황을 저장합니다.
     */
    bool saveProgress(const std::string& filename, const std::vector<Statistics>& allStats) const;
    
    /**
     * 현재 에피소드에 맞는 phase 설정을 업데이트합니다.
     */
    void updatePhase(int episode);
    
private:
    Evaluator evaluator_;
    Config config_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
};

