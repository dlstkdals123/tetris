#pragma once

#include "Evaluator.h"
#include "Board.h"
#include "Block.h"
#include "ActionSimulator.h"
#include <vector>
#include <string>
#include <random>

/**
 * MonteCarloLearner 클래스
 * Monte Carlo Learning을 사용하여 가중치를 학습합니다.
 * 
 * Monte Carlo 업데이트 규칙:
 * w ← w + α[G - V(s)]·∇V(s)
 * 
 * 여기서:
 * - α (alpha): 학습률
 * - γ (gamma): 할인 인자
 * - G: 에피소드 끝까지의 누적 할인 보상
 * - V(s): 현재 상태의 가치
 * - ∇V(s): 가치 함수의 그래디언트 (= feature 값들)
 */
class MCLearner {
public:
    /**
     * 경험 구조체 (Monte Carlo용)
     */
    struct Experience {
        FeatureExtractor::Features state;
        Action action;
        double reward;
        
        Experience(const FeatureExtractor::Features& s, const Action& a, double r)
            : state(s), action(a), reward(r) {}
    };
    
    /**
     * 학습 통계 구조체
     */
    struct Statistics {
        int episode;
        int totalLines;
        int totalScore;
        int moves;
        double averageReward;
        
        Statistics() : episode(0), totalLines(0), totalScore(0), 
                       moves(0), averageReward(0.0) {}
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
            , discountFactor(0.99)
            , epsilon(0.1)
            , epsilonDecay(0.9995)
            , minEpsilon(0.01)
            , maxEpisodes(1000)
            , maxMovesPerEpisode(10000)
            , verbose(true)
            , useMultiStage(false)
        {}
        
        // Multi-stage training 설정 (3단계, Monte Carlo용 50K 에피소드)
        void setupMultiStage() {
            useMultiStage = true;
            maxEpisodes = 50000;
            
            phases.clear();
            // Phase 1: 더 강한 Exploration (0-15000) - 학습률 증가
            phases.push_back(Phase(0, 15000, 0.01, 0.0, 0.000001));
            // Phase 2: Exploitation (15000-40000) - 학습률 증가
            phases.push_back(Phase(15000, 40000, 0.0, 0.0, 0.0000001));
            // Phase 3: Fine-tuning (40000-50000) - 학습률 증가
            phases.push_back(Phase(40000, 50000, 0.0, 0.0, 0.0000001));
        }
    };
    
    /**
     * 생성자
     * @param config 학습 설정
     * @param initialWeights 초기 가중치 (기본값: 휴리스틱 가중치)
     */
    MCLearner(const Config& config = Config(), 
              const Evaluator::Weights& initialWeights = Evaluator::Weights());
    
    /**
     * 한 에피소드를 실행합니다.
     * @param initialStateType 초기 상태 타입 (0=비어있음, 1=1줄, 2=2줄, 3=3줄, 4=4줄)
     * @return 에피소드 통계
     */
    Statistics runEpisode(int initialStateType = 0);
    
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
     * @param nextBlock 다음 블록 (선택적, look-ahead 사용 시)
     * @return 선택된 액션
     */
    Action selectAction(const Board& board, const Block& block, const Block* nextBlock = nullptr);
    
    /**
     * 보상 함수: 상태 전이에 대한 보상을 계산합니다.
     * 각 feature를 직접 받아서 reward를 계산합니다.
     * @param currentFeatures 이전 상태의 feature
     * @param newFeatures 현재 상태의 feature
     * @param linesCleared 제거된 라인 수
     * @param gameOver 게임 오버 여부
     * @return 보상 값
     */
    double calculateReward(const FeatureExtractor::Features& currentFeatures,
                          const FeatureExtractor::Features& newFeatures,
                          int linesCleared, bool gameOver) const;
    
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
     * @param stats 출력할 통계
     * @param sameLine true면 같은 줄에 덮어쓰기, false면 새 줄에 출력
     */
    void printStatistics(const Statistics& stats, bool sameLine = false) const;
    
    /**
     * 현재 에피소드에 맞는 phase 설정을 업데이트합니다.
     */
    void updatePhase(int episode);
    
private:
    /**
     * Monte Carlo 방식으로 가중치를 업데이트합니다.
     * @param state 현재 상태의 feature
     * @param G 누적 할인 보상
     * @param isTerminal 종료 상태 여부
     */
    void updateWeightsMonteCarlo(const FeatureExtractor::Features& state,
                                  double G,
                                  bool isTerminal);
    
    /**
     * 랜덤 액션을 선택합니다 (Exploration).
     */
    Action selectRandomAction(const Board& board, const Block& block);
    
    Evaluator evaluator_;
    Config config_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
};

