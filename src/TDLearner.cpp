#include "TDLearner.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include <iostream>
#include <iomanip>
#include <fstream>
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
        return selectRandomAction(block);
    }
    else
    {
        // Exploitation: 최고 가치 액션 선택
        auto [bestAction, score] = evaluator_.selectBestAction(board, block);
        return bestAction;
    }
}

Action TDLearner::selectRandomAction(const Block& block)
{
    constexpr int DEFAULT_ROTATION = 0;
    constexpr int DEFAULT_COLUMN = 5;
    
    vector<Action> actions = ActionSimulator::generatePossibleActions(block.getType());
    if (actions.empty())
    {
        return Action(DEFAULT_ROTATION, DEFAULT_COLUMN);
    }
    
    uniform_int_distribution<size_t> actionDist(0, actions.size() - 1);
    return actions[actionDist(rng_)];
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
    
    // TD 오차 클리핑 (gradient explosion 방지)
    constexpr double MAX_TD_ERROR = 50.0;
    tdError = max(-MAX_TD_ERROR, min(MAX_TD_ERROR, tdError));
    
    // L2 정규화 계수
    constexpr double L2_LAMBDA = 0.0001;
    
    // 가중치 업데이트: w ← w + α·δ·∇V(s) - α·λ·w (L2 regularization)
    // ∇V(s) = feature 값들 (linear approximation이므로)
    Evaluator::Weights currentWeights = evaluator_.getWeights();
    
    currentWeights.aggregateHeight += config_.learningRate * tdError * currentFeatures.aggregateHeight
                                     - config_.learningRate * L2_LAMBDA * currentWeights.aggregateHeight;
    currentWeights.completeLines += config_.learningRate * tdError * currentFeatures.completeLines
                                   - config_.learningRate * L2_LAMBDA * currentWeights.completeLines;
    currentWeights.holes += config_.learningRate * tdError * currentFeatures.holes
                           - config_.learningRate * L2_LAMBDA * currentWeights.holes;
    currentWeights.bumpiness += config_.learningRate * tdError * currentFeatures.bumpiness
                               - config_.learningRate * L2_LAMBDA * currentWeights.bumpiness;
    currentWeights.maxHeight += config_.learningRate * tdError * currentFeatures.maxHeight
                               - config_.learningRate * L2_LAMBDA * currentWeights.maxHeight;
    currentWeights.minHeight += config_.learningRate * tdError * currentFeatures.minHeight
                               - config_.learningRate * L2_LAMBDA * currentWeights.minHeight;
    
    evaluator_.setWeights(currentWeights);
}

double TDLearner::calculateReward(int linesCleared, int heightDiff, int holesDiff, bool gameOver) const
{
    // 보상 상수들
    constexpr double GAME_OVER_PENALTY = -1000.0;
    constexpr double LINE_REWARDS[] = {0.0, 40.0, 100.0, 300.0, 1200.0};
    constexpr double HEIGHT_PENALTY_FACTOR = 5.0;
    constexpr double HOLE_PENALTY_FACTOR = 20.0;
    constexpr double SURVIVAL_REWARD = 1.0;
    
    if (gameOver)
    {
        return GAME_OVER_PENALTY;
    }
    
    double reward = SURVIVAL_REWARD;
    
    // 라인 제거 보상 (exponential reward for multiple lines)
    if (linesCleared > 0)
    {
        int index = min(linesCleared, 4);
        reward += LINE_REWARDS[index];
    }
    
    // 높이 증가 페널티
    if (heightDiff > 0)
    {
        reward -= heightDiff * HEIGHT_PENALTY_FACTOR;
    }
    
    // 구멍 증가 페널티
    if (holesDiff > 0)
    {
        reward -= holesDiff * HOLE_PENALTY_FACTOR;
    }
    
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
    
    // Note: Epsilon은 train() 함수에서 updatePhase()로 관리됨
    
    return stats;
}

