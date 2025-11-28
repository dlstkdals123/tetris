#include <iostream>
#include "TDLearner.h"

using namespace std;

/**
 * Multi-Stage Training 프로그램
 * 
 * Phase 1 (0-5000): Exploration
 *   - Epsilon: 0.3 → 0.1
 *   - Learning Rate: 0.005
 *   - 목표: 다양한 전략 탐색
 * 
 * Phase 2 (5000-15000): Exploitation
 *   - Epsilon: 0.1 → 0.05
 *   - Learning Rate: 0.001
 *   - 목표: 좋은 전략 강화
 * 
 * Phase 3 (15000-30000): Fine-tuning
 *   - Epsilon: 0.05 → 0.01
 *   - Learning Rate: 0.0001
 *   - 목표: 미세 조정
 */

int main()
{
    cout << "========================================" << endl;
    cout << "Multi-Stage TD-Learning Training" << endl;
    cout << "Training the Ultimate Tetris Bot!" << endl;
    cout << "========================================" << endl << endl;
    
    // Multi-stage 설정
    TDLearner::Config config;
    config.setupMultiStage();  // 3단계 학습 활성화
    config.discountFactor = 0.95;
    config.maxMovesPerEpisode = 1000;
    config.verbose = true;
    
    cout << "Configuration:" << endl;
    cout << "  Total Episodes: " << config.maxEpisodes << endl;
    cout << "  Max Moves per Episode: " << config.maxMovesPerEpisode << endl;
    cout << "  Discount Factor: " << config.discountFactor << endl;
    cout << endl;
    
    // 초기 가중치 (휴리스틱 기반)
    TDLearner learner(config);
    
    cout << "Initial Weights:" << endl;
    learner.getEvaluator().printWeights();
    cout << endl;
    
    // 학습 시작
    cout << "Starting training... (This will take a while!)" << endl;
    cout << "Press Ctrl+C to stop early if needed." << endl;
    cout << "Weights will be saved every 1000 episodes." << endl << endl;
    
    auto startTime = chrono::steady_clock::now();
    
    vector<TDLearner::Statistics> stats = learner.train(config.maxEpisodes);
    
    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::minutes>(endTime - startTime);
    
    cout << "\n========================================" << endl;
    cout << "Training Complete!" << endl;
    cout << "========================================" << endl;
    cout << "Total time: " << duration.count() << " minutes" << endl;
    cout << "Total episodes: " << stats.size() << endl;
    
    // 최종 가중치
    cout << "\nFinal Weights:" << endl;
    learner.getEvaluator().printWeights();
    
    // 최종 가중치 저장
    string finalWeightsFile = "final_weights_multistage.txt";
    learner.saveWeights(finalWeightsFile);
    cout << "\nFinal weights saved to: " << finalWeightsFile << endl;
    
    // 학습 진행 상황 저장
    string progressFile = "training_progress_multistage.csv";
    learner.saveProgress(progressFile, stats);
    cout << "Training progress saved to: " << progressFile << endl;
    
    // 통계 요약
    cout << "\n========================================" << endl;
    cout << "Training Statistics Summary" << endl;
    cout << "========================================" << endl;
    
    if (!stats.empty())
    {
        // 초기 100 에피소드 평균
        int earlyLines = 0, earlyMoves = 0;
        for (int i = 0; i < min(100, (int)stats.size()); i++)
        {
            earlyLines += stats[i].totalLines;
            earlyMoves += stats[i].moves;
        }
        
        // 마지막 100 에피소드 평균
        int lateLines = 0, lateMoves = 0;
        int startIdx = max(0, (int)stats.size() - 100);
        for (int i = startIdx; i < (int)stats.size(); i++)
        {
            lateLines += stats[i].totalLines;
            lateMoves += stats[i].moves;
        }
        
        int earlyCount = min(100, (int)stats.size());
        int lateCount = stats.size() - startIdx;
        
        cout << "Early Performance (first " << earlyCount << " episodes):" << endl;
        cout << "  Avg Lines: " << (double)earlyLines / earlyCount << endl;
        cout << "  Avg Moves: " << (double)earlyMoves / earlyCount << endl;
        
        cout << "\nLate Performance (last " << lateCount << " episodes):" << endl;
        cout << "  Avg Lines: " << (double)lateLines / lateCount << endl;
        cout << "  Avg Moves: " << (double)lateMoves / lateCount << endl;
        
        double improvement = ((double)lateLines / lateCount) - ((double)earlyLines / earlyCount);
        cout << "\nImprovement: " << (improvement > 0 ? "+" : "") << improvement << " lines/episode" << endl;
        
        // 최고 성능 찾기
        int maxLines = 0;
        int maxLinesEpisode = 0;
        int maxMoves = 0;
        int maxMovesEpisode = 0;
        
        for (const auto& s : stats)
        {
            if (s.totalLines > maxLines)
            {
                maxLines = s.totalLines;
                maxLinesEpisode = s.episode;
            }
            if (s.moves > maxMoves)
            {
                maxMoves = s.moves;
                maxMovesEpisode = s.episode;
            }
        }
        
        cout << "\nBest Performance:" << endl;
        cout << "  Max Lines: " << maxLines << " (Episode " << maxLinesEpisode << ")" << endl;
        cout << "  Max Moves: " << maxMoves << " (Episode " << maxMovesEpisode << ")" << endl;
    }
    
    cout << "\n========================================" << endl;
    cout << "You can now use the trained weights to play!" << endl;
    cout << "Load '" << finalWeightsFile << "' in your game." << endl;
    cout << "========================================" << endl;
    
    return 0;
}

