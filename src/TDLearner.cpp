#include "TDLearner.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>

using namespace std;

TDLearner::TDLearner(const Config& config, const Evaluator::Weights& initialWeights)
    : evaluator_(initialWeights)
    , config_(config)
    , rng_(chrono::steady_clock::now().time_since_epoch().count())
    , dist_(0.0, 1.0)
{
}

Action TDLearner::selectAction(const Board& board, const Block& block)
{
    // ε-greedy 정책
    if (dist_(rng_) < config_.epsilon)
    {
        // Exploration: 랜덤 액션 선택
        vector<Action> actions = ActionSimulator::generatePossibleActions(block.getType());
        if (actions.empty())
        {
            return Action(0, 5); // 기본 액션
        }
        
        uniform_int_distribution<size_t> actionDist(0, actions.size() - 1);
        return actions[actionDist(rng_)];
    }
    else
    {
        // Exploitation: 최고 가치 액션 선택
        auto [bestAction, score] = evaluator_.selectBestAction(board, block);
        return bestAction;
    }
}

void TDLearner::updateWeights(const FeatureExtractor::Features& currentFeatures,
                              double reward,
                              const FeatureExtractor::Features& nextFeatures,
                              bool isTerminal)
{
    // 현재 가치 계산
    double currentValue = evaluator_.evaluate(currentFeatures);
    
    // 다음 가치 계산 (종료 상태면 0)
    double nextValue = isTerminal ? 0.0 : evaluator_.evaluate(nextFeatures);
    
    // TD 오차: δ = r + γ·V(s') - V(s)
    double tdError = reward + config_.discountFactor * nextValue - currentValue;
    
    // 가중치 업데이트: w ← w + α·δ·∇V(s)
    // ∇V(s) = feature 값들 (linear approximation이므로)
    Evaluator::Weights currentWeights = evaluator_.getWeights();
    
    currentWeights.aggregateHeight += config_.learningRate * tdError * currentFeatures.aggregateHeight;
    currentWeights.completeLines += config_.learningRate * tdError * currentFeatures.completeLines;
    currentWeights.holes += config_.learningRate * tdError * currentFeatures.holes;
    currentWeights.bumpiness += config_.learningRate * tdError * currentFeatures.bumpiness;
    currentWeights.maxHeight += config_.learningRate * tdError * currentFeatures.maxHeight;
    currentWeights.minHeight += config_.learningRate * tdError * currentFeatures.minHeight;
    
    evaluator_.setWeights(currentWeights);
}

double TDLearner::calculateReward(int linesCleared, int heightDiff, int holesDiff, bool gameOver) const
{
    if (gameOver)
    {
        return -1000.0; // 게임 오버는 큰 페널티
    }
    
    double reward = 0.0;
    
    // 라인 제거 보상 (exponential reward for multiple lines)
    if (linesCleared == 1) reward += 40.0;
    else if (linesCleared == 2) reward += 100.0;
    else if (linesCleared == 3) reward += 300.0;
    else if (linesCleared >= 4) reward += 1200.0;
    
    // 높이 증가 페널티
    if (heightDiff > 0)
    {
        reward -= heightDiff * 5.0;
    }
    
    // 구멍 증가 페널티
    if (holesDiff > 0)
    {
        reward -= holesDiff * 20.0;
    }
    
    // 생존 보상 (작은 양수)
    reward += 1.0;
    
    return reward;
}