void TDLearner::updatePhase(int episode)
{
    if (!config_.useMultiStage || config_.phases.empty())
    {
        // 기본 epsilon decay
        config_.epsilon = max(config_.minEpsilon, config_.epsilon * config_.epsilonDecay);
        return;
    }
    
    // 현재 phase 찾기
    for (const auto& phase : config_.phases)
    {
        if (episode >= phase.startEpisode && episode < phase.endEpisode)
        {
            // Learning rate 업데이트
            config_.learningRate = phase.learningRate;
            
            // Epsilon 선형 보간
            int phaseProgress = episode - phase.startEpisode;
            int phaseLength = phase.endEpisode - phase.startEpisode;
            double progress = static_cast<double>(phaseProgress) / phaseLength;
            
            config_.epsilon = phase.startEpsilon + 
                             (phase.endEpsilon - phase.startEpsilon) * progress;
            
            return;
        }
    }
    
    // 마지막 phase 이후에는 최소값 유지
    config_.epsilon = config_.minEpsilon;
}

std::vector<TDLearner::Statistics> TDLearner::train(int numEpisodes)
{
    vector<Statistics> allStats;
    allStats.reserve(numEpisodes);
    
    cout << "=== TD-Learning Training Started ===" << endl;
    cout << "Episodes: " << numEpisodes << endl;
    
    if (config_.useMultiStage)
    {
        cout << "Mode: Multi-Stage Training" << endl;
        cout << "Phases:" << endl;
        for (size_t i = 0; i < config_.phases.size(); i++)
        {
            const auto& phase = config_.phases[i];
            cout << "  Phase " << (i+1) << ": Episodes " << phase.startEpisode 
                 << "-" << phase.endEpisode << endl;
            cout << "    Epsilon: " << phase.startEpsilon << " → " << phase.endEpsilon << endl;
            cout << "    Learning Rate: " << phase.learningRate << endl;
        }
    }
    else
    {
        cout << "Mode: Standard Training" << endl;
        cout << "Learning Rate: " << config_.learningRate << endl;
        cout << "Initial Epsilon: " << config_.epsilon << endl;
    }
    
    cout << "Discount Factor: " << config_.discountFactor << endl;
    cout << "=====================================" << endl << endl;
    
    for (int episode = 0; episode < numEpisodes; episode++)
    {
        // Phase 업데이트
        updatePhase(episode);
        
        Statistics stats = runEpisode();
        stats.episode = episode + 1;
        allStats.push_back(stats);
        
        // 주기적으로 진행 상황 출력
        constexpr int PRINT_INTERVAL = 100;
        constexpr int SAVE_INTERVAL = 1000;
        
        if (config_.verbose && (episode + 1) % PRINT_INTERVAL == 0)
        {
            // 100 에피소드마다만 출력
            printStatistics(stats, false);
            
            // Phase 전환 알림
            if (config_.useMultiStage)
            {
                cout << "  [LR=" << fixed << setprecision(5) << config_.learningRate 
                     << ", ε=" << fixed << setprecision(3) << config_.epsilon << "]" << endl;
            }
        }
        
        // 주기적으로 가중치 저장
        if ((episode + 1) % SAVE_INTERVAL == 0)
        {
            string filename = "weights_episode_" + to_string(episode + 1) + ".txt";
            saveWeights(filename);
            
            if (config_.verbose)
            {
                cout << "✓ Weights saved to " << filename << endl << endl;
            }
        }
    }
    
    cout << "\n=== Training Complete ===" << endl;
    cout << "Final Epsilon: " << fixed << setprecision(4) << config_.epsilon << endl;
    cout << "Final Learning Rate: " << fixed << setprecision(6) << config_.learningRate << endl;
    
    return allStats;
}

void TDLearner::printStatistics(const Statistics& stats, bool sameLine) const
{
    // 같은 줄에 출력할 경우 커서를 줄 시작으로 이동
    if (sameLine)
    {
        cout << "\r";  // 커서를 줄 시작으로
    }
    
    // 진행률 바 생성
    int barWidth = 30;
    int progress = (stats.episode * barWidth) / config_.maxEpisodes;
    
    cout << "Episode " << stats.episode << "/" << config_.maxEpisodes << " [";
    for (int i = 0; i < barWidth; i++)
    {
        if (i < progress) cout << "█";
        else cout << "░";
    }
    cout << "] ";
    
    // 통계 출력
    cout << "Lines=" << stats.totalLines 
         << " Moves=" << stats.moves 
         << " Height=" << stats.maxHeight 
         << " Reward=" << fixed << setprecision(2) << stats.averageReward;
    
    if (sameLine)
    {
        cout << flush;  // 버퍼만 비우기 (줄바꿈 안함)
    }
    else
    {
        cout << endl;  // 줄바꿈
    }
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

