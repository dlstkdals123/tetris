#include <iostream>
#include <cstdlib>
#include <ctime>
#include "MCLearner.h"

using namespace std;

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    cout << "========================================" << endl;
    cout << "Multi-Stage Monte Carlo Training" << endl;
    cout << "Training the Ultimate Tetris Bot!" << endl;
    cout << "========================================" << endl << endl;
    
    // Multi-stage 설정
    MCLearner::Config config;
    config.setupMultiStage();  // 3단계 학습 활성화
    config.discountFactor = 0.99;
    config.maxMovesPerEpisode = 1000;
    config.verbose = true;
    
    cout << "Configuration:" << endl;
    cout << "  Total Episodes: " << config.maxEpisodes << endl;
    cout << "  Max Moves per Episode: " << config.maxMovesPerEpisode << endl;
    cout << "  Discount Factor: " << config.discountFactor << endl;
    cout << "  Print Interval: Every 500 episodes" << endl;
    cout << "  Save: Only final weights" << endl;
    cout << endl;
    
    // 초기 가중치 로드 시도
    MCLearner learner(config);
    const string initialWeightsFile = "initial_weights.txt";
    
    if (learner.loadWeights(initialWeightsFile)) {
        cout << "Loaded initial weights from: " << initialWeightsFile << endl;
    } else {
        cout << "Using default heuristic weights (file not found: " << initialWeightsFile << ")" << endl;
    }
    
    cout << "\nInitial Weights:" << endl;
    learner.getEvaluator().printWeights();
    cout << endl;
    
    // 학습 시작
    cout << "Starting training...)" << endl;
    cout << "Press Ctrl+C to stop early if needed." << endl;
    cout << "Weights will be saved only at completion." << endl << endl;
    
    auto startTime = chrono::steady_clock::now();
    
    vector<MCLearner::Statistics> stats = learner.train(config.maxEpisodes);
    
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

