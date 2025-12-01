#include "MCLearner.h"
#include "BlockData.h"
#include "BoardConstants.h"
#include <iostream>
#include <iomanip>
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
        // Exploration: 유효한 액션 중에서 랜덤 선택
        return selectRandomAction(board, block);
    }
    else
    {
        // Exploitation: 최고 가치 액션 선택 (다음 블록까지 고려)
        if (nextBlock != nullptr)
        {
            auto [bestAction, score] = evaluator_.selectBestActionWithLookAhead(board, block, nextBlock, 1.0);
            // 유효한 액션이 없을 경우를 대비해 유효성 검사
            SimulationResult testResult = ActionSimulator::simulateAction(board, block, bestAction);
            if (!testResult.isValid)
            {
                // 유효한 액션이 없으면 랜덤 액션으로 폴백
                return selectRandomAction(board, block);
            }
            return bestAction;
        }
        else
        {
            auto [bestAction, score] = evaluator_.selectBestAction(board, block);
            // 유효한 액션이 없을 경우를 대비해 유효성 검사
            SimulationResult testResult = ActionSimulator::simulateAction(board, block, bestAction);
            if (!testResult.isValid)
            {
                // 유효한 액션이 없으면 랜덤 액션으로 폴백
                return selectRandomAction(board, block);
            }
            return bestAction;
        }
    }
}

Action MCLearner::selectRandomAction(const Board& board, const Block& block)
{
    constexpr int DEFAULT_ROTATION = 0;
    constexpr int DEFAULT_COLUMN = 5;
    
    // 이론적으로 가능한 모든 액션을 시뮬레이션하여 유효한 액션만 필터링
    vector<SimulationResult> validResults = ActionSimulator::simulateAllActions(board, block);
    
    if (validResults.empty())
    {
        // 유효한 액션이 없으면 기본 액션 반환 (게임 오버 상황)
        return Action(DEFAULT_ROTATION, DEFAULT_COLUMN);
    }
    
    // 유효한 액션 중에서 랜덤 선택
    uniform_int_distribution<size_t> actionDist(0, validResults.size() - 1);
    return validResults[actionDist(rng_)].action;
}

void MCLearner::updateWeightsMonteCarlo(const FeatureExtractor::Features& state,
                                        double G,
                                        bool isTerminal)
{
    // Monte Carlo 업데이트: w ← w + α[G - V(s)]·∇V(s)
    
    // 현재 상태의 가치 계산 (Bertsekas & Tsitsiklis: 26개 feature)
    const auto& weights = evaluator_.getWeights();
    double currentValue = 0.0;
    
    // 1. 각 열의 높이 (12개)
    for (int i = 0; i < 12; i++) {
        currentValue += weights.columnHeights[i] * state.columnHeights[i];
    }
    
    // 2. 인접한 열의 높이 차이 (11개)
    for (int i = 0; i < 11; i++) {
        currentValue += weights.heightDiffs[i] * state.heightDiffs[i];
    }
    
    // 3. 최대 높이 (1개)
    currentValue += weights.maxHeight * state.maxHeight;
    
    // 4. 구멍의 개수 (1개)
    currentValue += weights.holes * state.holes;
    
    // 5. 우물 깊이 합 (1개)
    currentValue += weights.wells * state.wells;
    
    // Monte Carlo 에러 계산: G - V(s)
    double error = G - currentValue;
    
    // 가중치 업데이트
    Evaluator::Weights newWeights = weights;
    
    // 1. 각 열의 높이 가중치 업데이트 (12개)
    for (int i = 0; i < 12; i++) {
        newWeights.columnHeights[i] += config_.learningRate * error * state.columnHeights[i];
    }
    
    // 2. 인접한 열의 높이 차이 가중치 업데이트 (11개)
    for (int i = 0; i < 11; i++) {
        newWeights.heightDiffs[i] += config_.learningRate * error * state.heightDiffs[i];
    }
    
    // 3. 최대 높이 가중치 업데이트 (1개)
    newWeights.maxHeight += config_.learningRate * error * state.maxHeight;
    
    // 4. 구멍의 개수 가중치 업데이트 (1개)
    newWeights.holes += config_.learningRate * error * state.holes;
    
    // 5. 우물 깊이 합 가중치 업데이트 (1개)
    newWeights.wells += config_.learningRate * error * state.wells;
    
    // L2 Regularization (Weight Decay)
    constexpr double L2_LAMBDA = 0.0001;
    for (int i = 0; i < 12; i++) {
        newWeights.columnHeights[i] *= (1.0 - config_.learningRate * L2_LAMBDA);
    }
    for (int i = 0; i < 11; i++) {
        newWeights.heightDiffs[i] *= (1.0 - config_.learningRate * L2_LAMBDA);
    }
    newWeights.maxHeight *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.holes *= (1.0 - config_.learningRate * L2_LAMBDA);
    newWeights.wells *= (1.0 - config_.learningRate * L2_LAMBDA);
    
    // Weight Normalization
    constexpr double MAX_NORM = 1000.0;
    double norm = 0.0;
    for (int i = 0; i < 12; i++) {
        norm += newWeights.columnHeights[i] * newWeights.columnHeights[i];
    }
    for (int i = 0; i < 11; i++) {
        norm += newWeights.heightDiffs[i] * newWeights.heightDiffs[i];
    }
    norm += newWeights.maxHeight * newWeights.maxHeight;
    norm += newWeights.holes * newWeights.holes;
    norm += newWeights.wells * newWeights.wells;
    norm = sqrt(norm);
    
    if (norm > MAX_NORM)
    {
        double scale = MAX_NORM / norm;
        for (int i = 0; i < 12; i++) {
            newWeights.columnHeights[i] *= scale;
        }
        for (int i = 0; i < 11; i++) {
            newWeights.heightDiffs[i] *= scale;
        }
        newWeights.maxHeight *= scale;
        newWeights.holes *= scale;
        newWeights.wells *= scale;
    }
    
    // 가중치 적용
    evaluator_.setWeights(newWeights);
}

