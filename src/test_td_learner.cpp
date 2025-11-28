#include <iostream>
#include "TDLearner.h"

using namespace std;

/**
 * TD-Learner 테스트 프로그램
 * TD-Learning 알고리즘의 동작을 테스트합니다.
 */

void testBasicLearning()
{
    cout << "\n### Test 1: Basic TD-Learning (10 episodes) ###" << endl;
    
    TDLearner::Config config;
    config.learningRate = 0.001;
    config.discountFactor = 0.95;
    config.epsilon = 0.1;
    config.maxEpisodes = 10;
    config.maxMovesPerEpisode = 100;
    config.verbose = true;
    
    TDLearner learner(config);
    
    cout << "\nInitial weights:" << endl;
    learner.getEvaluator().printWeights();
    
    cout << "\nStarting training..." << endl;
    vector<TDLearner::Statistics> stats = learner.train(10);
    
    cout << "\nFinal weights:" << endl;
    learner.getEvaluator().printWeights();
    
    cout << "\nTraining summary:" << endl;
    cout << "  Total episodes: " << stats.size() << endl;
    if (!stats.empty())
    {
        int totalLines = 0;
        int totalMoves = 0;
        for (const auto& s : stats)
        {
            totalLines += s.totalLines;
            totalMoves += s.moves;
        }
        cout << "  Average lines per episode: " << (double)totalLines / stats.size() << endl;
        cout << "  Average moves per episode: " << (double)totalMoves / stats.size() << endl;
    }
}

void testEpsilonDecay()
{
    cout << "\n### Test 2: Epsilon Decay ###" << endl;
    
    TDLearner::Config config;
    config.epsilon = 0.5;
    config.epsilonDecay = 0.95;
    config.minEpsilon = 0.01;
    config.maxMovesPerEpisode = 50;
    config.verbose = false;
    
    TDLearner learner(config);
    
    cout << "Initial epsilon: " << learner.getEpsilon() << endl;
    
    for (int i = 0; i < 5; i++)
    {
        learner.runEpisode();
        cout << "After episode " << (i+1) << ": epsilon = " << learner.getEpsilon() << endl;
    }
}

void testSaveLoadWeights()
{
    cout << "\n### Test 3: Save and Load Weights ###" << endl;
    
    TDLearner::Config config;
    config.maxMovesPerEpisode = 50;
    config.verbose = false;
    
    TDLearner learner1(config);
    
    cout << "Training for 5 episodes..." << endl;
    learner1.train(5);
    
    cout << "\nWeights after training:" << endl;
    learner1.getEvaluator().printWeights();
    
    // 가중치 저장
    string filename = "test_td_weights.txt";
    learner1.saveWeights(filename);
    
    // 새로운 learner에 로드
    TDLearner learner2(config);
    cout << "\nBefore loading:" << endl;
    learner2.getEvaluator().printWeights();
    
    learner2.loadWeights(filename);
    cout << "\nAfter loading:" << endl;
    learner2.getEvaluator().printWeights();
}

void testRewardFunction()
{
    cout << "\n### Test 4: Reward Function ###" << endl;
    
    TDLearner::Config config;
    TDLearner learner(config);
    
    cout << "Testing different scenarios:" << endl;
    
    // 1줄 제거
    double r1 = learner.calculateReward(1, 0, 0, false);
    cout << "  1 line cleared: " << r1 << endl;
    
    // 4줄 제거 (테트리스!)
    double r4 = learner.calculateReward(4, 0, 0, false);
    cout << "  4 lines cleared (Tetris!): " << r4 << endl;
    
    // 높이 증가
    double rHeight = learner.calculateReward(0, 5, 0, false);
    cout << "  Height increased by 5: " << rHeight << endl;
    
    // 구멍 생성
    double rHoles = learner.calculateReward(0, 0, 3, false);
    cout << "  3 holes created: " << rHoles << endl;
    
    // 게임 오버
    double rGameOver = learner.calculateReward(0, 0, 0, true);
    cout << "  Game over: " << rGameOver << endl;
}

void testActionSelection()
{
    cout << "\n### Test 5: Action Selection (ε-greedy) ###" << endl;
    
    Board board(true);
    board.init();
    Block block(BlockType::I);
    
    // High epsilon (more exploration)
    TDLearner::Config config1;
    config1.epsilon = 0.9;
    TDLearner learner1(config1);
    
    cout << "With high epsilon (0.9):" << endl;
    for (int i = 0; i < 5; i++)
    {
        Action action = learner1.selectAction(board, block);
        cout << "  Action " << (i+1) << ": Rot=" << action.rotation 
             << ", Col=" << action.column << endl;
    }
    
    // Low epsilon (more exploitation)
    TDLearner::Config config2;
    config2.epsilon = 0.0;
    TDLearner learner2(config2);
    
    cout << "\nWith low epsilon (0.0):" << endl;
    for (int i = 0; i < 5; i++)
    {
        Action action = learner2.selectAction(board, block);
        cout << "  Action " << (i+1) << ": Rot=" << action.rotation 
             << ", Col=" << action.column << endl;
    }
}

void testProgressSaving()
{
    cout << "\n### Test 6: Save Training Progress ###" << endl;
    
    TDLearner::Config config;
    config.maxMovesPerEpisode = 50;
    config.verbose = false;
    
    TDLearner learner(config);
    
    cout << "Training for 20 episodes..." << endl;
    vector<TDLearner::Statistics> stats = learner.train(20);
    
    string filename = "test_training_progress.csv";
    learner.saveProgress(filename, stats);
    
    cout << "Progress saved. You can plot the results using the CSV file." << endl;
}

int main()
{
    system("cls");

    
    cout << "======================================" << endl;
    cout << "TD-Learner Test Program" << endl;
    cout << "======================================" << endl;
    
    testBasicLearning();
    testEpsilonDecay();
    testSaveLoadWeights();
    testRewardFunction();
    testActionSelection();
    testProgressSaving();
    
    cout << "\n======================================" << endl;
    cout << "All Tests Complete!" << endl;
    cout << "======================================" << endl;
    
    return 0;
}

