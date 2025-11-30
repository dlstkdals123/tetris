#include "MCLearner.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <chrono>
#include <cmath>

using namespace std;

MCLearner::MCLearner(const Config& config, const Evaluator::Weights& initialWeights)
    : evaluator_(initialWeights)
    , config_(config)
    , rng_(chrono::steady_clock::now().time_since_epoch().count())
    , dist_(0.0, 1.0)
{
}

Action MCLearner::selectAction(const Board& board, const Block& block, const Block* nextBlock)
{
    // ε-greedy 정책
    if (dist_(rng_) < config_.epsilon)
    {
        // Exploration: 랜덤 액션 선택
        return selectRandomAction(block);
    }
    else
    {
        // Exploitation: 최고 가치 액션 선택 (다음 블록까지 고려)
        if (nextBlock != nullptr)
        {
            auto [bestAction, score] = evaluator_.selectBestActionWithLookAhead(board, block, nextBlock, 1.0);
            return bestAction;
        }
        else
        {
            auto [bestAction, score] = evaluator_.selectBestAction(board, block);
            return bestAction;
        }
    }
}

Action MCLearner::selectRandomAction(const Block& block)
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

void MCLearner::updateWeightsMonteCarlo(const FeatureExtractor::Features& state,
                                        double G,
                                        bool isTerminal)
{
    // Monte Carlo 업데이트: w ← w + α[G - V(s)]·∇V(s)
    
    // 현재 상태의 가치 계산
    const auto& weights = evaluator_.getWeights();
    double currentValue = weights.aggregateHeight * state.aggregateHeight +
                         weights.holes * state.holes +
                         weights.bumpiness * state.bumpiness +
                         weights.maxHeight * state.maxHeight +
                         weights.minHeight * state.minHeight;
    
    // Monte Carlo 에러 계산: G - V(s)
    double error = G - currentValue;
    
    // 가중치 업데이트
    Evaluator::Weights newWeights = weights;
    
    newWeights.aggregateHeight += config_.learningRate * error * state.aggregateHeight;
    newWeights.holes += config_.learningRate * error * state.holes;
    newWeights.bumpiness += config_.learningRate * error * state.bumpiness;
    newWeights.maxHeight += config_.learningRate * error * state.maxHeight;
    newWeights.minHeight += config_.learningRate * error * state.minHeight;
    
    // L2 Regularization (Weight Decay)
    constexpr double L2_LAMBDA = 0.0001;
    newWeights.aggregateHeight *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.holes *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.bumpiness *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.maxHeight *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.minHeight *= (1.0 - config_.learningRate * L2_LAMBDA);
    
    // Weight Normalization
    constexpr double MAX_NORM = 1000.0;
    double norm = sqrt(newWeights.aggregateHeight * newWeights.aggregateHeight +
                      newWeights.holes * newWeights.holes +
                      newWeights.bumpiness * newWeights.bumpiness +
                      newWeights.maxHeight * newWeights.maxHeight +
                      newWeights.minHeight * newWeights.minHeight);
    
    if (norm > MAX_NORM)
    {
        double scale = MAX_NORM / norm;
        newWeights.aggregateHeight *= scale;
        newWeights.holes *= scale;
        newWeights.bumpiness *= scale;
        newWeights.maxHeight *= scale;
        newWeights.minHeight *= scale;
    }
    
    // 가중치 적용
    evaluator_.setWeights(newWeights);
}

