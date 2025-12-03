#include "Evaluator.h"
#include "BoardConstants.h"
#include "GameConstants.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

// Bertsekas & Tsitsiklis 논문 스타일: 26개 feature 초기 가중치
// 모든 가중치는 학습을 통해 최적값을 찾습니다.
Evaluator::Weights::Weights()
    : maxHeight(GameConstants::Evaluator::InitialWeight::MAX_HEIGHT)      // 최대 높이는 낮을수록 좋음
    , holes(GameConstants::Evaluator::InitialWeight::HOLES)         // 구멍은 치명적
    , wells(GameConstants::Evaluator::InitialWeight::WELLS)          // 우물은 낮을수록 좋음
{ 
    // 각 열의 높이 가중치 초기화 (높이는 낮을수록 좋음)
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        columnHeights[i] = GameConstants::Evaluator::InitialWeight::COLUMN_HEIGHT;
    }
    
    // 인접한 열의 높이 차이 가중치 초기화 (차이는 적을수록 좋음)
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        heightDiffs[i] = GameConstants::Evaluator::InitialWeight::HEIGHT_DIFF;
    }
}

Evaluator::Weights::Weights(const double heights[GameConstants::Feature::COLUMN_COUNT], const double diffs[GameConstants::Feature::HEIGHT_DIFF_COUNT],
                            double maxH, double holes, double wells)
    : maxHeight(maxH)
    , holes(holes)
    , wells(wells)
{
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        columnHeights[i] = heights[i];
    }
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        heightDiffs[i] = diffs[i];
    }
}

Evaluator::Evaluator(const Weights& weights)
    : weights_(weights)
{
}

double Evaluator::evaluate(const FeatureExtractor::Features& features) const
{
    double score = GameConstants::Simulation::INITIAL_LINES_CLEARED;

    // 1. 각 열의 높이 (12개)
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        double normalizedHeight = features.columnHeights[i] / GameConstants::Feature::NORMALIZATION_VALUE;
        score += weights_.columnHeights[i] * normalizedHeight;
    }

    // 2. 인접한 열의 높이 차이 (11개)
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        double normalizedDiff = features.heightDiffs[i] / GameConstants::Feature::NORMALIZATION_VALUE;
        score += weights_.heightDiffs[i] * normalizedDiff;
    }

    // 3. 최대 높이 (1개)
    double normalizedMaxHeight = features.maxHeight / GameConstants::Feature::NORMALIZATION_VALUE;
    score += weights_.maxHeight * normalizedMaxHeight;

    // 4. 구멍의 개수 (1개)
    double normalizedHoles = features.holes / GameConstants::Feature::NORMALIZATION_VALUE;
    score += weights_.holes * normalizedHoles;

    // 5. 우물 깊이 합 (1개)
    double normalizedWells = features.wells / GameConstants::Feature::NORMALIZATION_VALUE;
    score += weights_.wells * normalizedWells;

    return score;
}

double Evaluator::evaluateBoard(const Board& board) const
{
    FeatureExtractor::Features features = FeatureExtractor::extractFeatures(board);
    return evaluate(features);
}

double Evaluator::evaluateResult(const SimulationResult& result) const
{
    // 유효하지 않거나 게임 오버인 경우 최악의 점수
    if (!result.isValid || result.gameOver)
    {
        return GameConstants::Evaluator::DEATH_PENALTY;
    }
    
    return evaluate(result.features);
}