TDLearner::Statistics TDLearner::runEpisode()
{
    Statistics stats;
    stats.episode = 0;
    
    // 보드 초기화
    Board board(true);
    board.init();
    
    // 블록 생성기 (간단한 랜덤)
    uniform_int_distribution<int> blockDist(0, 6);
    
    // 이전 상태 저장
    FeatureExtractor::Features prevFeatures = FeatureExtractor::extractFeatures(board);
    
    bool gameOver = false;
    int moves = 0;
    double totalReward = 0.0;
    
    while (!gameOver && moves < config_.maxMovesPerEpisode)
    {
        // 랜덤 블록 생성
        BlockType blockType = static_cast<BlockType>(blockDist(rng_));
        Block block(blockType);
        
        // 액션 선택
        Action action = selectAction(board, block);
        
        // 액션 시뮬레이션
        SimulationResult result = ActionSimulator::simulateAction(board, block, action);
        
        if (!result.isValid || result.gameOver)
        {
            // 게임 오버
            double reward = calculateReward(0, 0, 0, true);
            updateWeights(prevFeatures, reward, prevFeatures, true);
            totalReward += reward;
            gameOver = true;
            break;
        }
        
        // 보드 업데이트 (실제로 블록 배치)
        Block simBlock(blockType, Rotation(action.rotation), 
                      Position(action.column, BoardConstants::BLOCK_START_Y));
        ActionSimulator::moveBlockToPosition(simBlock, action.rotation, action.column);
        ActionSimulator::dropBlock(board, simBlock);
        board.mergeBlock(simBlock);
        int linesCleared = board.deleteFullLine();
        
        // 새로운 상태의 feature 추출
        FeatureExtractor::Features newFeatures = FeatureExtractor::extractFeatures(board);
        
        // 보상 계산
        int heightDiff = newFeatures.maxHeight - prevFeatures.maxHeight;
        int holesDiff = newFeatures.holes - prevFeatures.holes;
        double reward = calculateReward(linesCleared, heightDiff, holesDiff, false);
        
        // TD 업데이트
        updateWeights(prevFeatures, reward, newFeatures, false);
        
        // 통계 업데이트
        stats.totalLines += linesCleared;
        stats.totalScore += reward;
        stats.maxHeight = max(stats.maxHeight, newFeatures.maxHeight);
        totalReward += reward;
        moves++;
        
        // 다음 스텝 준비
        prevFeatures = newFeatures;
    }
    
    stats.moves = moves;
    stats.averageReward = moves > 0 ? totalReward / moves : 0.0;
    
    // Epsilon 감소
    config_.epsilon = max(config_.minEpsilon, config_.epsilon * config_.epsilonDecay);
    
    return stats;
}

std::vector<TDLearner::Statistics> TDLearner::train(int numEpisodes)
{
    vector<Statistics> allStats;
    allStats.reserve(numEpisodes);
    
    cout << "=== TD-Learning Training Started ===" << endl;
    cout << "Episodes: " << numEpisodes << endl;
    cout << "Learning Rate: " << config_.learningRate << endl;
    cout << "Discount Factor: " << config_.discountFactor << endl;
    cout << "Initial Epsilon: " << config_.epsilon << endl;
    cout << "=====================================" << endl << endl;
    
    for (int episode = 0; episode < numEpisodes; episode++)
    {
        Statistics stats = runEpisode();
        stats.episode = episode + 1;
        allStats.push_back(stats);
        
        // 주기적으로 진행 상황 출력
        if (config_.verbose && (episode + 1) % 10 == 0)
        {
            printStatistics(stats);
        }
        
        // 주기적으로 가중치 저장
        if ((episode + 1) % 100 == 0)
        {
            string filename = "weights_episode_" + to_string(episode + 1) + ".txt";
            saveWeights(filename);
            
            if (config_.verbose)
            {
                cout << "Weights saved to " << filename << endl;
            }
        }
    }
    
    cout << "\n=== Training Complete ===" << endl;
    cout << "Final Epsilon: " << config_.epsilon << endl;
    
    return allStats;
}

void TDLearner::printStatistics(const Statistics& stats) const
{
    cout << "Episode " << stats.episode << ": ";
    cout << "Lines=" << stats.totalLines << ", ";
    cout << "Moves=" << stats.moves << ", ";
    cout << "MaxHeight=" << stats.maxHeight << ", ";
    cout << "AvgReward=" << stats.averageReward << ", ";
    cout << "Epsilon=" << config_.epsilon;
    cout << endl;
}

bool TDLearner::saveProgress(const string& filename, const vector<Statistics>& allStats) const
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file for writing: " << filename << endl;
        return false;
    }
    
    file << "# TD-Learning Training Progress" << endl;
    file << "# Episode,TotalLines,TotalScore,MaxHeight,Moves,AverageReward" << endl;
    
    for (const auto& stats : allStats)
    {
        file << stats.episode << ","
             << stats.totalLines << ","
             << stats.totalScore << ","
             << stats.maxHeight << ","
             << stats.moves << ","
             << stats.averageReward << endl;
    }
    
    file.close();
    cout << "Training progress saved to " << filename << endl;
    return true;
}