double MCLearner::calculateReward(int linesCleared, int currentMaxHeight, int currentHoles, int currentBumpiness, int holesDiff, int bumpinessDiff, bool gameOver) const
{
    // 보상 관련 상수
    constexpr double GAME_OVER_PENALTY = -5000.0;           // 게임 오버 시 패널티
    constexpr double LIVING_REWARD = 1.0;                   // 살아있을 때 기본 보상
    constexpr double LINE_CLEAR_REWARD = 20.0;              // 한 줄 지울 때 보상 (선형)
    constexpr double HEIGHT_PENALTIES[] = {0.0, 20.0, 100.0, 500.0, 2000.0}; // 높이 구간별 패널티
    constexpr double HOLE_PENALTY = 20.0;                   // 구멍 증가 패널티
    constexpr double HOLE_STATE_PENALTY = 5.0;              // 남아있는 구멍 패널티
    constexpr double BUMPINESS_PENALTY = 2.0;               // 울퉁불퉁함 변화 패널티
    constexpr double BUMPINESS_STATE_PENALTY = 1.0;         // 현재 울퉁불퉁함 패널티

    // 게임 오버 시 패널티만 반환
    if (gameOver)
        return GAME_OVER_PENALTY;

    double reward = LIVING_REWARD;

    // 줄 삭제 보상 (선형, 여러 줄 보너스 없음)
    if (linesCleared > 0)
        reward += linesCleared * LINE_CLEAR_REWARD;

    // 높이 패널티 (구간별 적용)
    int heightIndex = std::min(currentMaxHeight / 5, 4);
    reward -= HEIGHT_PENALTIES[heightIndex];

    // 구멍 패널티
    reward -= holesDiff * HOLE_PENALTY;
    reward -= currentHoles * HOLE_STATE_PENALTY;

    // 울퉁불퉁함 패널티
    reward -= bumpinessDiff * BUMPINESS_PENALTY;
    reward -= currentBumpiness * BUMPINESS_STATE_PENALTY;

    return reward;
}

MCLearner::Statistics MCLearner::runEpisode(int initialStateType)
{
    Statistics stats;
    stats.episode = 0;
    
    // 보드 초기화 (5가지 초기 상태 중 하나)
    Board board(true);
    board.initWithState(initialStateType, rng_);
     
    // 블록 생성기 (간단한 랜덤)
    uniform_int_distribution<int> blockDist(0, 6);
    
    // 에피소드 경험 저장
    vector<Experience> episode;
    
    // 현재 상태
    FeatureExtractor::Features currentFeatures = FeatureExtractor::extractFeatures(board);
    
    bool gameOver = false;
    int moves = 0;
    
    // 첫 번째 블록 미리 생성
    BlockType nextBlockType = static_cast<BlockType>(blockDist(rng_));
    Block nextBlock(nextBlockType);
    
    while (!gameOver && moves < config_.maxMovesPerEpisode)
    {
        // 현재 블록 = 이전에 생성한 다음 블록
        Block block = nextBlock;
        
        // 다음 블록 미리 생성 (look-ahead용)
        nextBlockType = static_cast<BlockType>(blockDist(rng_));
        nextBlock = Block(nextBlockType);
        
        // 액션 선택 (다음 블록까지 고려)
        Action action = selectAction(board, block, &nextBlock);
        
        // 액션 시뮬레이션
        SimulationResult result = ActionSimulator::simulateAction(board, block, action);
        
        if (!result.isValid || result.gameOver)
        {
            // 게임 오버
            double reward = calculateReward(0, currentFeatures.maxHeight, currentFeatures.holes, currentFeatures.bumpiness, 0, 0, true);
            episode.push_back(Experience(currentFeatures, action, reward));
            gameOver = true;
            break;
        }
        
        // 보드 업데이트 (실제로 블록 배치)
        Block simBlock(block.getType(), Rotation(action.rotation), 
                      Position(action.column, BoardConstants::BLOCK_START_Y));
        ActionSimulator::moveBlockToPosition(simBlock, action.rotation, action.column);
        ActionSimulator::dropBlock(board, simBlock);
        board.mergeBlock(simBlock);
        
        // Feature 추출 (라인 삭제 전)
        FeatureExtractor::Features newFeatures = FeatureExtractor::extractFeatures(board);
        
        // 라인 삭제
        int linesCleared = board.deleteFullLine();
        
        // 구멍 변화량 계산 (이전 상태와 비교)
        int holesDiff = newFeatures.holes - currentFeatures.holes;
        
        // Bumpiness 변화량 계산 (이전 상태와 비교)
        int bumpinessDiff = newFeatures.bumpiness - currentFeatures.bumpiness;
        
        // 보상 계산
        double reward = calculateReward(linesCleared, newFeatures.maxHeight, newFeatures.holes, newFeatures.bumpiness, holesDiff, bumpinessDiff, false);
        
        // 경험 저장 (가중치 업데이트는 나중에!)
        episode.push_back(Experience(currentFeatures, action, reward));
        
        // 통계 업데이트
        stats.totalLines += linesCleared;
        stats.totalScore += reward;
        stats.maxHeight = max(stats.maxHeight, newFeatures.maxHeight);
        moves++;
        
        // 다음 스텝 준비
        currentFeatures = newFeatures;
    }
    
    // Monte Carlo 업데이트: 에피소드가 끝난 후 전체 경험을 역순으로 처리
    double G = 0.0;  // 누적 할인 보상
    double totalReward = 0.0;

    // 에피소드 길이 보너스 제거 - 매 스텝 생존 보상으로 대체
    // (각 경험의 reward에 이미 LIVING_REWARD가 포함되어 있음)

    for (int t = static_cast<int>(episode.size()) - 1; t >= 0; t--)
    {
        // 각 경험의 원래 reward 사용 (이미 calculateReward에서 생존 보상 포함)
        double adjustedReward = episode[t].reward;
        
        G = adjustedReward + config_.discountFactor * G;
        totalReward += episode[t].reward;
        
        // Monte Carlo 업데이트
        bool isTerminal = (t == static_cast<int>(episode.size()) - 1 && gameOver);
        updateWeightsMonteCarlo(episode[t].state, G, isTerminal);
    }
    
    stats.moves = moves;
    stats.averageReward = moves > 0 ? totalReward / moves : 0.0;
    
    return stats;
}