std::pair<Action, double> Evaluator::selectBestAction(const Board& board, const Block& block) const
{
    // 모든 가능한 액션 시뮬레이션
    vector<SimulationResult> results = ActionSimulator::simulateAllActions(board, block);
    
    if (results.empty())
    {
        // 가능한 액션이 없으면 기본 액션 반환
        return {Action(GameConstants::DefaultAction::ROTATION, GameConstants::DefaultAction::COLUMN), GameConstants::Evaluator::DEATH_PENALTY};
    }
    
    // 유효한 액션이 있는지 확인
    bool hasValidAction = false;
    for (const auto& result : results)
    {
        if (result.isValid && !result.gameOver)
        {
            hasValidAction = true;
            break;
        }
    }
    
    if (!hasValidAction)
    {
        // 유효한 액션이 없으면 기본 액션 반환
        return {Action(GameConstants::DefaultAction::ROTATION, GameConstants::DefaultAction::COLUMN), GameConstants::Evaluator::DEATH_PENALTY};
    }
    
    // 최고 점수 찾기 (유효한 액션 중에서)
    Action bestAction = results[GameConstants::Simulation::GAME_OVER_Y_THRESHOLD].action;
    double bestScore = GameConstants::Evaluator::DEATH_PENALTY;
    
    // 첫 번째 유효한 액션을 찾아서 초기값으로 설정
    for (const auto& result : results)
    {
        if (result.isValid && !result.gameOver)
        {
            bestAction = result.action;
            bestScore = evaluateResult(result);
            break;
        }
    }
    
    // 나머지 유효한 액션들과 비교
    for (size_t i = 1; i < results.size(); i++)
    {
        if (results[i].isValid && !results[i].gameOver)
        {
            double score = evaluateResult(results[i]);
            if (score > bestScore)
            {
                bestScore = score;
                bestAction = results[i].action;
            }
        }
    }
    
    return {bestAction, bestScore};
}

std::pair<Action, double> Evaluator::selectBestActionWithLookAhead(
    const Board& board, 
    const Block& currentBlock, 
    const Block* nextBlock,
    double lookAheadDiscount) const
{
    // 현재 블록의 모든 가능한 액션 시뮬레이션
    vector<SimulationResult> currentResults = ActionSimulator::simulateAllActions(board, currentBlock);
    
    if (currentResults.empty())
    {
        // 가능한 액션이 없으면 기본 액션 반환
        return {Action(GameConstants::DefaultAction::ROTATION, GameConstants::DefaultAction::COLUMN), GameConstants::Evaluator::DEATH_PENALTY};
    }
    
    // 유효한 액션이 있는지 확인
    bool hasValidAction = false;
    for (const auto& result : currentResults)
    {
        if (result.isValid && !result.gameOver)
        {
            hasValidAction = true;
            break;
        }
    }
    
    if (!hasValidAction)
    {
        // 유효한 액션이 없으면 기본 액션 반환
        return {Action(GameConstants::DefaultAction::ROTATION, GameConstants::DefaultAction::COLUMN), GameConstants::Evaluator::DEATH_PENALTY};
    }
    
    Action bestAction = currentResults[GameConstants::Simulation::GAME_OVER_Y_THRESHOLD].action;
    double bestScore = GameConstants::Evaluator::DEATH_PENALTY;
    
    // 각 현재 블록 액션에 대해 평가
    for (const auto& currentResult : currentResults)
    {
        if (!currentResult.isValid || currentResult.gameOver)
        {
            continue;  // 유효하지 않은 액션은 스킵
        }
        
        // 현재 블록 배치 후 보드 상태 시뮬레이션
        Board simBoard(board);
        Block simBlock(currentBlock.getType(), Rotation(currentResult.action.rotation),
                      Position(currentResult.action.column, BoardConstants::BLOCK_START_Y));
        ActionSimulator::moveBlockToPosition(simBlock, currentResult.action.rotation, currentResult.action.column);
        ActionSimulator::dropBlock(simBoard, simBlock);
        simBoard.mergeBlock(simBlock);
        simBoard.deleteFullLine();  // 라인 삭제 (반환값 무시)
        
        // 현재 블록 점수
        double currentScore = evaluateResult(currentResult);
        
        // 다음 블록까지 고려
        if (nextBlock != nullptr)
        {
            // 다음 블록의 모든 가능한 액션 시뮬레이션
            vector<SimulationResult> nextResults = ActionSimulator::simulateAllActions(simBoard, *nextBlock);
            
            if (!nextResults.empty())
            {
                // 다음 블록의 최고 점수 찾기
                double bestNextScore = GameConstants::Evaluator::DEATH_PENALTY;
                for (const auto& nextResult : nextResults)
                {
                    if (nextResult.isValid && !nextResult.gameOver)
                    {
                        double nextScore = evaluateResult(nextResult);
                        if (nextScore > bestNextScore)
                        {
                            bestNextScore = nextScore;
                        }
                    }
                }
                
                // 현재 점수 + (다음 블록 최고 점수 * 할인 인자)
                currentScore += bestNextScore * lookAheadDiscount;
            }
        }
        
        // 최고 점수 업데이트
        if (currentScore > bestScore)
        {
            bestScore = currentScore;
            bestAction = currentResult.action;
        }
    }
    
    return {bestAction, bestScore};
}