double MCLearner::calculateReward(const FeatureExtractor::Features& currentFeatures,
    const FeatureExtractor::Features& newFeatures,
    int linesCleared, bool gameOver) const
{
    if (gameOver) return -50.0;
    return (double) linesCleared * 1.0;

    // // Bertsekas & Tsitsiklis 논문 스타일 보상 함수
    // // 비율: 구멍(15배) > 높이 차이(3.5배) > 높이(1배)
    
    // constexpr double GAME_OVER_PENALTY = -100.0;
    // constexpr double LINE_CLEAR_REWARD = 10.0;
    // constexpr double LIVING_REWARD = 1.0;
    
    // // 기본 단위: 높이 패널티
    // constexpr double HEIGHT_PENALTY = 1.0;           // 기본 단위 (가장 작음)
    // constexpr double HEIGHT_DIFF_PENALTY = 3.5;     // 높이의 3.5배 (2~5배 중간값)
    // constexpr double HOLES_PENALTY = 15.0;          // 높이의 15배 (10~20배 중간값)
    
    // // 상태 유지 패널티 (구멍이 있으면 계속 패널티)
    // constexpr double HOLES_STATE_PENALTY = 1.5;     // 구멍 상태 자체에 대한 작은 패널티
    
    // if (gameOver) return GAME_OVER_PENALTY;

    // double reward = 0.0;

    // // 라인 클리어 보상
    // if (linesCleared > 0) {
    //     reward += linesCleared * LINE_CLEAR_REWARD;
    // }

    // // 1. 높이 변화 패널티 (기본 단위)
    // // 각 열의 높이 변화량 합계
    // int totalHeightChange = 0;
    // for (int i = 0; i < 12; i++) {
    //     int heightDiff = newFeatures.columnHeights[i] - currentFeatures.columnHeights[i];
    //     totalHeightChange += heightDiff;
    // }
    // reward -= totalHeightChange * HEIGHT_PENALTY;
    
    // // 최대 높이 변화 패널티
    // int maxHeightDiff = newFeatures.maxHeight - currentFeatures.maxHeight;
    // reward -= maxHeightDiff * HEIGHT_PENALTY;

    // // 2. 인접한 열의 높이 차이 변화 패널티 (높이의 3.5배)
    // // 울퉁불퉁해지면 큰 패널티
    // int totalDiffChange = 0;
    // for (int i = 0; i < 11; i++) {
    //     int diffChange = newFeatures.heightDiffs[i] - currentFeatures.heightDiffs[i];
    //     totalDiffChange += diffChange;
    // }
    // reward -= totalDiffChange * HEIGHT_DIFF_PENALTY;

    // // 3. 구멍 변화 패널티 (높이의 15배 - 절대 악)
    // // 구멍이 생기면 압도적으로 큰 패널티
    // int holesDiff = newFeatures.holes - currentFeatures.holes;
    // reward -= holesDiff * HOLES_PENALTY;
    
    // // 구멍 상태 자체에도 패널티 (구멍이 있으면 계속 패널티)
    // reward -= newFeatures.holes * HOLES_STATE_PENALTY;

    // return reward;
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
            // 게임 오버 시에는 현재 feature를 그대로 사용
            FeatureExtractor::Features dummyFeatures = currentFeatures;
            double reward = calculateReward(currentFeatures, dummyFeatures, 0, true);
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
        
        // 라인 삭제
        int linesCleared = board.deleteFullLine();
        
        // Feature 추출 (Bertsekas & Tsitsiklis 스타일: 26개 feature)
        FeatureExtractor::Features newFeatures = FeatureExtractor::extractFeatures(board);
        
        // 보상 계산 (각 feature를 직접 사용)
        double reward = calculateReward(currentFeatures, newFeatures, linesCleared, false);
        
        // 경험 저장 (가중치 업데이트는 나중에!)
        episode.push_back(Experience(currentFeatures, action, reward));
        
        // 통계 업데이트
        stats.totalLines += linesCleared;
        stats.totalScore += reward;
        // maxHeight는 더 이상 feature가 아니므로 제거
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
        
        // 항상 empty 보드에서 시작 (stateType = 0)
        int initialStateType = 0;
        
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
        constexpr int PRINT_INTERVAL = 200;
        
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
            
            // 현재 가중치 출력
            cout << "\nCurrent Weights (Episode " << (episode + 1) << "):" << endl;
            evaluator_.printWeights();
            cout << endl;
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