void MCLearner::updatePhase(int episode)
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

std::vector<MCLearner::Statistics> MCLearner::train(int numEpisodes)
{
    vector<Statistics> allStats;
    allStats.reserve(numEpisodes);
    
    // 최대값 추적
    int maxLinesCleared = 0;
    int maxMoves = 0;
    int maxLinesEpisode = 0;
    int maxMovesEpisode = 0;
    
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
        
        // 6가지 초기 상태를 골고루 사용 (에피소드 번호에 따라 순환)
        // 0: 4줄, 1: 3줄, 2: 2줄, 3: 1줄, 4: empty, 5: random
        int initialStateType = episode % 6;
        
        Statistics stats = runEpisode(initialStateType);
        stats.episode = episode + 1;
        allStats.push_back(stats);
        
        // 최대값 업데이트
        if (stats.totalLines > maxLinesCleared)
        {
            maxLinesCleared = stats.totalLines;
            maxLinesEpisode = stats.episode;
        }
        if (stats.moves > maxMoves)
        {
            maxMoves = stats.moves;
            maxMovesEpisode = stats.episode;
        }
        
        // 주기적으로 진행 상황 출력
        constexpr int PRINT_INTERVAL = 500;
        
        if (config_.verbose && (episode + 1) % PRINT_INTERVAL == 0)
        {
            // 500 에피소드마다만 출력
            printStatistics(stats, false);
            
            // Phase 전환 알림
            if (config_.useMultiStage)
            {
                cout << "  [LR=" << fixed << setprecision(5) << config_.learningRate 
                     << ", ε=" << fixed << setprecision(3) << config_.epsilon << "]" << endl;
            }
            cout << "  [Max Lines: " << maxLinesCleared << " (Ep " << maxLinesEpisode 
            << "), Max Moves: " << maxMoves << " (Ep " << maxMovesEpisode << ")]" << endl;
        }
    }
    
    cout << "\n=== Training Complete ===" << endl;
    cout << "Final Epsilon: " << fixed << setprecision(4) << config_.epsilon << endl;
    cout << "Final Learning Rate: " << fixed << setprecision(6) << config_.learningRate << endl;
    cout << "Best Performance:" << endl;
    cout << "  Max Lines: " << maxLinesCleared << " (Episode " << maxLinesEpisode << ")" << endl;
    cout << "  Max Moves: " << maxMoves << " (Episode " << maxMovesEpisode << ")" << endl;
    
    return allStats;
}

void MCLearner::printStatistics(const Statistics& stats, bool sameLine) const
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
    
    // 통계 출력 (더 자세한 정보)
    cout << "Lines=" << stats.totalLines 
         << " Moves=" << stats.moves 
         << " Height=" << stats.maxHeight 
         << " Reward=" << fixed << setprecision(2) << stats.averageReward
         << " Score=" << (int)stats.totalScore;
    
    if (sameLine)
    {
        cout << flush;  // 버퍼만 비우기 (줄바꿈 안함)
    }
    else
    {
        cout << endl;  // 줄바꿈
    }
}

bool MCLearner::saveProgress(const string& filename, const vector<Statistics>& allStats) const
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