std::vector<std::pair<Action, double>> Evaluator::evaluateAllActions(const Board& board, const Block& block) const
{
    vector<SimulationResult> results = ActionSimulator::simulateAllActions(board, block);
    vector<pair<Action, double>> actionScores;
    
    for (const auto& result : results)
    {
        double score = evaluateResult(result);
        actionScores.push_back({result.action, score});
    }
    
    // 점수 내림차순 정렬
    sort(actionScores.begin(), actionScores.end(),
         [](const pair<Action, double>& a, const pair<Action, double>& b) {
             return a.second > b.second;
         });
    
    return actionScores;
}

bool Evaluator::saveWeights(const string& filename) const
{
    ofstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file for writing: " << filename << endl;
        return false;
    }
    
    file << "# Tetris Evaluator Weights (Bertsekas & Tsitsiklis Style - 26 features)" << endl;
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        file << "columnHeight" << i << " " << weights_.columnHeights[i] << endl;
    }
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        file << "heightDiff" << i << " " << weights_.heightDiffs[i] << endl;
    }
    file << "maxHeight " << weights_.maxHeight << endl;
    file << "holes " << weights_.holes << endl;
    file << "wells " << weights_.wells << endl;
    
    file.close();
    return true;
}

bool Evaluator::loadWeights(const string& filename, bool silent)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: Could not open file for reading: " << filename << endl;
        return false;
    }
    
    // 가중치 포인터 맵 (Bertsekas & Tsitsiklis: 26개 feature)
    unordered_map<string, double*> weightMap;
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        string key = "columnHeight" + to_string(i);
        weightMap[key] = &weights_.columnHeights[i];
    }
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        string key = "heightDiff" + to_string(i);
        weightMap[key] = &weights_.heightDiffs[i];
    }
    weightMap["maxHeight"] = &weights_.maxHeight;
    weightMap["holes"] = &weights_.holes;
    weightMap["wells"] = &weights_.wells;
    
    string line, key;
    double value;
    
    while (file >> key)
    {
        if (key[0] == '#')
        {
            // 주석 라인 건너뛰기
            getline(file, line);
            continue;
        }
        
        file >> value;
        
        auto it = weightMap.find(key);
        if (it != weightMap.end())
        {
            *(it->second) = value;
        }
    }
    
    file.close();
    if (!silent) {
        cout << "Weights loaded from " << filename << endl;
    }
    return true;
}

void Evaluator::printWeights() const
{
    cout << "=== Evaluator Weights (Bertsekas & Tsitsiklis - 26 features) ===" << endl;
    cout << "Column Heights: ";
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::COLUMN_COUNT; i++) {
        cout << weights_.columnHeights[i] << " ";
    }
    cout << endl;
    cout << "Height Diffs: ";
    for (int i = GameConstants::Simulation::GAME_OVER_Y_THRESHOLD; i < GameConstants::Feature::HEIGHT_DIFF_COUNT; i++) {
        cout << weights_.heightDiffs[i] << " ";
    }
    cout << endl;
    cout << "Max Height: " << weights_.maxHeight << endl;
    cout << "Holes: " << weights_.holes << endl;
    cout << "Wells: " << weights_.wells << endl;
    cout << "================================================================" << endl;
}

