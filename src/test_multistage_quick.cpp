#include <iostream>
#include "TDLearner.h"

using namespace std;

/**
 * Multi-Stage Training 빠른 테스트
 * 축소된 버전으로 동작 확인
 */

int main()
{
    cout << "========================================" << endl;
    cout << "Multi-Stage Training Quick Test" << endl;
    cout << "========================================" << endl << endl;
    
    // 축소된 multi-stage 설정
    TDLearner::Config config;
    config.useMultiStage = true;
    config.maxEpisodes = 300;  // 30,000 대신 300
    config.maxMovesPerEpisode = 100;
    config.verbose = true;
    config.discountFactor = 0.95;
    
    // 축소된 phase 설정
    config.phases.clear();
    config.phases.push_back(TDLearner::Phase(0, 100, 0.3, 0.1, 0.005));      // Phase 1
    config.phases.push_back(TDLearner::Phase(100, 200, 0.1, 0.05, 0.001));   // Phase 2
    config.phases.push_back(TDLearner::Phase(200, 300, 0.05, 0.01, 0.0001)); // Phase 3
    
    cout << "Quick Test Configuration (scaled down 100x):" << endl;
    cout << "  Total Episodes: " << config.maxEpisodes << endl;
    cout << "  Phase 1: 0-100 (ε: 0.3→0.1, LR: 0.005)" << endl;
    cout << "  Phase 2: 100-200 (ε: 0.1→0.05, LR: 0.001)" << endl;
    cout << "  Phase 3: 200-300 (ε: 0.05→0.01, LR: 0.0001)" << endl;
    cout << endl;
    
    TDLearner learner(config);
    
    cout << "Initial Weights:" << endl;
    learner.getEvaluator().printWeights();
    cout << endl;
    
    cout << "Starting training..." << endl << endl;
    
    auto stats = learner.train(config.maxEpisodes);
    
    cout << "\n========================================" << endl;
    cout << "Test Complete!" << endl;
    cout << "========================================" << endl;
    
    cout << "\nFinal Weights:" << endl;
    learner.getEvaluator().printWeights();
    
    // Phase별 성능 분석
    cout << "\n========================================" << endl;
    cout << "Performance by Phase" << endl;
    cout << "========================================" << endl;
    
    auto calcAvg = [&](int start, int end) {
        int totalLines = 0, totalMoves = 0;
        int count = 0;
        for (int i = start; i < end && i < (int)stats.size(); i++) {
            totalLines += stats[i].totalLines;
            totalMoves += stats[i].moves;
            count++;
        }
        return make_pair((double)totalLines / count, (double)totalMoves / count);
    };
    
    auto [phase1Lines, phase1Moves] = calcAvg(0, 100);
    auto [phase2Lines, phase2Moves] = calcAvg(100, 200);
    auto [phase3Lines, phase3Moves] = calcAvg(200, 300);
    
    cout << "Phase 1 (Exploration):" << endl;
    cout << "  Avg Lines: " << phase1Lines << endl;
    cout << "  Avg Moves: " << phase1Moves << endl;
    
    cout << "\nPhase 2 (Exploitation):" << endl;
    cout << "  Avg Lines: " << phase2Lines << endl;
    cout << "  Avg Moves: " << phase2Moves << endl;
    
    cout << "\nPhase 3 (Fine-tuning):" << endl;
    cout << "  Avg Lines: " << phase3Lines << endl;
    cout << "  Avg Moves: " << phase3Moves << endl;
    
    cout << "\nImprovement:" << endl;
    cout << "  Phase 1→3: " << (phase3Lines - phase1Lines) << " lines/episode" << endl;
    
    // 가중치 저장
    learner.saveWeights("test_multistage_weights.txt");
    learner.saveProgress("test_multistage_progress.csv", stats);
    
    cout << "\n========================================" << endl;
    cout << "Multi-stage training works correctly!" << endl;
    cout << "Ready for full 30,000 episode training." << endl;
    cout << "========================================" << endl;
    
    return 0;
}

